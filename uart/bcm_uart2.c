#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <linux/ioctl.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/sched.h> 
#include <linux/interrupt.h>
#include <asm/spinlock.h>
#include <linux/slab.h>

#include "kri_drv_tty.h"



#define __IRQT_FALEDGE IRQ_TYPE_EDGE_FALLING
#define __IRQT_RISEDGE IRQ_TYPE_EDGE_RISING
#define __IRQT_LOWLVL IRQ_TYPE_LEVEL_LOW
#define __IRQT_HIGHLVL IRQ_TYPE_LEVEL_HIGH
#define IRQT_NOEDGE (0)
#define IRQT_RISING (__IRQT_RISEDGE)
#define IRQT_FALLING (__IRQT_FALEDGE)
#define IRQT_BOTHEDGE (__IRQT_RISEDGE|__IRQT_FALEDGE)
#define IRQT_LOW (__IRQT_LOWLVL)
#define IRQT_HIGH (__IRQT_HIGHLVL)
#define IRQT_PROBE IRQ_TYPE_PROBE


//#define		SEND_WITH_TIME_IRQ
//#define 	SEND_WITH_HRTIMER

static struct class *jhtty_class;
static struct device	*jhtty_class_dev;

volatile unsigned long *pincon = NULL;	// IO功能配置
volatile unsigned long *pindat = NULL;	// 数据位
volatile unsigned long *pindir = NULL;	// io方向 0：输出，1：输入
volatile unsigned long *pinode = NULL;	// 输出方式 0：推挽  1：开漏
volatile unsigned long *pinmsk = NULL;	// IO输入中断位

static unsigned char uart_rcv_buf[100];	// 读取的数据
static unsigned int ur_rcv_len = 0;	// 读取的数据长度
// 随眠唤醒机制
static DECLARE_WAIT_QUEUE_HEAD(uart_waitq);
static volatile int send_flag = 0;

spinlock_t txlock = SPIN_LOCK_UNLOCKED;

// static bool uart_send_dat_flag = false;

struct kri_tty_setting *jhtty_setting;


//#define DELAY_TIME(x)	(1000000000/x)
//#define DELAY_TIME	33300	 // 28800
#define DELAY_TIME	51000	 // 19200
//#define DELAY_TIME	104000	 // 9600

void uart_send_byte(unsigned char dat, unsigned int delay_time)//, unsigned int delay)
{
	int i; 
	unsigned long flags;

	spin_lock_irqsave(&txlock, flags);
	
	for(i=0; i<jhtty_setting->start + jhtty_setting->stop + jhtty_setting->dlen; i++)
	{
		if(i < jhtty_setting->start)
		{
			*pindat |= 1<<16;
		}
		else if(i < jhtty_setting->start + jhtty_setting->dlen)
		{
			if(0x01 & (dat >> (i -1)))
			{
				*pindat &= ~(1<<16);
			}
			else
			{
				*pindat |= 1<<16;
			}
		}
		else
		{
			*pindat &= ~(1<<16);
		}
		
		udelay(delay_time/1000);
		if(delay_time%1000 > 0)
			ndelay(delay_time%1000);
	}

	spin_unlock_irqrestore(&txlock, flags);
}

// 获取默认波特率
int jhtty_setting_get_default(struct kri_tty_setting *psetting)
{
	psetting->start = 1;
	psetting->stop = 1;
	psetting->dlen = 8;
	psetting->baud = 19200;

	return 1;
}


// 设置输入的波特率
int jhtty_setting_set(struct kri_tty_setting *psetting)
{
	if(psetting->start < 1 || psetting->stop < 1 || psetting->dlen < 1 
	|| (psetting->baud < 9600 || psetting->baud > 115200))
	{
		printk(KERN_ALERT "error setting arg");
		return -1;
	}

	jhtty_setting->start = psetting->start;
	jhtty_setting->stop = psetting->stop;
	jhtty_setting->dlen = psetting->dlen;
	jhtty_setting->baud = psetting->baud;
	
	return 1;
}


// 根据波特率获取对应电平延迟时间(单位：ns)
unsigned int jhtty_get_delay_time(int baudrate)
{
	unsigned int delay_time;
	
	switch(baudrate)
	{
	case 9600:
		delay_time = 104000;
		break;
	case 19200:
		delay_time = 51000;
		break;
	case 28800:
		delay_time = 33300;
		break;
	default:
		delay_time = 51000;
		break;
	}
	
	return  delay_time;
}



// 外部中断函数
static irqreturn_t jhtty_ur_irq(int irq, void *dev_id)
{

    send_flag = 1;                  /* 表示中断发生了 */
    wake_up_interruptible(&uart_waitq);   /* 唤醒休眠的进程 */
	
	// kill_fasync (&button_async, SIGIO, POLL_IN);
	
	return IRQ_RETVAL(IRQ_HANDLED);
}

// 设备打开函数
static int jhtty_open(struct inode *inode, struct file *file)
{
	int ret;

	
	jhtty_setting = kmalloc(sizeof(struct kri_tty_setting), GFP_KERNEL);
	if(!jhtty_setting)
	{
		ret = -ENOMEM;
		goto GO_RET;
	}

	/* 配置GPIO_96 97 为uart2输出 */
	*pincon &= ~(0xFF << 24);
	*pincon |= 0x11 << 24;

	*pindir &= ~(1<<16);	// Pin74 输出
	*pindir |= (1<<17);		// Pin75 输入
	*pinode &= ~(1<<16);
	
	// 获取并设置默认配置：1start 8datalen 1stop 119200 baudrate
	jhtty_setting_get_default(jhtty_setting);
	
	// 设置输入的中断判定, 高电平中断
	request_irq(0, jhtty_ur_irq, IRQT_HIGH, "jhttyi", NULL);
	
	ret = 0;  /* success */
	
GO_RET:
	return ret;
}


