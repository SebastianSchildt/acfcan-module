#pragma once

#include <linux/types.h> 
#include <linux/can/can-ml.h> // Include the CAN networking header
#include <net/net_trackers.h>

/* Private per-device configuration */
struct acfcan_cfg {
    __u8 dstmac[6];
    __u16 streamid;
    struct net_device *netdev;
    netdevice_tracker tracker;

};

#define get_acfcan_cfg(dev) ((struct acfcan_cfg *) ((char *)(can_get_ml_priv(dev)) + sizeof(struct can_ml_priv)))