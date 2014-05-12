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

static struct request *dispatched_rq = NULL; // the request dispatched first
static unsigned long long dispatched_rq_pos = 0;
static unsigned long long min_diff = 65536;


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
        struct request *rq = NULL;
        struct request *rq_next = NULL;
        struct request *rq_prev = NULL;
        unsigned long long next_pos, prev_pos;

        /* get request from the task queue */

        printk(KERN_INFO "queue before dispatch:");
        list_for_each_entry(rq, &nd->queue, queuelist) {
            printk(KERN_INFO "[%-5llu], ", blk_rq_pos(rq));
        }

        /* should not enter this, give a rq to dispatch anyway */
        if(!dispatched_rq) {
            rq = list_entry(nd->queue.next, struct request, queuelist);
            printk(KERN_ALERT "error dispatch:%-5llu\n", blk_rq_pos(rq));
            goto done;
        }


        printk(KERN_INFO "@ dispatch:%-5llu\n", blk_rq_pos(dispatched_rq));
        rq = dispatched_rq;

        // /* check dispatched_rq->next exists */
        // if(rq->queuelist.next != rq->queuelist && rq->queuelist.next != NULL) {
        //  rq_next = list_entry(rq->queuelist.next, struct request, queuelist);
        //  next_pos = blk_rq_pos(rq_next);
        // }

        // /* check dispatched_rq->prev exists */
        // if(rq->queuelist.prev != rq->queuelist && rq->queuelist.prev != NULL) {
        //  rq_prev = list_entry(rq->queuelist.prev, struct request, queuelist);
        //  prev_pos = blk_rq_pos(rq_prev);
        // }


        rq_next = list_entry(rq->queuelist.next, struct request, queuelist);
        rq_prev = list_entry(rq->queuelist.prev, struct request, queuelist);

        /* both next and prev exist */
        if(rq_next && rq_prev) {
            next_pos = blk_rq_pos(rq_next);
            prev_pos = blk_rq_pos(rq_prev);
            printk(KERN_INFO "next_pos:%-5llu\n", next_pos);
            printk(KERN_INFO "prev_pos:%-5llu\n", prev_pos);

            if(get_diff_abs(next_pos,dispatched_rq_pos) < get_diff_abs(prev_pos,dispatched_rq_pos)) {
                dispatched_rq = rq_next;
                dispatched_rq_pos = next_pos;
            } else {
                dispatched_rq = rq_prev;
                dispatched_rq_pos = prev_pos;
            }

        } else if(rq_next && !rq_prev) {
            next_pos = blk_rq_pos(rq_next);
            printk(KERN_INFO "next_pos:%-5llu\n", next_pos);

            dispatched_rq = rq_next;
            dispatched_rq_pos = next_pos;

        } else if(!rq_next && rq_prev) {
            prev_pos = blk_rq_pos(rq_prev);
            printk(KERN_INFO "prev_pos:%-5llu\n", prev_pos);

            dispatched_rq = rq_prev;
            dispatched_rq_pos = prev_pos;

        } else {
            // at this point, queue should be empty. Should call add_request
        }


done:
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
        return 1;
    }

    /* finish dispatch */
    else {
        dispatched_rq = NULL;
        min_diff = 65536;
        return 0;
    }
}

static void noop_add_request(struct request_queue *q, struct request *rq)
{
    struct noop_data *nd = q->elevator->elevator_data;
    struct request *req;
    unsigned long long pos_diff;


    /* print rq's sector position */
    printk(KERN_INFO "@ add %-5llu\n", blk_rq_pos(rq));

    /* check wether rq is the request to be dispatched first */
    pos_diff = get_diff_abs(blk_rq_pos(rq), dispatched_rq_pos);
    if(pos_diff < min_diff) {
        printk(KERN_INFO "dispatched_rq_pos:");
        dispatched_rq = rq;
        min_diff = pos_diff;
    }

    /*  insertion sort to the elevator queue*/
    if(!list_empty(&nd->queue)) {
        list_for_each_entry(req, &nd->queue, queuelist) {
            if(blk_rq_pos(rq) < blk_rq_pos(req)) {
                list_add_tail(&rq->queuelist, &req->queuelist);
                return;
            }
        }
    }

    /* add to tail if rq has the largest pos or queue is empty*/
    list_add_tail(&rq->queuelist, &nd->queue);

    // printk(KERN_INFO "Sorted queue:");
    // if(!list_empty(&nd->queue)) {
    //  list_for_each_entry(req, &nd->queue, queuelist) {
    //      printk(KERN_INFO "%llu ", blk_rq_pos(req));
    //  }
    // }
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
