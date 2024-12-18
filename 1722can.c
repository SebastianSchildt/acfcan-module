#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#include <linux/if_arp.h>

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/can.h>
#include <linux/can/can-ml.h>
#include <linux/can/dev.h>
#include <linux/can/skb.h>
#include <net/rtnetlink.h>
#include <linux/sysfs.h>


#include "1722ethernet.h"
#include "acfcandev.h"

#define DRV_NAME "acfcan"

#define NETDEV "enp0s31f6"
// #define NETDEV "lo"

// Module metadata
MODULE_AUTHOR("Sebastian Schildt");
MODULE_DESCRIPTION("IEEE-1722 ACF-CAN bridge");

MODULE_LICENSE("Dual BSD/GPL");
// MODULE_LICENSE("Proprieatary");

MODULE_ALIAS_RTNL_LINK(DRV_NAME);

char *version = "2016";


static ssize_t dstmac_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (count == 1 && *buf == '\n') {
		return 1;
	}
	struct net_device *net_dev = to_net_dev(dev);
	struct acfcan_cfg *cfg = get_acfcan_cfg(net_dev);

	// Handle the input value here
	__u8 newmac[6] = {0,1,2,3,4,5};
	int rc = sscanf(buf,"%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",&newmac[0],&newmac[1],&newmac[2],&newmac[3],&newmac[4],&newmac[5]);
	if (rc != 6) {
		printk(KERN_INFO "ACF-CAN Can not set destination MAC for %s. to %s\n", net_dev->name,buf);
		return count;
	}
	memcpy(cfg->dstmac, newmac, 6);
	printk(KERN_INFO "ACF-CAN: Setting destination MAC for %s to %02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx \n", net_dev->name, newmac[0],newmac[1],newmac[2],newmac[3],newmac[4],newmac[5]);
	return 17;
}

static ssize_t dstmac_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct net_device *net_dev = to_net_dev(dev);
	struct acfcan_cfg *cfg = get_acfcan_cfg(net_dev);

	return sprintf(buf, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", cfg->dstmac[0],cfg->dstmac[1],cfg->dstmac[2],cfg->dstmac[3],cfg->dstmac[4],cfg->dstmac[5]);
}

static ssize_t ethif_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (count == 1 && *buf == '\n') {
		return 1;
	}

	struct net_device *net_dev = to_net_dev(dev);
	struct acfcan_cfg *cfg = get_acfcan_cfg(net_dev);

	struct net_device *ethif;

	if (cfg->netdev) {
		netdev_put(cfg->netdev,&cfg->tracker);
	}
	ethif = netdev_get_by_name(&init_net, buf, &cfg->tracker, GFP_KERNEL);
	
	if (!ethif) {
		printk(KERN_INFO "ACF-CAN Can not use interface %s for %s\n", buf,net_dev->name);
		return count;
	}

	cfg->netdev=ethif;
	printk(KERN_INFO "ACF-CAN Using interface %s for %s\n", buf,net_dev->name);
	return count;
}

static ssize_t ethif_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct net_device *net_dev = to_net_dev(dev);
	struct acfcan_cfg *cfg = get_acfcan_cfg(net_dev);

	if (cfg->netdev) {
		return sprintf(buf, "%s", cfg->netdev->name);
	} else {
		buf[0] = '\0';
		return 0;
	}
}

static DEVICE_ATTR_RW(dstmac);
static DEVICE_ATTR_RW(ethif);


static struct attribute *dev_attrs[] = {
    &dev_attr_dstmac.attr,
	&dev_attr_ethif.attr,
    NULL, /* NULL-terminated list */
};

static struct attribute_group dev_attr_group = {
    .name = "acfcan", /* Subdirectory name in sysfs */
    .attrs = (struct attribute **)dev_attrs,
};


// We keep holding to this, becasue without it life does not make sense for acfcan
// Issue: This is currently global to the module, shoudl be per iface
static struct net_device *ether_dev;

static void acfcan_rx(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats = &dev->stats;

	stats->rx_packets++;
	stats->rx_bytes += can_skb_get_data_len(skb);

	skb->pkt_type = PACKET_BROADCAST;
	skb->dev = dev;
	skb->ip_summed = CHECKSUM_UNNECESSARY;

	netif_rx(skb);
}

static netdev_tx_t acfcan_tx(struct sk_buff *skb, struct net_device *dev)
{
	struct net_device_stats *stats = &dev->stats;
	unsigned int len;
	int loop;
	bool echo = false;

	if (can_dropped_invalid_skb(dev, skb))
		return NETDEV_TX_OK;

	len = can_skb_get_data_len(skb);
	stats->tx_packets++;
	stats->tx_bytes += len;

	/* set flag whether this packet has to be looped back */
	loop = skb->pkt_type == PACKET_LOOPBACK;

	skb_tx_timestamp(skb);

	if (can_is_can_skb(skb))
	{
		send_can_frame(ether_dev, dev,  (struct can_frame *)skb->data);
	}
	else if (can_is_canfd_skb(skb))
	{
		send_canfd_frame(ether_dev, dev, (struct canfd_frame *)skb->data);
	}
	else
	{
		printk(KERN_INFO "Packet is not a known CAN packet\n");
	}

	if (!echo)
	{
		/* no echo handling available inside this driver */
		if (loop)
		{
			/* only count the packets here, because the
			 * CAN core already did the echo for us
			 */
			stats->rx_packets++;
			stats->rx_bytes += len;
		}
		consume_skb(skb);
		return NETDEV_TX_OK;
	}

	/* perform standard echo handling for CAN network interfaces */

	if (loop)
	{
		skb = can_create_echo_skb(skb);
		if (!skb)
			return NETDEV_TX_OK;

		/* receive with packet counting */
		acfcan_rx(skb, dev);
	}
	else
	{
		/* no looped packets => no counting */
		consume_skb(skb);
	}
	return NETDEV_TX_OK;
}

