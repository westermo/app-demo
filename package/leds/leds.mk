################################################################################
#
# leds
#
################################################################################

LEDS_VERSION = 1
LEDS_LICENSE = MIT
LEDS_SITE_METHOD = local
LEDS_SITE = $(BR2_EXTERNAL_DEMO_PATH)/src/leds

define LEDS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		LDLIBS="$(TARGET_LDFLAGS)"
endef

define LEDS_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		DESTDIR="$(TARGET_DIR)" \
		PREFIX="usr" install
endef

$(eval $(generic-package))
