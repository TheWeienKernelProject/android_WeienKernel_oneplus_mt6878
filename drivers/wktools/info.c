#define pr_fmt(fmt) "wktools: " fmt

#include <linux/init.h>
#include <linux/kernel.h>

#define VERSION 1

static int __init wkpr_info(void)
{
    pr_info("Hello Guys\n");
    return 0;
}

device_initcall(wkpr_info);
