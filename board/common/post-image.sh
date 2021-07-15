#!/bin/sh

. $BR2_CONFIG 2>/dev/null

# Figure out version suffix for image files, default to empty suffix for
# developer builds.  After a long discussion, this turned out to be the
# least contentious alternative to: -dev, -devel, -HEAD, etc.
err=0
ver=""
if [ -n "$RELEASE" ]; then
    # NOTE: Must use `-f $BR2_EXTERNAL` here to get, e.g. app-demo GIT version
    ver="-$($BR2_EXTERNAL_NETBOX_PATH/bin/mkversion -f $BR2_EXTERNAL_DEMO_PATH)"

    if [ "-$RELEASE" != "$ver" ]; then
       echo "==============================================================================="
       echo "WARNING: Release verision '$RELEASE' does not match latest tag '$ver'!"
       echo "==============================================================================="
       err=1
    fi
fi

img="${BINARIES_DIR}/${BR2_TARGET_GENERIC_HOSTNAME}-${DEMO_PLATFORM}${ver}.img"
dir=$(dirname "$img")
md5=$dir/$(basename "$img" .img).md5

mv "$BINARIES_DIR"/rootfs.squashfs "$img"
md5sum "$img" > "$md5"

exit $err
