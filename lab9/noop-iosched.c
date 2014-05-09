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
        rq = list_entry(nd->queue.next, struct request, queuelist);
        printk(KERN_INFO "dispatch %-8llu\n", blk_rq_pos(rq));

        /* re pointing of list_head->next, list_head->prev*/
        list_del_init(&rq->queuelist);  // struct list_head queuelist;

        /* TODO: we may need to modify this so as not to mess up the SSFT result */
        elv_dispatch_sort(q, rq);
        return 1;
    }
    return 0;
}

static void noop_add_request(struct request_queue *q, struct request *rq)
{
    struct noop_data *nd = q->elevator->elevator_data;

    /* print rq's sector position */
    printk(KERN_INFO "add %-8llu\n", blk_rq_pos(rq));
    list_add_tail(&rq->queuelist, &nd->queue);
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
