#!/bin/sh

# VID to use on the backbone.
vid=$1

# Add a VLAN filtering bridge with spanning tree disabled. Assign all
# ports, including the bridge itself, to the supplied VID.
ip link add dev usbboot type bridge \
   stp_state 0 \
   vlan_filtering 1 \
   vlan_default_pvid $vid


# Attach the link aggregates to the bridge.
for lag in west east; do
    ip link set dev $lag master usbboot
    ip link set dev $lag up
done

# The clsact qdisc allows classifier/action filters to be attached to
# an interface's ingress/egress processing without the need for any
# "real" qdiscs.
tc qdisc add dev usbboot clsact

# Bring up the bridge and the local LAN, assign the local address of
# the router.
ip link set dev usbboot up
ip link set dev eth0     up
ip addr add 192.168.0.1/24 dev eth0
