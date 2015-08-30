#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
 .arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xde59746, "module_layout" },
	{ 0x60247532, "__iounmap" },
	{ 0xd8e2c80, "class_destroy" },
	{ 0x5efb64ba, "plat_iounmap" },
	{ 0xea2cc4e8, "device_unregister" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xacf2df89, "__ioremap" },
	{ 0x452c44e4, "plat_ioremap" },
	{ 0xc480df26, "device_create" },
	{ 0x545605d2, "__class_create" },
	{ 0x2a96b025, "__register_chrdev" },
	{ 0x12da5bb2, "__kmalloc" },
	{ 0xe89b68b6, "_raw_spin_unlock_irqrestore" },
	{ 0xdf8c695a, "__ndelay" },
	{ 0x9e7d6bd0, "__udelay" },
	{ 0x610090ee, "_raw_spin_lock_irqsave" },
	{ 0xea147363, "printk" },
	{ 0x354ef288, "finish_wait" },
	{ 0x1000e51, "schedule" },
	{ 0x7efb2701, "prepare_to_wait" },
	{ 0xc8b57c27, "autoremove_wake_function" },
	{ 0xb6091ec0, "__copy_user" },
	{ 0x37a0cba, "kfree" },
	{ 0xb7ceeb99, "__wake_up" },
	{ 0xfda85a7d, "request_threaded_irq" },
	{ 0xc3f3e73d, "kmem_cache_alloc" },
	{ 0xe6a37586, "malloc_sizes" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "076619C8ED014ECD84C40FB");
