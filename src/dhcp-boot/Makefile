CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter -Wno-strict-aliasing
CPPFLAGS += -D_GNU_SOURCE
LOADLIBES += -lev

all: dhcp-boot

dhcp-boot:

install:
	@mkdir -p \
		$(DESTDIR)/$(PREFIX)/sbin/ \
		$(DESTDIR)/$(PREFIX)/lib/dhcp-boot/
	@cp dhcp-boot $(DESTDIR)/$(PREFIX)/sbin/
