#include <linux/can.h>
#include <linux/can/dev.h>
#include <linux/can/skb.h>

int send_canfd_frame(struct net_device *dev, struct canfd_frame *cfd);
int send_can_frame(struct net_device *dev, struct can_frame *cf);
