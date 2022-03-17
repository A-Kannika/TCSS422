#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(.gnu.linkonce.this_module) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section(__versions) = {
	{ 0x581b1365, "module_layout" },
	{ 0xbae2e3ca, "single_release" },
	{ 0xc9abd559, "seq_read" },
	{ 0xf07d6b2a, "seq_lseek" },
	{ 0xbafacf41, "remove_proc_entry" },
	{ 0x7413dd1c, "proc_create" },
	{ 0x37a0cba, "kfree" },
	{ 0x724e165d, "kmem_cache_alloc_trace" },
	{ 0x2b1decc4, "init_task" },
	{ 0x216e934e, "kmalloc_caches" },
	{ 0x9688de8b, "memstart_addr" },
	{ 0x50cfd4ac, "seq_printf" },
	{ 0xc5850110, "printk" },
	{ 0xb91729cc, "single_open" },
	{ 0x1fdc7df2, "_mcount" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "320B3189B292176A870715A");
