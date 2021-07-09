#!/bin/sh

TARGET_DIR=$1

# Add start of dhcp-boot to /etc/inittab if it doesn't already exist
if [ -e ${TARGET_DIR}/etc/inittab ]; then
    if ! grep -q "::sysinit:/usr/sbin/create-config.sh" ${TARGET_DIR}/etc/inittab; then
        sed -i '/now run any rc scripts/i\::sysinit:/usr/sbin/create-config.sh' ${TARGET_DIR}/etc/inittab;
    fi
fi
