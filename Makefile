PREFIX ?= /usr/local
CC = cc
CFLAGS = -std=c99 -Wall -Wextra -pedantic -Os
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
