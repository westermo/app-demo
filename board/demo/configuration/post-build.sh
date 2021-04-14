#!/bin/sh

TARGET_DIR=$1

# This is a symlink to /usr/lib/os-release, so we remove this to keep
# original Buildroot information.
rm $TARGET_DIR/etc/os-release

echo "NAME=Configuration"                               >$TARGET_DIR/etc/os-release
echo "VERSION=1.0.0"                              >>$TARGET_DIR/etc/os-release
echo "ID=configuration"                                >>$TARGET_DIR/etc/os-release
echo "VERSION_ID=1.0.0"                           >>$TARGET_DIR/etc/os-release
echo "PRETTY_NAME=\"Configuration Demo\""              >>$TARGET_DIR/etc/os-release

# Provide symbolic names for routes added by the configuration daemon
if ! grep -q configuration $TARGET_DIR/etc/iproute2/rt_protos; then
    echo "254\tconfiguration" >>$TARGET_DIR/etc/iproute2/rt_protos
fi
