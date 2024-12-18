#include <linux/can.h>
#include <linux/can/dev.h>
#include <linux/can/skb.h>


//Todo store etherdev in prov
int send_canfd_frame(struct net_device *ether_dev, struct net_device *can_dev, struct canfd_frame *cfd);
int send_can_frame(struct net_device *ether_dev,struct net_device *can_dev, struct can_frame *cf);
