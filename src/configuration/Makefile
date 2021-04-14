CFLAGS += -Wall -Wextra -Werror -Wno-unused-parameter -Wno-strict-aliasing
CPPFLAGS += -D_GNU_SOURCE
LOADLIBES += -lev -ljansson

all: configuration

configuration:

install:
	@mkdir -p \
		$(DESTDIR)/$(PREFIX)/sbin/ \
		$(DESTDIR)/$(PREFIX)/lib/configuration/
	@cp configuration $(DESTDIR)/$(PREFIX)/sbin/
