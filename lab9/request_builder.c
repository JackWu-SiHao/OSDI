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
    for (i = 0; i < 13; i+=4)
    {
        if (isAscending)
        {
            __breadahead(bdev, (i*2)*4096, set_size);
            __breadahead(bdev, ((i+7)*2)*4096, set_size);
            __breadahead(bdev, ((i+1)*2)*4096, set_size);
            __breadahead(bdev, ((i+3)*2)*4096, set_size);
            isAscending = false;
        }
        else
        {
            __breadahead(bdev, ((i-1)*2-1)*4096, set_size);
            __breadahead(bdev, ((i-3)*2-1)*4096, set_size);
            __breadahead(bdev, ((i-7)*2-1)*4096, set_size);
            __breadahead(bdev, ((i)*2-1)*4096, set_size);
            isAscending = true;
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

MODULE_LICENSE("GPL");
module_init(init_read);
module_exit(exit_read);
