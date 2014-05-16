/* file-mmu.c: ramfs MMU-based file operations
 *
 * Resizable simple ram filesystem for Linux.
 *
 * Copyright (C) 2000 Linus Torvalds.
 *               2000 Transmeta Corp.
 *
 * Usage limits added by David Gibson, Linuxcare Australia.
 * This file is released under the GPL.
 */

/*
 * NOTE! This filesystem is probably most useful
 * not as a real filesystem, but as an example of
 * how virtual filesystems can be written.
 *
 * It doesn't get much simpler than this. Consider
 * that this file implements the full semantics of
 * a POSIX-compliant read-write filesystem.
 *
 * Note in particular how the filesystem does not
 * need to implement any data structures of its own
 * to keep track of the virtual data: using the VFS
 * caches is sufficient.
 */

#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/ramfs.h>
#include <linux/uio.h>

#include "internal.h"

extern bool enable_encryption;

ssize_t my_aio_write(struct kiocb *iocb, const struct iovec *iov,
    unsigned long nr_segs, loff_t pos)
{
    printk(KERN_INFO "my ramfs aio_write handler!\n");

    /* if encryption is enabled, XOR with 0x25 */
    if(enable_encryption) {

        struct file *file = iocb->ki_filp;
        struct address_space *mapping = file->f_mapping;
        struct inode *inode = file->f_mapping->host;
        struct iov_iter i;

        pgoff_t index;      /* Pagecache index for current page */
        unsigned long offset;   /* Offset into pagecache page */
        unsigned long bytes;    /* Bytes to write to page */
        size_t copied;      /* Bytes copied from user */


        mutex_lock(&inode->i_mutex);


        /* encrypt the user data here */
        iov_iter_init(&i, iov, nr_segs, count, written);
        offset = (pos & (PAGE_CACHE_SIZE - 1));


        mutex_unlock(&inode->i_mutex);

    }
    else {

    }

    return generic_file_aio_write(iocb, iov, nr_segs, pos);
}

ssize_t my_aio_read(struct kiocb *iocb, const struct iovec *iov,
    unsigned long nr_segs, loff_t pos)
{
    printk(KERN_INFO "my ramfs aio_read handler!\n");
    return generic_file_aio_read(iocb, iov, nr_segs, pos);
}

const struct address_space_operations ramfs_aops = {
    .readpage   = simple_readpage,
    .write_begin    = simple_write_begin,
    .write_end  = simple_write_end,
    .set_page_dirty = __set_page_dirty_no_writeback,
};

const struct file_operations ramfs_file_operations = {
    .read       = do_sync_read,
    .aio_read   = my_aio_read,
    .write      = do_sync_write,
    .aio_write  = my_aio_write,
    .mmap       = generic_file_mmap,
    .fsync      = simple_sync_file,
    .splice_read    = generic_file_splice_read,
    .splice_write   = generic_file_splice_write,
    .llseek     = generic_file_llseek,
};

const struct inode_operations ramfs_file_inode_operations = {
    .getattr    = simple_getattr,
};
