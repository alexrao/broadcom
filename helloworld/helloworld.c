#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/init.h>


static int helloworld_init(void)
{
    printk("helloworld  init\n");
    return 0;
}

static void helloworld_exit(void)
{
    printk("helloWrold exit\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);

MODULE_LICENSE("GPL");

