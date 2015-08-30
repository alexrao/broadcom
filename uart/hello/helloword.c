#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>


/* hello_init ³õÊ¼»¯º¯Êý */
static int hello_init(void)
{
	printk(KERN_ALERT "Hello World Init.\n");
	return 0;
}

static void hello_exit(void)
{
	printk(KERN_ALERT "goodbye World exit.\n");
	
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("rxping");
MODULE_DESCRIPTION("A Hello World Module");
