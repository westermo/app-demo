BR2_DL_DIR="$(BR2_EXTERNAL_DEMO_PATH)/dl"
BR2_CCACHE_DIR="$(BR2_EXTERNAL_DEMO_PATH)/.buildroot-ccache"

define(DEMO_VENDOR_HOME, "https://github.com/westermo/app-demo/")
define(DEMO_VENDOR_DESC, "Application Demo - Westermo LXC container")
define(DEMO_VENDOR_VERSION, "`$BR2_EXTERNAL_NETBOX_PATH/bin/mkversion $BR2_EXTERNAL_DEMO_PATH`")

dnl Apply NetBox patches first, then AppDemo patches
BR2_GLOBAL_PATCH_DIR="$(BR2_EXTERNAL_NETBOX_PATH)/patches $(BR2_EXTERNAL_DEMO_PATH)/patches"

dnl Install NetBox common rootfs + platform rootfs, then the same from AppDemo
format([BR2_ROOTFS_OVERLAY="$(BR2_EXTERNAL_NETBOX_PATH)/board/common/rootfs %s $(BR2_EXTERNAL_DEMO_PATH)/board/demo/$(DEMO_VENDOR_ID)/rootfs %s"],
	ifdef([os_rootfs_overlay], os_rootfs_overlay,),
	ifdef([os_rootfs_overlay_extra], os_rootfs_overlay_extra,))

dnl Run NetBox post-build and post-image script first.  That way we can
dnl rely on the default experience from NetBox in AppDemo and just build
dnl on that, or adjust from it.
BR2_ROOTFS_POST_BUILD_SCRIPT="$(BR2_EXTERNAL_NETBOX_PATH)/board/common/post-build.sh $(BR2_EXTERNAL_DEMO_PATH)/board/common/post-build.sh $(BR2_EXTERNAL_DEMO_PATH)/board/demo/$(DEMO_VENDOR_ID)/post-build.sh"
BR2_ROOTFS_POST_IMAGE_SCRIPT="$(BR2_EXTERNAL_NETBOX_PATH)/board/common/post-image.sh $(BR2_EXTERNAL_NETBOX_PATH)/board/$(NETBOX_PLAT)/post-image.sh $(BR2_EXTERNAL_DEMO_PATH)/board/common/post-image.sh $(BR2_EXTERNAL_DEMO_PATH)/board/demo/$(DEMO_VENDOR_ID)/post-image.sh"
