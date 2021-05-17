#!/bin/sh

# In this example we only change the hostname in a WeOS config file.
# The pre-config.cfg must have been created
if [ -e /mnt/pre-config.cfg ]; then
    sed 's/"hostname\": ".*"/"hostname": "Host_name_from_app"/' /mnt/pre-config.cfg > /mnt/config.cfg;
fi
