################################################################################
#
# backbone
#
################################################################################

BACKBONE_VERSION = 1
BACKBONE_LICENSE = MIT
BACKBONE_DEPENDENCIES = iproute2 jansson libev libteam nftables smcroute
BACKBONE_SITE_METHOD = local
BACKBONE_SITE = $(BR2_EXTERNAL_DEMO_PATH)/src/backbone

define BACKBONE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		LDLIBS="$(TARGET_LDFLAGS)"
endef

define BACKBONE_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		DESTDIR="$(TARGET_DIR)" \
		PREFIX="usr" install
endef

define BACKBONE_INSTALL_INIT_SYSV
	$(INSTALL) -m 755 -D \
		$(BR2_EXTERNAL_DEMO_PATH)/package/backbone/S80backbone \
		$(TARGET_DIR)/etc/init.d/S80backbone
endef

$(eval $(generic-package))
