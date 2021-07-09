#!/bin/sh
. $BR2_CONFIG 2>/dev/null

TARGET_DIR=$1

platform=$DEMO_PLATFORM
Platform=`echo $platform | awk '{print toupper(substr($0,0,1))tolower(substr($0,2))}'`

# This is a symlink to /usr/lib/os-release, so we remove this to keep
# original Buildroot information.
rm $TARGET_DIR/etc/os-release

echo "NAME=${DEMO_VENDOR_NAME}"               >$TARGET_DIR/etc/os-release
echo "VERSION=${DEMO_VENDOR_VERSION}"        >>$TARGET_DIR/etc/os-release
echo "ID=${DEMO_VENDOR_ID}"                  >>$TARGET_DIR/etc/os-release
echo "VERSION_ID=${DEMO_VENDOR_VERSION}"     >>$TARGET_DIR/etc/os-release
echo "PRETTY_NAME=\"${DEMO_VENDOR_DESC}\""   >>$TARGET_DIR/etc/os-release
echo "VARIANT=${Platform}"                   >>$TARGET_DIR/etc/os-release
echo "VARIANT_ID=${platform}"                >>$TARGET_DIR/etc/os-release
echo "HOME_URL=${DEMO_VENDOR_HOME}"          >>$TARGET_DIR/etc/os-release

printf "$DEMO_VENDOR_NAME $DEMO_VENDOR_VERSION -- `date +"%b %e %H:%M %Z %Y"`\n" > $TARGET_DIR/etc/version
