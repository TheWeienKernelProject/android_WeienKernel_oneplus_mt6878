#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/string.h>

#define WK_PROTECT_VERSION "1.0"

static const char *protected_disks[] = {
    "sdc40", /* boot_a */
    NULL               
};

static bool is_protected_device(const char *name)
{
    int i;
    if (!name) return false;
    for (i = 0; protected_disks[i] != NULL; i++) {
        if (strcmp(name, protected_disks[i]) == 0) {
            return true;
        }
    }
    return false;
}

static blk_qc_t wk_protect_submit_bio(struct bio *bio)
(
    if (op_is_write(bio_op(bio))) {
        struct gendisk *disk = bio->bi_bdev->bd_disk;
        if (disk && is_protected_device(disk->disk_name)) {
            printk_ratelimited(KERN_ERR "wkPartitionProtecter: Write to '%s' blocked!\n", 
                               disk->disk_name);
            bio_io_error(bio);
            return BLK_QC_T_NONE; 
        }
    }
    
    return bio->bi_bdev->bd_disk->fops->submit_bio(bio);
}

static struct blk_filter_ops wk_protect_filter_ops = {
    .submit_bio = wk_protect_submit_bio,
};

static int __init wk_protect_init(void)
{
    struct block_device *bdev;
    int i, attached_count = 0;

    for (i = 0; protected_disks[i] != NULL; i++) {
        bdev = blkdev_get_by_path(protected_disks[i], FMODE_READ, NULL);
        if (IS_ERR(bdev)) {
            printk(KERN_WARNING "wkPartitionProtecter: Device '%s' not found, skipping.\n", 
                   protected_disks[i]);
            continue;
        }
        
        if (blk_filter_attach(bdev, &wk_protect_filter_ops) == 0) {
            printk(KERN_INFO "wkPartitionProtecter: Successfully protected '%s'.\n", 
                   protected_disks[i]);
            attached_count++;
        } else {
            printk(KERN_ERR "wkPartitionProtecter: Failed to attach filter to '%s'.\n", 
                   protected_disks[i]);
        }

        blkdev_put(bdev, FMODE_READ);
    }

    printk(KERN_INFO "wkPartitionProtecter v%s loaded. Protected %d device(s).\n", 
           WK_PROTECT_VERSION, attached_count);
    return 0;
}

static void __exit wk_protect_exit(void)
{
    struct block_device *bdev;
    int i;
    for (i = 0; protected_disks[i] != NULL; i++) {
        bdev = blkdev_get_by_path(protected_disks[i], FMODE_READ, NULL);
        if (!IS_ERR(bdev)) {
            blk_filter_detach(bdev);
            blkdev_put(bdev, FMODE_READ);
        }
    }
    printk(KERN_INFO "wkPartitionProtecter unloaded.\n");
}

__initcall(wk_protect_init);
__exitcall(wk_protect_exit);
