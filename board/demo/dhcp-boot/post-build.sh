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
