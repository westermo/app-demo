define([os_rootfs_overlay], $(BR2_EXTERNAL_DEMO_PATH)/board/demo/$(DEMO_VENDOR_ID)/rootfs)
include([netbox_app_dagger_defconfig.m4])
include([override.m4])
include([backbone.m4])