// ioctl 数据写入函数（与用户空间接口）
static ssize_t jhtty_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int i, ret = 0;
	unsigned int delay_time;
	//printk("jhtty_write\n");
	char *ur_send_buf;
	ur_send_buf = kmalloc(count, GFP_KERNEL);
	if(!ur_send_buf)
	{
		ret = -ENOMEM;
		goto GO_RET;
	}

	copy_from_user(ur_send_buf, buf, count); 

	delay_time = jhtty_get_delay_time(jhtty_setting->baud);
	for(i=0; i<count; i++)
	{
		uart_send_byte(*(ur_send_buf+i), delay_time);
	}
	
	kfree(ur_send_buf);

	
	ret = count;
	
GO_RET:
	return count;
}

// ioctl 读取数据函数
ssize_t jhtty_read(struct file *file, char __user *buf, size_t size, loff_t *ppos)
{
	if (size != 1)
		return -EINVAL;

	if (file->f_flags & O_NONBLOCK)
	{
		if (!send_flag)
			return -EAGAIN;
	}
	else
	{
		// 等待得到数据
		wait_event_interruptible(uart_waitq, send_flag);
		printk("wait_event_interruptible \n");
	}
	uart_rcv_buf[0] = 0xAA;
	ur_rcv_len = 1;
	/* 如果有按键动作, 返回键值 */
	copy_to_user(buf, uart_rcv_buf, ur_rcv_len);
	send_flag = 0;
	
	return ur_rcv_len;
}

// ioctl 接口函数
long jhtty_ioctl (struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct kri_tty_setting setting;
	
	/* don't even decode wrong cmds: better returning  ENOTTY than EFAULT */
	if (_IOC_TYPE(cmd) != JHTTY_MAGIC)
		return -ENOTTY;
	if (_IOC_NR(cmd) > JHTTY_MAX_NR)
		return -ENOTTY;

	switch(cmd)
	{

	case JHTTY_SETTING_GET:
	
		if(copy_to_user( (struct kri_tty_setting *)arg, jhtty_setting, sizeof(struct kri_tty_setting)))
		{
			ret = - EFAULT;
			goto GO_RET;
		}
		ret = 0;
		break;
	case JHTTY_SETTING_SET:
		if(copy_from_user(&setting, (struct kri_tty_setting *)arg, sizeof(struct kri_tty_setting)))
		{
			ret = - EFAULT;
			goto GO_RET;
		}
		jhtty_setting_set(&setting);
		ret = 0;
		break;

	default:  /* redundant, as cmd was checked against MAXNR */
		return -ENOTTY;
	}

GO_RET:
	return ret;
}

int jhtty_close(struct inode *inode, struct file *file)
{
//	printk("jhtty_close\n");
	kfree(jhtty_setting);
	return 0;
}

static struct file_operations jhtty_fops = {
    .owner  =   THIS_MODULE,    /* 这是一个宏，推向编译模块时自动创建的__this_module变量 */
    .open   =   jhtty_open,     
	.read	=	jhtty_read,	
	.write	=	jhtty_write,	
	.unlocked_ioctl 	=	jhtty_ioctl,
	.release	=	jhtty_close,
};


int major;
static int jhtty_init(void)
{
	major = register_chrdev(0, "bcmur2", &jhtty_fops); // 注册, 告诉内核

	jhtty_class = class_create(THIS_MODULE, "bcmur2");

	jhtty_class_dev = device_create(jhtty_class, NULL, MKDEV(major, 0), NULL, "bcmur2"); /* /dev/bcmur2 */

	// 功能配置，作为选择为io或者uart等硬件功能
	pincon = (volatile unsigned long *)ioremap(0x1040412c, 1);

	// UR2寄存器
	uart_rx_buf = (volatile unsigned long *)ioremap(0x10406980, 8); // 串口接收缓冲寄存器
	uart_tx_buf = uart_tx_buf;
	uart_dll = uart_tx_buf;
	uart_dlh = uart_tx_buf+1; 
	uart_irq_en = uart_tx_buf + 1;
	uart_irq_identity = uart_tx_buf + 2;
	uart_fifo_ctl = uart_tx_buf + 2;

	pinode = (volatile unsigned long *)ioremap(0x10406640, 4);
	pindat = pinode +1 ;
	pindir = pindat + 1;
	pinmsk = (volatile unsigned long *)ioremap(0x10406654, 4);
	printk("jhtty_init\n");
	return 0;
}

static void jhtty_exit(void)
{
	unregister_chrdev(major, "jhtty"); // 卸载

	device_unregister(jhtty_class_dev);
	class_destroy(jhtty_class);
	iounmap(pincon);
	iounmap(pindat);
	printk("jhtty_exit\n");

}

module_init(jhtty_init);
module_exit(jhtty_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("RXP");
MODULE_DESCRIPTION("for gpio simulate uart");
MODULE_VERSION("jhtty-001T1"); 
