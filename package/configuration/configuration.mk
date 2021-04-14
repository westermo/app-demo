################################################################################
#
# configuration
#
################################################################################

CONFIGURATION_VERSION = 1
CONFIGURATION_LICENSE = MIT
CONFIGURATION_DEPENDENCIES = iproute2 jansson libev libteam nftables smcroute
CONFIGURATION_SITE_METHOD = local
CONFIGURATION_SITE = $(BR2_EXTERNAL_DEMO_PATH)/src/configuration

define CONFIGURATION_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		LDLIBS="$(TARGET_LDFLAGS)"
endef

define CONFIGURATION_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		DESTDIR="$(TARGET_DIR)" \
		PREFIX="usr" install
endef

define CONFIGURATION_INSTALL_INIT_SYSV
	$(INSTALL) -m 755 -D \
		$(BR2_EXTERNAL_DEMO_PATH)/package/configuration/S80configuration \
		$(TARGET_DIR)/etc/init.d/S80configuration
endef

$(eval $(generic-package))
