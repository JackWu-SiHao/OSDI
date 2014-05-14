/*
 * elevator noop
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/init.h>

#define MAX_POS 1048576

struct noop_data {
    /* a doublely-linked list queue */
    struct list_head queue;
};


static unsigned long long last_rq_pos = 0;
static bool is_first_dispatch = true;
static unsigned int dispatch_count = 0;


static unsigned long long get_diff_abs(unsigned long long a, unsigned long long b)
{
    if(a > b)
        return a-b;
    else
        return b-a;
}

static void noop_merged_requests(struct request_queue *q, struct request *rq,
                 struct request *next)
{
    list_del_init(&next->queuelist);
}

static int noop_dispatch(struct request_queue *q, int force)
{
    struct noop_data *nd = q->elevator->elevator_data;

    /* task queue: request_queue->elecator->elevator_data->queue */
    if (!list_empty(&nd->queue)) {
        struct request *rq;

        rq = list_entry(nd->queue.next, struct request, queuelist);
        last_rq_pos = blk_rq_pos(rq);
        printk(KERN_INFO "@ dispatch:%-5llu\n", last_rq_pos);

        list_del_init(&rq->queuelist);  // struct list_head queuelist;

        printk(KERN_INFO "queue after dispatch:");
        if(!list_empty(&nd->queue)) {
            list_for_each_entry(rq, &nd->queue, queuelist) {
                printk(KERN_INFO "[%-5llu], ", blk_rq_pos(rq));
            }
        } else
            printk(KERN_INFO "queue empty\n");

        /* TODO: we may need to modify this so as not to mess up the SSFT result */
        elv_dispatch_sort(q, rq);

        // printk(KERN_INFO "queue after elv_dispatch_sort:");
        // if(!list_empty(&nd->queue)) {
        //     list_for_each_entry(rq, &nd->queue, queuelist) {
        //         printk(KERN_INFO "[%-5llu], ", blk_rq_pos(rq));
        //     }
        // } else
        //     printk(KERN_INFO "queue empty\n");

        if(is_first_dispatch) {
            last_rq_pos = 0;
            is_first_dispatch = false;
        }

        printk(KERN_INFO "dispatch_count:%-5u\n", dispatch_count);
        dispatch_count++;

        return 1;
    }
    return 0;
}

