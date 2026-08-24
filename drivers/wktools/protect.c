#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/kprobes.h>
#include <linux/string.h>

#define WK_PROTECT_VERSION "1.0"

static const char *protected_name = "sdc40"; 

static void (*orig_submit_bio)(struct bio *bio);

static void hooked_submit_bio(struct bio *bio)
{
    if (op_is_write(bio_op(bio)) && bio->bi_bdev) {
        struct gendisk *disk = bio->bi_bdev->bd_disk;
        if (disk && strcmp(disk->disk_name, protected_name) == 0) {
            printk_ratelimited(KERN_ERR "wkProtect: Write to '%s' BLOCKED!\n", 
                               disk->disk_name);
            bio->bi_status = BLK_STS_IOERR; 
            bio_endio(bio);                 
            return;
        }
    }
    orig_submit_bio(bio);
}

static struct kprobe kp = {
    .symbol_name = "submit_bio",
};

static int __init wk_protect_init(void)
{
    int ret;
    orig_submit_bio = (void *)kprobe_lookup_name("submit_bio", 0);
    if (!orig_submit_bio) {
        printk(KERN_ERR "wkProtect: Cannot find submit_bio symbol!\n");
        return -ENOENT;
    }

    kp.pre_handler = (kprobe_pre_handler_t)hooked_submit_bio;
    ret = register_kprobe(&kp);
    if (ret < 0) {
        printk(KERN_ERR "wkProtect: Failed to register kprobe, ret=%d\n", ret);
        return ret;
    }

    printk(KERN_INFO "wkProtect v%s loaded. Protecting '%s'.\n", 
           WK_PROTECT_VERSION, protected_name);
    return 0;
}

static void __exit wk_protect_exit(void)
{
    unregister_kprobe(&kp);
    printk(KERN_INFO "wkProtect unloaded.\n");
}

__initcall(wk_protect_init);
__exitcall(wk_protect_exit);
