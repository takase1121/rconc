PREFIX ?= /usr/local

OPTIMIZATION ?= size
CFLAGS_OPTIMIZE_size = -Os
CFLAGS_OPTIMIZE_speed = -O3
CFLAGS_OPTIMIZE_native = -march=native -mtune=native
CFLAGS_OPTIMIZE_speed_native = -O3 -march=native -mtune=native

USE_COSMOCC ?= true
CC_COSMOCC_true := cosmocc/bin/cosmocc
CC_COSMOCC_false := $(CC)
CC_DEP_COSMOCC_true := $(CC_IF_COSMOCC_1)
CC_DEP_COSMOCC_false :=
LDLIBS_COSMOCC_true :=
LDLIBS_COSMOCC_false := -lc

CC = $(CC_COSMOCC_$(USE_COSMOCC))
CC_DEP = $(CC_DEP_COSMOCC_$(USE_COSMOCC))
LDLIBS = $(LDLIBS_COSMOCC_$(USE_COSMOCC))
CFLAGS = -Wall -Wextra -pedantic $(CFLAGS_OPTIMIZE_$(OPTIMIZATION))
LDFLAGS = -s

.PHONY: all
all: $(CC_DEP) rconc

rconc: rconc.c bestline.c

.PHONY: clean
clean:
	rm -f rconc *.o

.PHONY: install
install: rconc
	install -TDm0755 rconc $(DESTDIR)$(PREFIX)/bin/rconc

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/rconc

# extra rule to download cosmocc if not available
cosmocc/bin/cosmocc:
	wget -O cosmocc/cosmocc.zip "https://cosmo.zip/pub/cosmocc/cosmocc.zip"
	cd cosmocc; unzip cosmocc.zip