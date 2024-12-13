
#include "1722ethernet.h"

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>



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

int send_canfd_frame(struct net_device *dev, struct canfd_frame *cfd)
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

    return 0;
}



int send_can_frame(struct net_device *dev, struct can_frame *cf)
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
        dev_put(dev);
        return -ENOMEM;
    }

    skb_reserve(skb, ETH_HLEN); // Reserve space for Ethernet header
    unsigned char *data = skb_put(skb, cf->len); // Add payload data
    // todo, add id first
    memcpy(data, cf->data, cf->len); // Fill payload with example data

    // Set up the Ethernet header

    struct ethhdr *eth = (struct ethhdr *)skb_push(skb, sizeof(struct ethhdr));


    memcpy(eth->h_dest, dest_mac, ETH_ALEN);
    memcpy(eth->h_source, dev->dev_addr, ETH_ALEN);
    eth->h_proto = htons(0x22F0);

    // Set the network device
    skb->dev = dev;
    skb->protocol = eth->h_proto;
    skb->ip_summed = CHECKSUM_NONE;

    // Send the frame
    printk(KERN_INFO "Sending Ethernet frame\n");
    dev_queue_xmit(skb);

    // Release the network device
    dev_put(dev);


    return 0;
}


