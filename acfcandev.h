#pragma once

#include <linux/types.h> 
#include <linux/can/can-ml.h> // Include the CAN networking header



struct acfcan_cfg {
    __u8 dstmac[6];
    __u16 streamid;
};

#define get_acfcan_cfg(dev) ((struct acfcan_cfg *) ((char *)(can_get_ml_priv(dev)) + sizeof(struct can_ml_priv)))