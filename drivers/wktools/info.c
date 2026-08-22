#include <linux/init.h>
#include <linux/kernel.h>

#define VERSION 1
#define pr_fmt(fmt) "my_builtin: " fmt

static int __init wkpr_info(void)
{
    pr_info("wktools: HelloWorld！\n");
    return 0;
}
