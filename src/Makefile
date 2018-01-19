# Based on redis Makefile
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
uname_M := $(shell sh -c 'uname -m 2>/dev/null || echo not')
OPTIMIZATION?=-O2
NODEPS:=clean distclean


#default settings
STD=-std=c11 -pedantic
WARN=-Wall -W
OPT=$(OPTIMIZATION)
DBG=$(DEBUG)

PREFIX?=/usr/local
INSTALL_BIN=$(PREFIX)/bin
INSTALL=install
BUILDDIR=../build

FINAL_CFLAGS=$(STD) $(WARN) $(OPT) $(DBG) $(CFLAGS) -I ./
FINAL_LDFLAGS=$(LDFLAGS) $(DBG) -luv
FINAL_LIBS=-lm
DEBUG=-g -ggdb

ifeq ($(uname_S),Darwin)
# Darwin
	FINAL_LIBS+= -ldl
else
# All the other OSes (notably Linux)
	FINAL_LDFLAGS+= -rdynamic
	FINAL_LIBS+=-ldl -pthread
endif

DORENOM_CC=$(CC) $(FINAL_CFLAGS)
DORENOM_LD=$(CC) $(FINAL_LDFLAGS)

DORENOM_EXECUTABLE=dorenom
MONERO_OBJS=monero/monero_miner.o monero/monero_stratum.o
DORENOM_OBJS=buffer.o cli_opts.o config.o connection.o console.o currency.o cJSON/cJSON.o dorenom.o foreman.o miner.o stratum.o $(MONERO_OBJS)

all: $(DORENOM_EXECUTABLE)
.PHONY: all

%.o: %.c
	$(DORENOM_CC) -c $< -o $@

$(DORENOM_EXECUTABLE): $(DORENOM_OBJS)
	$(DORENOM_LD) -o $@ $^ $(FINAL_LIBS)

.PHONY: clean
clean:
	$(RM) $(DORENOM_EXECUTABLE) $(DORENOM_OBJS)


release:
	$(MAKE) DEBUG="-DNDEBUG" OPTIMIZATION="-O2"

debug:
	$(MAKE) DEBUG="-g -ggdb" OPTIMIZATION="-O0"

-include $(DEPS)
