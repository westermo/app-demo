RELEASE   ?= $(shell git describe --tags)
EXEC      := led-relay
OBJS      := led-relay.o
SRCS      := $(OBJS:.o=.c)
DEPS      := $(addprefix .,$(SRCS:.c=.d))
CFLAGS    += -W -Wall -Wextra -Werror
CPPFLAGS  += -D_GNU_SOURCE -DVERSION=\"$(RELEASE)\"

.%.d: %.c
	@$(SHELL) -ec "$(CC) -MM $(CFLAGS) $(CPPFLAGS) $<	\
		| sed 's,.*: ,$*.o $@ : ,g' > $@;		\
		[ -s $@ ] || rm -f $@"

all: $(EXEC)

$(EXEC): $(OBJS)

clean:
	$(RM) $(OBJS) $(EXEC)

distclean: clean
	$(RM) *.o *.bak *~

install:
	@cp led-relay $(DESTDIR)/$(PREFIX)/sbin/

uninstall:
	$(RM) $(DESTDIR)/$(PREFIX)/sbin/

.PHONY: all clean distclean install

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),distclean)
-include $(DEPS)
endif
endif
