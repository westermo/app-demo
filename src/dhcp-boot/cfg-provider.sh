#!/bin/sh

# Set up the default interface
ip link set dev eth0 up
ip addr add 10.0.0.1/24 dev eth0

# Here is were we could start a program that would
# modify /mnt/config.cfg, or even generate it from scratch.

# Start our DHCP/TFTP server Dnsmasq
exec dnsmasq -C /etc/dnsmasq.conf
