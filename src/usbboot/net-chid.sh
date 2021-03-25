#!/bin/sh

id=$1

for iface in usbboot eth0; do
    ip neigh  flush dev $iface
    ip addr   flush dev $iface
done

tc filter del dev usbboot ingress
tc filter del dev usbboot egress
ip route flush proto usbboot

ip addr add 192.168.0.1/24 dev eth0

if [ $id -eq 0 ]; then
    # We're detached, so once everything is teared down we're done.
    exit 0
fi

# Translate the destination address of packets destined for this node
# to internal addresses.
tc filter add dev usbboot ingress protocol ip \
   u32 match ip dst 192.168.$id.0/24 \
   action \
      nat ingress 192.168.$id.0/24 192.168.0.0/24

# Translate the source address for local packets going out over the
# backbone.
tc filter add dev usbboot egress protocol ip \
   u32 match ip src 192.168.0.0/24 \
   action \
      nat egress 192.168.0.0/24 192.168.$id.0/24

ip addr add 192.168.255.$id/24 dev usbboot
ip addr add 192.168.$id.1/24   dev eth0

# Setup routes to all other networks, exempting the local one.
for net in $(seq 8 | grep -v $id); do
    ip route add 192.168.$net.0/24 via 192.168.255.$net proto usbboot
done
