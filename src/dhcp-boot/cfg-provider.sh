#!/bin/sh

# Start our DHCP/TFTP server Dnsmasq
exec dnsmasq -C /etc/dnsmasq.conf
