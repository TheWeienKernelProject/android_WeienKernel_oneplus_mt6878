#include <linux/init.h>
#include <linux/kernel.h>

#define VERSION 1
#define pr_fmt(fmt) "wktools: " fmt

static int __init wkpr_info(void)
{
    pr_info("HelloWorld！\n");
    return 0;
}
