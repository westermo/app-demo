################################################################################
#
# dhcp-boot
#
################################################################################

DHCP_BOOT_VERSION = 1
DHCP_BOOT_LICENSE = MIT
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

$(eval $(generic-package))
