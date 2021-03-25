CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter -Wno-strict-aliasing
# Needed for sendmmsg(2)
CPPFLAGS += -D_GNU_SOURCE
LOADLIBES += -lev -ljansson -lteam

all: usbboot

usbboot: lag.o

install:
	@mkdir -p \
		$(DESTDIR)/$(PREFIX)/sbin/ \
		$(DESTDIR)/$(PREFIX)/lib/usbboot/
	@cp usbboot $(DESTDIR)/$(PREFIX)/sbin/
	@cp net-chid.sh $(DESTDIR)/$(PREFIX)/lib/usbboot/
	@cp net-up.sh   $(DESTDIR)/$(PREFIX)/lib/usbboot/
