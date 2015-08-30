#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>

static struct class *jhtty_class;
static struct device	*jhtty_class_dev;

volatile unsigned long *pinconf = NULL;
volatile unsigned long *pinpull = NULL;
volatile unsigned long *pindata = NULL;


static int jhtty_open(struct inode *inode, struct file *file)
{
	printk("jhtty_open\n");
	/* 配置GPIO_74 为gpio输出 */
	*pinconf &= ~0x0F;
	return 0;
}

static ssize_t jhtty_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;

	//printk("jhtty_write\n");

	copy_from_user(&val, buf, count); //	copy_to_user();

	if (val == 1)
	{
		printk("drver High\n");
		// 高电平
		*pindata &= ~0x03;
		*pindata |= 0x02;
	}
	else
	{
		printk("drver Low\n");
		// 低电平电平
		*pindata &= ~0x03;
		*pindata |= 0x01;
	}
	
	return 0;
}

static struct file_operations jhtty_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   jhtty_open,     
	.write	=	jhtty_write,	   
};


int major;
static int jhtty_init(void)
{
	major = register_chrdev(0, "jhtty", &jhtty_fops); // 注册, 告诉内核

	jhtty_class = class_create(THIS_MODULE, "jhtty");

	jhtty_class_dev = device_create(jhtty_class, NULL, MKDEV(major, 0), NULL, "jhtty"); /* /dev/xyz */

	pinconf = (volatile unsigned long *)ioremap(0x10404124, 4);
	pindata = (volatile unsigned long *)ioremap(0x10404160, 4);
	printk("jhtty_init");
	return 0;
}

static void jhtty_exit(void)
{
	unregister_chrdev(major, "jhtty"); // 卸载

	device_unregister(jhtty_class_dev);
	class_destroy(jhtty_class);
	iounmap(pinconf);
	iounmap(pindata);
}

module_init(jhtty_init);
module_exit(jhtty_exit);


MODULE_LICENSE("GPL");

