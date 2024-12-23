sudo ip link add dev test type acfcan
sudo ip link set test mtu 72
sudo echo -n "lo"  | sudo tee /sys/class/net/test/acfcan/ethif
sudo ip link set up test
