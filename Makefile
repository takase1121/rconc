PREFIX ?= /usr/local

LDLIBS = -lreadline -lc

OPTIMIZATION ?= size
CFLAGS_OPTIMIZE_size = -Os
CFLAGS_OPTIMIZE_speed = -O3
CFLAGS_OPTIMIZE_native = -march=native -mtune=native
CFLAGS_OPTIMIZE_speed_native = -O3 -march=native -mtune=native

CFLAGS = -Wall -Wextra -pedantic $(CFLAGS_OPTIMIZE_$(OPTIMIZATION))
LDFLAGS = -s

.PHONY: all
all: rconc

rconc: rconc.c

.PHONY: clean
clean:
	rm -f rconc *.o

.PHONY: install
install: rconc
	install -TDm0755 rconc $(DESTDIR)$(PREFIX)/bin/rconc

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/rconc
