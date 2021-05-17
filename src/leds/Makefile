CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter -Wno-strict-aliasing
LOADLIBES += -lev

all: leds

leds: leds.o

install:
	@mkdir -p \
		$(DESTDIR)/$(PREFIX)/sbin/
	@cp leds $(DESTDIR)/$(PREFIX)/sbin/
