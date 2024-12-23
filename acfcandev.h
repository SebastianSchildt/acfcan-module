#pragma once

#include <linux/types.h>
#include <linux/can/can-ml.h>
#include <net/net_trackers.h>
#include <linux/list.h>
#include <linux/sysfs.h>


/* Private per-device configuration */
struct acfcan_cfg
{ 
    struct list_head list; //we need a list so we can map received ethernet packets
    __u8 dstmac[6]; //send acf-can frames to this mac
    __u64 streamid; //use acf-can stream-id
    char ethif[IFNAMSIZ];
    struct net_device *netdev; //use this interface for acf-can frames
    netdevice_tracker tracker;
};

// get the acfcan_cfg struct from the device
#define get_acfcan_cfg(dev) ((struct acfcan_cfg *)((char *)(can_get_ml_priv(dev)) + sizeof(struct can_ml_priv)))

/* Device attribute handling */



static ssize_t dstmac_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (count == 1 && *buf == '\n') {
		return 1;
	}
	struct net_device *net_dev = to_net_dev(dev);

	// Check if the device is up
    if (netif_running(net_dev)) {
        printk(KERN_INFO "ACF-CAN: Cannot change dst mac while device %s is up\n", net_dev->name);
        return -EBUSY; // Return an appropriate error code
    }
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
	printk(KERN_INFO "ACF-CAN: Reading destination MAC\n");
	struct net_device *net_dev = to_net_dev(dev);
	struct acfcan_cfg *cfg = get_acfcan_cfg(net_dev);

	return sprintf(buf, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", cfg->dstmac[0],cfg->dstmac[1],cfg->dstmac[2],cfg->dstmac[3],cfg->dstmac[4],cfg->dstmac[5]);
}

// Only store the interface name. we will check if it is valid later when the interface is opened/upped
static ssize_t ethif_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (count == 1 && *buf == '\n') {
		return 1;
	}

	struct net_device *net_dev = to_net_dev(dev);

	// Check if the device is up
    if (netif_running(net_dev)) {
        printk(KERN_INFO "ACF-CAN: Cannot change interface while device %s is up\n", net_dev->name);
        return -EBUSY; // Return an appropriate error code
    }


	struct acfcan_cfg *cfg = get_acfcan_cfg(net_dev);

    if (count >= (IFNAMSIZ-1)) {
        printk(KERN_INFO "ACF-CAN: Interface name too long\n");
        return -EINVAL;
    }

    memcpy(cfg->ethif, buf, count);
    cfg->ethif[count] = '\0';

	printk(KERN_INFO "ACF-CAN Storing interface %s for %s\n", buf,net_dev->name);
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

static ssize_t streamid_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if (count == 1 && *buf == '\n') {
		return 1;
	}

	struct net_device *net_dev = to_net_dev(dev);

	// Check if the device is up
    if (netif_running(net_dev)) {
        printk(KERN_INFO "ACF-CAN: Cannot change stream id while device %s is up\n", net_dev->name);
        return -EBUSY; // Return an appropriate error code
    }


	struct acfcan_cfg *cfg = get_acfcan_cfg(net_dev);

    __u64 streamid;
    int rc = sscanf(buf, "%llx", &streamid);
    if (!rc) {
        printk(KERN_INFO "ACF-CAN: Invalid stream id\n");
        return -EINVAL;
    }

    cfg->streamid = streamid;


	printk(KERN_INFO "ACF-CAN setting streamid 0x%016llx for %s\n", streamid,net_dev->name);
	return count;
}


static ssize_t streamid_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct net_device *net_dev = to_net_dev(dev);
	struct acfcan_cfg *cfg = get_acfcan_cfg(net_dev);

	return sprintf(buf, "0x%016llx", cfg->streamid);
}



static DEVICE_ATTR_RW(dstmac);
static DEVICE_ATTR_RW(ethif);
static DEVICE_ATTR_RW(streamid);



static struct attribute *dev_attrs[] = {
    &dev_attr_dstmac.attr,
	&dev_attr_ethif.attr,
    &dev_attr_streamid.attr,
    NULL, /* NULL-terminated list */
};
