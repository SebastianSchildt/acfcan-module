obj-$(CONFIG_ACF_CAN) += acfcan.o
acfcan-objs := 1722can.o 1722ethernet.o 


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean