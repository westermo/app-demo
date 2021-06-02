CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter -Wno-strict-aliasing
CPPFLAGS += -D_GNU_SOURCE
# LOADLIBES += 

all: led-relay

led-relay: led-relay.c

install:
	@mkdir -p \
		$(DESTDIR)/$(PREFIX)/sbin/ \
	@cp backbone $(DESTDIR)/$(PREFIX)/sbin/