static int acfcan_change_mtu(struct net_device *dev, int new_mtu)
{
	/* Do not allow changing the MTU while running */
	if (dev->flags & IFF_UP)
		return -EBUSY;

	if (new_mtu != CAN_MTU && new_mtu != CANFD_MTU &&
		!can_is_canxl_dev_mtu(new_mtu))
		return -EINVAL;

	WRITE_ONCE(dev->mtu, new_mtu);
	return 0;
}

static const struct net_device_ops acfcan_netdev_ops = {
	.ndo_start_xmit = acfcan_tx,
	.ndo_change_mtu = acfcan_change_mtu,
};

static const struct ethtool_ops acfcan_ethtool_ops = {
	.get_ts_info = ethtool_op_get_ts_info,
};


// The default stuff. Newlink can do more 
static void acfcan_setup(struct net_device *dev)
{
	bool echo = false;
	dev->type = ARPHRD_CAN;
	dev->mtu = CANFD_MTU;
	dev->hard_header_len = 0;
	dev->addr_len = 0;
	dev->tx_queue_len = 0;
	dev->flags = IFF_NOARP;
	can_set_ml_priv(dev, netdev_priv(dev));

    void *canpriv = can_get_ml_priv(dev);
	printk(KERN_INFO "Setting up device %s, priv is at %p \n", dev->name, canpriv);


	/* set flags according to driver capabilities */
	if (echo)
		dev->flags |= IFF_ECHO;

	dev->netdev_ops = &acfcan_netdev_ops;
	dev->ethtool_ops = &acfcan_ethtool_ops;
	dev->needs_free_netdev = true;

	struct acfcan_cfg *cfg = get_acfcan_cfg(dev);
	cfg->streamid = 0x1234;
	cfg->dstmac[0] = 0xff;
	cfg->dstmac[1] = 0xff;
	cfg->dstmac[2] = 0xff;
	cfg->dstmac[3] = 0xff;
	cfg->dstmac[4] = 0xff;
	cfg->dstmac[5] = 0xff;
	cfg->netdev = NULL;
}

static void acfcan_remove(struct net_device *dev, struct list_head *head)
{
	// Remove the custom sysfs attribute
	device_remove_file(&dev->dev, &dev_attr_dstmac);
	struct acfcan_cfg *cfg =  get_acfcan_cfg(dev);
	if (cfg->netdev) {
		netdev_put(cfg->netdev, &cfg->tracker);
	}
	unregister_netdevice(dev);
	printk(KERN_INFO "ACF-CAN remove interface %s\n", dev->name);
}

static int acfcan_newlink(struct net *net, struct net_device *dev,
                      struct nlattr *tb[], struct nlattr *data[],
                      struct netlink_ext_ack *extack)
{

    void *canpriv = can_get_ml_priv(dev);
	printk(KERN_INFO "Newlink for device %s, priv is at %p \n", dev->name, canpriv);

   //Need a new interface
	int err = register_netdevice(dev);

    if (err) {
        printk(KERN_ERR "Failed to register netdevice\n");
        return err;
    }

	printk(KERN_INFO "Creating group for device %s\n", dev->name);
	int ret = sysfs_create_group(&dev->dev.kobj, &dev_attr_group);
    if (ret) {
        pr_err("Failed to create sysfs group for net_device\n");
    }
	
	return 0;
}

//	.dellink = acfcan_remove, ?
static struct rtnl_link_ops acfcan_link_ops __read_mostly = {
	.kind = DRV_NAME,
	.priv_size = sizeof(struct can_ml_priv)+sizeof(struct acfcan_cfg),
	.setup = acfcan_setup,
	.newlink = acfcan_newlink,
	.dellink = acfcan_remove,
};

static int __init init_acfcan(void)
{
	pr_info("ACF-CAN\n");
	if (strcmp(version, "2016") == 0)
	{
		pr_info("Using version: %s\n", version);
	}
	else
	{
		pr_info("Unsupported 1722 version %s\n", version);
		return -1;
	}

	return rtnl_link_register(&acfcan_link_ops);
}

static void __exit cleanup_acfcan(void)
{
	pr_info("Unloading ACF-CAN\n");
	rtnl_link_unregister(&acfcan_link_ops);
}

module_init(init_acfcan);
module_exit(cleanup_acfcan);

module_param(version, charp, 0);
MODULE_PARM_DESC(version, "IEEE 1722 version");