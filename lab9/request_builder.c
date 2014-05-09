#include <linux/module.h>
#include <linux/init.h>
#include <linux/buffer_head.h>
#include <linux/blkdev.h>

static int set_size = 512;
static struct block_device *bdev;

static int __init init_read(void)
{
    int i=0;
    bool isAscending = true;
    printk("init!!!\n");
    // #define MKDEV(ma,mi)    (((ma) << MINORBITS) | (mi))
    bdev = open_by_devnum(MKDEV(8,17), 0x08000);
    if(IS_ERR(bdev)){
        return -EIO;
    }
    if(set_blocksize(bdev,set_size)){
        printk("set block size error\n");
        return -EIO;
    }

    // Design your write pattern here!!
    for(i=1; i<=24; i++){

        /* if the last request in a request group */
        if(i%4 == 0) {
            if(isAscending) {
                isAscending = false;
                __breadahead(bdev, (i*4)*512, set_size);

            }
            else {
                isAscending = true;
                __breadahead(bdev, ((24-i)*4-1)*512, set_size);
            }
        }

        /* not the last request in a request group*/
        else {
            if(isAscending) {
                __breadahead(bdev, (i*2)*512, set_size);
            }
            else {
                __breadahead(bdev, ((24-i)*2-1)*512, set_size);
            }
        }
    }
    return 0;
}
static void __exit exit_read(void)
{
    printk("exit!!!\n");
    if (bdev)
    {         blkdev_put(bdev, 0x08000);
        bdev = NULL;
    }
}
module_init(init_read);
module_exit(exit_read);
