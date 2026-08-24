#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/blkdev.h>
#include <linux/ftrace.h>
#include <linux/string.h>

#define WK_PROTECT_VERSION "1.0"

static const char *protected_name = "sdc40"; 

static void (*orig_submit_bio)(struct bio *bio);

static void hooked_submit_bio(struct bio *bio)
{
    if (op_is_write(bio_op(bio)) && bio->bi_bdev) {
        struct gendisk *disk = bio->bi_bdev->bd_disk;
    
        if (disk && strcmp(disk->disk_name, protected_name) == 0) {
            printk_ratelimited(KERN_ERR "wkProtect: Write to '%s' BLOCKED by Ftrace!\n", 
                               disk->disk_name);
            
            bio->bi_status = BLK_STS_IOERR;
            bio_endio(bio);
            return; 
        }
    }
    orig_submit_bio(bio);
}

static struct ftrace_ops ftrace_ops = {
    .func = (void *)hooked_submit_bio, 
    .flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_RECURSION_SAFE,
};

static int __init wk_protect_init(void)
{
    int ret;

    ret = register_ftrace_function(&ftrace_ops);
    if (ret < 0) {
        printk(KERN_ERR "wkProtect: Failed to register ftrace, ret=%d\n", ret);
        return ret;
    }
    
    ret = ftrace_set_filter(&ftrace_ops, "submit_bio", strlen("submit_bio"), 0);
    if (ret < 0) {
        unregister_ftrace_function(&ftrace_ops);
        printk(KERN_ERR "wkProtect: Failed to set ftrace filter, ret=%d\n", ret);
        return ret;
    }
    
    orig_submit_bio = (void *)kprobe_lookup_name("submit_bio", 0);
    if (!orig_submit_bio) {
        unregister_ftrace_function(&ftrace_ops);
        printk(KERN_ERR "wkProtect: Cannot find submit_bio symbol!\n");
        return -ENOENT;
    }

    printk(KERN_INFO "wkProtect v%s loaded. Protecting '%s' via Ftrace.\n", 
           WK_PROTECT_VERSION, protected_name);
    return 0;
}

static void __exit wk_protect_exit(void)
{
    unregister_ftrace_function(&ftrace_ops);
    printk(KERN_INFO "wkProtect unloaded.\n");
}

__initcall(wk_protect_init);
__exitcall(wk_protect_exit);
