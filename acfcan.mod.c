#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

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



static const char ____versions[]
__used __section("__versions") =
	"\x10\x00\x00\x00\xbc\x20\x73\x4e"
	"skb_put\0"
	"\x18\x00\x00\x00\xc5\x36\x6f\x78"
	"skb_tstamp_tx\0\0\0"
	"\x14\x00\x00\x00\x14\x48\xa6\x4a"
	"consume_skb\0"
	"\x1c\x00\x00\x00\x77\x17\x47\xb9"
	"rtnl_link_register\0\0"
	"\x14\x00\x00\x00\xbb\x6d\xfb\xbd"
	"__fentry__\0\0"
	"\x10\x00\x00\x00\x7e\x3a\x2c\x12"
	"_printk\0"
	"\x1c\x00\x00\x00\xcb\xf6\xfd\xf0"
	"__stack_chk_fail\0\0\0\0"
	"\x14\x00\x00\x00\xaa\xdf\x51\xd9"
	"__alloc_skb\0"
	"\x28\x00\x00\x00\xb3\x1c\xa2\x87"
	"__ubsan_handle_out_of_bounds\0\0\0\0"
	"\x20\x00\x00\x00\x7d\x5e\x7e\x8f"
	"can_dropped_invalid_skb\0"
	"\x20\x00\x00\x00\xd2\x28\x68\x47"
	"rtnl_link_unregister\0\0\0\0"
	"\x14\x00\x00\x00\xe6\xd0\xd7\x26"
	"init_net\0\0\0\0"
	"\x14\x00\x00\x00\x19\x03\xdd\x43"
	"skb_push\0\0\0\0"
	"\x18\x00\x00\x00\x4c\xbc\x0a\x6d"
	"param_ops_charp\0"
	"\x1c\x00\x00\x00\xca\x39\x82\x5b"
	"__x86_return_thunk\0\0"
	"\x10\x00\x00\x00\x5a\x25\xd5\xe2"
	"strcmp\0\0"
	"\x1c\x00\x00\x00\xb1\xdd\x22\xdb"
	"__dev_queue_xmit\0\0\0\0"
	"\x18\x00\x00\x00\x18\x01\x47\x56"
	"__warn_printk\0\0\0"
	"\x18\x00\x00\x00\x15\x13\x44\x0c"
	"dev_get_by_name\0"
	"\x20\x00\x00\x00\x7f\x11\x7e\xba"
	"skb_clone_tx_timestamp\0\0"
	"\x20\x00\x00\x00\xd6\x3c\xdb\x9e"
	"ethtool_op_get_ts_info\0\0"
	"\x18\x00\x00\x00\xd7\xd3\x75\x6d"
	"module_layout\0\0\0"
	"\x00\x00\x00\x00\x00\x00\x00\x00";

MODULE_INFO(depends, "can-dev");


MODULE_INFO(srcversion, "4C5D6F5A27FCC838012B8D4");
