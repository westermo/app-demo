################################################################################
#
# dhcp-boot
#
################################################################################

DHCP_BOOT_VERSION = 1
DHCP_BOOT_LICENSE = MIT
DHCP_BOOT_DEPENDENCIES = iproute2 jansson libev libteam nftables smcroute
DHCP_BOOT_SITE_METHOD = local
DHCP_BOOT_SITE = $(BR2_EXTERNAL_DEMO_PATH)/src/dhcp-boot

define DHCP_BOOT_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		LDLIBS="$(TARGET_LDFLAGS)"
endef

define DHCP_BOOT_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		DESTDIR="$(TARGET_DIR)" \
		PREFIX="usr" install
endef

define DHCP_BOOT_INSTALL_INIT_SYSV
	$(INSTALL) -m 755 -D \
		$(BR2_EXTERNAL_DEMO_PATH)/package/dhcp-boot/S80dhcp-boot \
		$(TARGET_DIR)/etc/init.d/S80dhcp-boot
endef

$(eval $(generic-package))
