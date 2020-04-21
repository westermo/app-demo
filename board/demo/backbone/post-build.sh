#!/bin/sh

TARGET_DIR=$1

# This is a symlink to /usr/lib/os-release, so we remove this to keep
# original Buildroot information.
rm $TARGET_DIR/etc/os-release

echo "NAME=Backbone"                               >$TARGET_DIR/etc/os-release
echo "VERSION=1.0.0"                              >>$TARGET_DIR/etc/os-release
echo "ID=backbone"                                >>$TARGET_DIR/etc/os-release
echo "VERSION_ID=1.0.0"                           >>$TARGET_DIR/etc/os-release
echo "PRETTY_NAME=\"Backbone Demo\""              >>$TARGET_DIR/etc/os-release

# Provide symbolic names for routes added by the backbone daemon
if ! grep -q backbone $TARGET_DIR/etc/iproute2/rt_protos; then
    echo "254\tbackbone" >>$TARGET_DIR/etc/iproute2/rt_protos
fi
