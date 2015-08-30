#ifndef __KRI_DRV_TTY_H__
#define __KRI_DRV_TTY_H__


struct kri_tty_setting
{
	unsigned int baud;
	unsigned int start;
	unsigned int stop;
	unsigned int dlen;
};

#define DEV_SIZE 100

#define JHTTY_MAGIC 'x' //定义幻数
#define JHTTY_MAX_NR 2 //定义命令的最大序数

#define JHTTY_SETTING_GET _IO(JHTTY_MAGIC, 1)
#define JHTTY_SETTING_SET _IO(JHTTY_MAGIC, 2)

#endif /*__KRI_DRV_TTY_H__*/