static void noop_add_request(struct request_queue *q, struct request *rq)
{
    struct noop_data *nd = q->elevator->elevator_data;
    struct request *req = NULL;
    struct request *req_tmp = NULL;
    struct request *chosen_req = NULL;
    struct request *min_req = NULL;
    unsigned long long pos_diff;
    unsigned long long prev_rq_pos = 0;
    unsigned int queue_size = 0, i;
    unsigned int curr_min = MAX_POS;
    bool can_move = false;


    /* print rq's sector position */
    printk(KERN_INFO "@ add %-5llu, last_rq_pos %-5llu, end_sector %-5llu\n",
        blk_rq_pos(rq), last_rq_pos, q->end_sector);
    list_add_tail(&rq->queuelist, &nd->queue);

    if(!is_first_dispatch) {
        /* get queue size */
        if(!list_empty(&nd->queue)) {
            printk(KERN_INFO "queue after list_add_tail:");
            list_for_each_entry(req, &nd->queue, queuelist) {
                queue_size++;
                printk(KERN_INFO "%llu ", blk_rq_pos(req));
            }
        }

        printk(KERN_INFO "queue size:%-5u\n", queue_size);

        /* find first request with mininum pos to the last time's dispathed request */
        if(!list_empty(&nd->queue)) {
            /* default min_req set last request */
            list_for_each_entry(req, &nd->queue, queuelist)
                min_req = req;

            list_for_each_entry(req, &nd->queue, queuelist) {
                if( get_diff_abs(blk_rq_pos(req), last_rq_pos) < curr_min ) {
                    curr_min = get_diff_abs(blk_rq_pos(req), last_rq_pos);
                    min_req = req;
                }
            }
        }

        if(min_req)
            printk(KERN_INFO "min_req:%-5llu\n", blk_rq_pos(min_req));
        else
            printk(KERN_INFO "min_req is NULL\n");

        /* Now we get the mininum position(min_req) request to the last time's
         * dispatched request. Then move min_req to the head of queue. And the rest
         * of the requests in the queue need to do SSTF sort based on min_rq
         */

        if(min_req) {
            prev_rq_pos = blk_rq_pos(min_req);
            list_move(&min_req->queuelist, &nd->queue);
        }

        if(!list_empty(&nd->queue)) {
            for( i = 0; i < queue_size-1; ++i) {

                /* set default value */
                can_move = false;
                curr_min = MAX_POS;

                list_for_each_entry(req, &nd->queue, queuelist) {

                    /* start to move request until we reach the first min request(min_req)
                     */
                    if( req == min_req ) {
                        can_move = true;
                        continue;
                    }

                    if( can_move ) {
                        pos_diff = get_diff_abs(blk_rq_pos(req), prev_rq_pos);
                        if( pos_diff < curr_min ) {
                            printk(KERN_INFO "have choose one:%-5llu\n", blk_rq_pos(req));
                            curr_min = pos_diff;
                            chosen_req = req;
                        }
                    }
                }

                /* should not enter this */
                if(!chosen_req) {
                    list_for_each_entry(req, &nd->queue, queuelist)
                        chosen_req = req;
                    printk(KERN_INFO "Error chosen_req:%-5llu\n", blk_rq_pos(chosen_req));
                    BUG_ON(1);
                } else {
                    prev_rq_pos = blk_rq_pos(chosen_req);
                    list_move(&chosen_req->queuelist, &nd->queue);

                    printk(KERN_INFO "chosen_req:%-5llu\n", prev_rq_pos);
                    printk(KERN_INFO "after move:");
                    list_for_each_entry(req_tmp, &nd->queue, queuelist) {
                        printk(KERN_INFO "%llu ", blk_rq_pos(req_tmp));
                    }
                }
            }

            printk(KERN_INFO "Sorted queue:");
            list_for_each_entry(req, &nd->queue, queuelist) {
                printk(KERN_INFO "%llu ", blk_rq_pos(req));
            }
        }
    }
}

static int noop_queue_empty(struct request_queue *q)
{
    struct noop_data *nd = q->elevator->elevator_data;

    return list_empty(&nd->queue);
}

static struct request *
noop_former_request(struct request_queue *q, struct request *rq)
{
    struct noop_data *nd = q->elevator->elevator_data;

    if (rq->queuelist.prev == &nd->queue)
        return NULL;
    return list_entry(rq->queuelist.prev, struct request, queuelist);
}

static struct request *
noop_latter_request(struct request_queue *q, struct request *rq)
{
    struct noop_data *nd = q->elevator->elevator_data;

    if (rq->queuelist.next == &nd->queue)
        return NULL;
    return list_entry(rq->queuelist.next, struct request, queuelist);
}

static void *noop_init_queue(struct request_queue *q)
{
    struct noop_data *nd;

    nd = kmalloc_node(sizeof(*nd), GFP_KERNEL, q->node);
    if (!nd)
        return NULL;
    INIT_LIST_HEAD(&nd->queue);
    return nd;
}

static void noop_exit_queue(struct elevator_queue *e)
{
    struct noop_data *nd = e->elevator_data;

    BUG_ON(!list_empty(&nd->queue));
    kfree(nd);
}

static struct elevator_type elevator_noop = {
    .ops = {
        .elevator_merge_req_fn      = noop_merged_requests,
        .elevator_dispatch_fn       = noop_dispatch,
        .elevator_add_req_fn        = noop_add_request,
        .elevator_queue_empty_fn    = noop_queue_empty,
        .elevator_former_req_fn     = noop_former_request,
        .elevator_latter_req_fn     = noop_latter_request,
        .elevator_init_fn       = noop_init_queue,
        .elevator_exit_fn       = noop_exit_queue,
    },
    .elevator_name = "noop",
    .elevator_owner = THIS_MODULE,
};

static int __init noop_init(void)
{
    elv_register(&elevator_noop);
    return 0;
}

static void __exit noop_exit(void)
{
    elv_unregister(&elevator_noop);
}

module_init(noop_init);
module_exit(noop_exit);


MODULE_AUTHOR("Jens Axboe");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("No-op IO scheduler");
