export BR2_EXTERNAL	:= $(CURDIR):$(CURDIR)/netbox
export PATH         := $(CURDIR)/utils:$(CURDIR)/netbox/utils:$(PATH)
export M4PATH		:= $(CURDIR)/configs/include:$(CURDIR)/netbox/configs:$(CURDIR)/netbox/configs/include:$(M4PATH)

# $(info $(BR2_EXTERNAL))

ARCH ?= $(shell uname -m)
O    ?= $(CURDIR)/output

config := $(O)/.config
bmake   = $(MAKE) -C netbox/buildroot O=$(O) $1
nmake   = $(MAKE) -C netbox O=$(O) $1


all: $(config) netbox/buildroot/Makefile
	@+$(call bmake,$@)

$(config):
	@+$(call bmake,list-defconfigs)
	@echo "ERROR: No configuration selected."
	@echo "Please choose a configuration from the list above by running"
	@echo "'make <board>_defconfig' before building an image."
	@exit 1

%_defconfig: configs/%_defconfig
	@+$(call bmake,$@)

configs/%_defconfig: configs/%_defconfig.m4 | netbox/buildroot/Makefile
	@echo "\e[7m>>>   Generating $(@F) -- ignore any \"reassigning to symbol\" those are AppDemo overrides.\e[0m"
	@gendefconfig -d $(@D) -i $(CURDIR)/configs -i $(CURDIR)/configs/include $(<F) >$@


#%: netbox/buildroot/Makefile
#	@+$(call bmake,$@)

netbox/buildroot/Makefile:
	@git submodule update --init --recursive

run diff-defconfig:
	@$(call nmake,$@)

.PHONY: all
