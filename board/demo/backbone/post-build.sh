#!/bin/sh

TARGET_DIR=$1

# Provide symbolic names for routes added by the backbone daemon
if ! grep -q backbone $TARGET_DIR/etc/iproute2/rt_protos; then
    echo "254\tbackbone" >>$TARGET_DIR/etc/iproute2/rt_protos
fi
