# Based on redis Makefile
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
uname_M := $(shell sh -c 'uname -m 2>/dev/null || echo not')
OPTIMIZATION?=-O2
NODEPS:=clean distclean

#default settings
STD=-std=c99 -pedantic
WARN=-Wall -W -Wno-missing-field-initializers
OPT=$(OPTIMIZATION)

PREFIX?=/usr/local
INSTALL_BIN=$(PREFIX)/bin
INSTALL=install

FINAL_CFLAGS=$(STD) $(WARN) $(OPT) $(DEBUG) $(CFLAGS)
FINAL_LDFLAGS=$(LDFLAGS) $(DEBUG)
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
DORENOM_OBJS=dorenom.o

all: $(DORENOM_EXECUTABLE)
.PHONY: all

%.o: %.c
	$(DORENOM_CC) -c $<

$(DORENOM_EXECUTABLE): $(DORENOM_OBJS)
	$(DORENOM_LD) -o $@ $^ $(FINAL_LIBS)

.PHONY: clean
clean:
	$(RM) $(DORENOM_EXECUTABLE) *.o

distclean: clean
	-(cd ../deps && $(MAKE) distclean)
	-(rm -f .make-*)

.PHONY: distclean

noopt:
	$(MAKE) OPTIMIZATION="-O0"

-include $(DEPS)
