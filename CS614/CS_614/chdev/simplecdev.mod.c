#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
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
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x71e635cb, "module_put" },
	{ 0x92997ed8, "_printk" },
	{ 0xde023e6a, "try_module_get" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xd0da656b, "__stack_chk_fail" },
	{ 0xed735972, "__register_chrdev" },
	{ 0x300207ba, "__class_create" },
	{ 0xf90c8a50, "device_create" },
	{ 0x468f544c, "class_destroy" },
	{ 0x41bcb337, "kmalloc_caches" },
	{ 0x14db893f, "kmalloc_trace" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x37a0cba, "kfree" },
	{ 0x4688dfb5, "device_destroy" },
	{ 0x7c6909e, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "A2FB9CB40E2190D8AD9E14B");
