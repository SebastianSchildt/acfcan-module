
#include "1722ethernet.h"

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>

#include "acfcandev.h"



int init_net_dev(char *name, struct net_device **dev)
{
    // Find the network device
    *dev = dev_get_by_name(&init_net, name);
    if (!dev) {
        printk(KERN_ERR "Device not found\n");
        return -ENODEV;
    }

    return 0;
}

int send_canfd_frame(struct net_device *ether_dev, struct net_device *can_dev, struct canfd_frame *cfd)
{
 	canid_t id = cfd->can_id;
    printk(KERN_INFO "Sending CAN FD packet,  ID: ");
	if (id & CAN_EFF_FLAG)
	{
		printk(KERN_CONT "0x%08x", id & CAN_EFF_MASK);
	}
	else
	{
		printk(KERN_CONT "0x%03x", id & CAN_SFF_MASK);
	}
    printk(KERN_CONT " Packet data: ");
    for (int i = 0; i < cfd->len; i++)
    {
        printk(KERN_CONT "%02x ", cfd->data[i]);
    }
    printk(KERN_CONT "\n");

    struct acfcan_cfg *cfg = get_acfcan_cfg(can_dev);
    void *canpriv = can_get_ml_priv(can_dev);
    
    printk(KERN_INFO "can Pointer is %i\n", (unsigned int)canpriv);
    printk(KERN_INFO "My  Pointer is %i, sizeof can priv is %i \n", (unsigned int)cfg, sizeof(struct can_ml_priv));
    int calc = (__u8 *)(canpriv)+sizeof(struct can_ml_priv);
    printk(KERN_INFO "My  data at %p\n", calc);


    __u8 *mac = cfg->dstmac;
    /*printk(KERN_INFO " BCAST Mac: ");
    for (int i = 0; i < 6; i++)
    {
        printk(KERN_CONT "%02x ", *(mac+i));
    }
    printk(KERN_CONT "\n");
    */

    return 0;
}



int send_can_frame(struct net_device *ether_dev, struct net_device *can_dev, struct can_frame *cf)
{
    uint16_t datalen=0;
 	canid_t id = cf->can_id;
    printk(KERN_INFO "Sending CAN  packet,  ID: ");
	if (id & CAN_EFF_FLAG)
	{
		printk(KERN_CONT "0x%08x", id & CAN_EFF_MASK);
        datalen+=4;
	}
	else
	{
		printk(KERN_CONT "0x%03x", id & CAN_SFF_MASK);
        datalen+=2;
	}
    printk(KERN_CONT " Packet data: ");
    for (int i = 0; i < cf->len; i++)
    {
        printk(KERN_CONT "%02x ", cf->data[i]);
    }
    printk(KERN_CONT "\n");

    //Prepare ethernet
    struct sk_buff *skb;
    unsigned char dest_mac[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; // Broadcast MAC address
    

    // Allocate a socket buffer
    skb = alloc_skb(ETH_HLEN + cf->len, GFP_KERNEL);
    if (!skb) {
        printk(KERN_ERR "Failed to allocate skb\n");
        dev_put(ether_dev);
        return -ENOMEM;
    }

    skb_reserve(skb, ETH_HLEN); // Reserve space for Ethernet header
    unsigned char *data = skb_put(skb, cf->len); // Add payload data
    // todo, add id first
    memcpy(data, cf->data, cf->len); // Fill payload with example data

    // Set up the Ethernet header

    struct ethhdr *eth = (struct ethhdr *)skb_push(skb, sizeof(struct ethhdr));

    uint8_t *bcast = ether_dev->broadcast;
    printk(KERN_INFO " BCAST Mac: ");
    for (int i = 0; i < 8; i++)
    {
        printk(KERN_CONT "%02x ", *(bcast+i));
    }
    printk(KERN_CONT "\n");


    memcpy(eth->h_dest, dest_mac, ETH_ALEN);
    memcpy(eth->h_source, ether_dev->dev_addr, ETH_ALEN);
    eth->h_proto = htons(0x22F0);

    // Set the network device
    skb->dev = ether_dev;
    skb->protocol = eth->h_proto;
    skb->ip_summed = CHECKSUM_NONE;

    // Send the frame
    printk(KERN_INFO "Sending Ethernet frame\n");
    dev_queue_xmit(skb);

    // Release the network device
    dev_put(ether_dev);


    return 0;
}


