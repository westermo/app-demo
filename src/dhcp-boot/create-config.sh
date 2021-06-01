#!/bin/sh

# In this example we only change the hostname in a WeOS config file.
# The pre-config.cfg must have been created. The path for config.cfg,
# i.e. /tmp must bee consistent with tftp-root in dnsmasq.conf.
if [ -e /mnt/pre-config.cfg ]; then
    sed 's/"hostname\": ".*"/"hostname": "Host_name_from_app"/' /mnt/pre-config.cfg > /tmp/config.cfg;
fi
