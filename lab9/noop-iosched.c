/*
 * elevator noop
 */
#include <linux/blkdev.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/init.h>

struct noop_data {
    /* a doublely-linked list queue */
    struct list_head queue;
};

#define MAX_POS 1048576

static unsigned long long dispatched_rq_pos = 0;
static unsigned long long min_pos = MAX_POS;


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

        /* get request from the task queue */
        // printk(KERN_INFO "queue before dispatch:");
        // list_for_each_entry(rq, &nd->queue, queuelist) {
        //  printk(KERN_INFO "[%-5llu], ", blk_rq_pos(rq));
        // }

        rq = list_entry(nd->queue.next, struct request, queuelist);
        dispatched_rq_pos = blk_rq_pos(rq);
        printk(KERN_INFO "@ dispatch:%-5llu\n", dispatched_rq_pos);

        list_del_init(&rq->queuelist);  // struct list_head queuelist;

        // printk(KERN_INFO "queue after dispatch:");
        // if(!list_empty(&nd->queue)) {
        //  list_for_each_entry(rq, &nd->queue, queuelist) {
        //      printk(KERN_INFO "[%-5llu], ", blk_rq_pos(rq));
        //  }
        // } else
        //  printk(KERN_INFO "queue empty\n");

        /* TODO: we may need to modify this so as not to mess up the SSFT result */
        elv_dispatch_sort(q, rq);
        return 1;
    }
    return 0;
}

static void noop_add_request(struct request_queue *q, struct request *rq)
{
    struct noop_data *nd = q->elevator->elevator_data;
    struct request *req = NULL;
    struct request *chosen_req = NULL;
    unsigned long long pos_diff;
    unsigned int queue_size = 0, i;
    bool can_move = true;


    /* print rq's sector position */
    printk(KERN_INFO "@ add %-5llu\n", blk_rq_pos(rq));
    list_add_tail(&rq->queuelist, &nd->queue);

    /* get queue size */
    if(!list_empty(&nd->queue)) {
        list_for_each_entry(req, &nd->queue, queuelist)
            queue_size++;
    }

    printk(KERN_INFO "queue size:%-5u\n", queue_size);

    if(!list_empty(&nd->queue)) {
        for( i = 0; i < queue_size; ++i) {
            /* find the SS request */
            list_for_each_entry(req, &nd->queue, queuelist) {
                pos_diff = get_diff_abs(blk_rq_pos(req), dispatched_rq_pos);
                if(pos_diff == 0)
                    can_move = true;

                if((pos_diff < min_pos) && (pos_diff != 0) && can_move ) {
                    printk(KERN_INFO "have choose one:%-5llu\n", blk_rq_pos(req));
                    min_pos = pos_diff;
                    chosen_req = req;
                }
            }

            /* should not enter this */
            if(!chosen_req) {
                chosen_req = list_entry(&nd->queue.next, struct request, queuelist);
                printk(KERN_INFO "Error chosen_req:%-5llu\n", blk_rq_pos(chosen_req));
            } else {
                printk(KERN_INFO "chosen_req:%-5llu\n", blk_rq_pos(chosen_req));
            }

            min_pos = MAX_POS;
            can_move = false;
            dispatched_rq_pos = blk_rq_pos(chosen_req);
            list_move(&chosen_req->queuelist, &nd->queue);
            chosen_req = NULL;
        }

        printk(KERN_INFO "Sorted queue:");
        list_for_each_entry(req, &nd->queue, queuelist) {
            printk(KERN_INFO "%llu ", blk_rq_pos(req));
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
