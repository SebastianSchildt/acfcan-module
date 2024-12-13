# acfcan-module

Just me playing. 

```
export CONFIG_ACF_CAN=m
make
sudo insmod acfcan.ko && ./upfd.sh 
cansend test ABC#112233445566
sudo rmmod acfcan
```