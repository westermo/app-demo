################################################################################
#
# led-relay
#
################################################################################

LED_RELAY_VERSION = 1
LED_RELAY_LICENSE = MIT
LED_RELAY_SITE_METHOD = local
LED_RELAY_SITE = $(BR2_EXTERNAL_DEMO_PATH)/src/led-relay

define LED_RELAY_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		LDLIBS="$(TARGET_LDFLAGS)"
endef

define LED_RELAY_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		DESTDIR="$(TARGET_DIR)" \
		PREFIX="usr" install
endef

$(eval $(generic-package))
