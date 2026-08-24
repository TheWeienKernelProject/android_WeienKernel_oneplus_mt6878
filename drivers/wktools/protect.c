#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device-mapper.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/string.h>

#define DM_MSG_PREFIX "protect"

static const char *protected_devices[] = {
    "sdc40",
    NULL 
};

struct protect_c {
    struct dm_dev *dev;
    sector_t start;
};

static bool is_protected_device(const char *name)
{
    int i;
    for (i = 0; protected_devices[i] != NULL; i++) {
        if (strcmp(name, protected_devices[i]) == 0) {
            return true;
        }
    }
    return false;
}

static int protect_ctr(struct dm_target *ti, unsigned int argc, char **argv)
{
    struct protect_c *pc;
    int r;

    if (argc != 2) {
        ti->error = "Invalid argument count";
        return -EINVAL;
    }

    if (!is_protected_device(argv[0])) {
        ti->error = "Device not in protected list";
        return -EPERM;
    }

    pc = kzalloc(sizeof(*pc), GFP_KERNEL);
    if (!pc) return -ENOMEM;

    r = dm_get_device(ti, argv[0], dm_table_get_mode(ti->table), &pc->dev);
    if (r) {
        kfree(pc);
        ti->error = "Device lookup failed";
        return r;
    }

    if (kstrtoull(argv[1], 10, (unsigned long long *)&pc->start)) {
        dm_put_device(ti, pc->dev);
        kfree(pc);
        ti->error = "Invalid start sector";
        return -EINVAL;
    }

    ti->private = pc;
    return 0;
}

static void protect_dtr(struct dm_target *ti)
{
    struct protect_c *pc = ti->private;
    dm_put_device(ti, pc->dev);
    kfree(pc);
}

static int protect_map(struct dm_target *ti, struct bio *bio)
{
    struct protect_c *pc = ti->private;

    if (op_is_write(bio_op(bio))) {
        bio_io_error(bio);
        return DM_MAPIO_SUBMITTED;
    }

    bio_set_dev(bio, pc->dev->bdev);
    bio->bi_iter.bi_sector = pc->start + bio->bi_iter.bi_sector;
    return DM_MAPIO_REMAPPED;
}

static struct target_type protect_target = {
    .name    = "protect",
    .version = {1, 0, 0},
    .ctr     = protect_ctr,
    .dtr     = protect_dtr,
    .map     = protect_map,
};

static int __init wk_dm_protect_init(void)
{
    int r = dm_register_target(&protect_target);
    if (r < 0) {
        DMERR("Failed to register protect target: %d", r);
    } else {
        pr_info("wkPartitionProtecter: Partition Protecter load successfully.\n");
    }
    return r;
}

static void __exit wk_dm_protect_exit(void)
{
    dm_unregister_target(&protect_target);
}

__initcall(wk_dm_protect_init);
__exitcall(wk_dm_protect_exit);
