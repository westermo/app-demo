#!/bin/sh

TARGET_DIR=$1

# This is a symlink to /usr/lib/os-release, so we remove this to keep
# original Buildroot information.
rm $TARGET_DIR/etc/os-release

echo "NAME=dhcp-boot"                               >$TARGET_DIR/etc/os-release
echo "VERSION=1.0.0"                              >>$TARGET_DIR/etc/os-release
echo "ID=dhcp-boot"                                >>$TARGET_DIR/etc/os-release
echo "VERSION_ID=1.0.0"                           >>$TARGET_DIR/etc/os-release
echo "PRETTY_NAME=\"Dhcp-boot Demo\""              >>$TARGET_DIR/etc/os-release

# Add start of dhcp-boot to /etc/inittab if it doesn't already exist
if [ -e ${TARGET_DIR}/etc/inittab ]; then
    if ! grep -q "::sysinit:/usr/sbin/create-config.sh" ${TARGET_DIR}/etc/inittab;
    then
        sed -i '/now run any rc scripts/i\::sysinit:/usr/sbin/create-config.sh' ${TARGET_DIR}/etc/inittab;
    fi
fi
