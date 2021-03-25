################################################################################
#
# usbboot
#
################################################################################

USBBOOT_VERSION = 1
USBBOOT_LICENSE = MIT
USBBOOT_DEPENDENCIES = iproute2 jansson libev libteam nftables smcroute
USBBOOT_SITE_METHOD = local
USBBOOT_SITE = $(BR2_EXTERNAL_DEMO_PATH)/src/usbboot

define USBBOOT_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		LDLIBS="$(TARGET_LDFLAGS)"
endef

define USBBOOT_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		DESTDIR="$(TARGET_DIR)" \
		PREFIX="usr" install
endef

define USBBOOT_INSTALL_INIT_SYSV
	$(INSTALL) -m 755 -D \
		$(BR2_EXTERNAL_DEMO_PATH)/package/usbboot/S80usbboot \
		$(TARGET_DIR)/etc/init.d/S80usbboot
endef

$(eval $(generic-package))
