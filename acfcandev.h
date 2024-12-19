#pragma once

#include <linux/types.h>
#include <linux/can/can-ml.h>
#include <net/net_trackers.h>

/* Private per-device configuration */
struct acfcan_cfg
{
    __u8 dstmac[6]; //send acf-can frames to this mac
    __u16 streamid; //use acf-can stream-id
    struct net_device *netdev; //use this interface for acf-can frames
    netdevice_tracker tracker;
};

// get the acfcan_cfg struct from the device
#define get_acfcan_cfg(dev) ((struct acfcan_cfg *)((char *)(can_get_ml_priv(dev)) + sizeof(struct can_ml_priv)))