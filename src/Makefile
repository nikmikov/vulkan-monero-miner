# Based on redis Makefile
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
uname_M := $(shell sh -c 'uname -m 2>/dev/null || echo not')
OPTIMIZATION?=-O2
NODEPS:=clean distclean


#default settings
STD=-std=c11 -pedantic -D_GNU_SOURCE
WARN=-Wall -W # -Werror
OPT=$(OPTIMIZATION)
DBG=$(DEBUG)

PREFIX?=/usr/local
INSTALL_BIN=$(PREFIX)/bin
INSTALL=install
BUILDDIR=../build

FINAL_CFLAGS=$(STD) $(WARN) $(OPT) $(DBG) -march=native $(CFLAGS) -I ./
FINAL_LDFLAGS=$(LDFLAGS) $(DBG) -luv
FINAL_LIBS=-lm
DEBUG=-g -ggdb

ifeq ($(uname_S),Darwin)
# Darwin
	FINAL_LIBS+= -ldl -framework OpenCL
else
# All the other OSes (notably Linux)
	FINAL_LDFLAGS+= -rdynamic
	FINAL_LIBS+=-ldl -pthread -lOpenCL -lvulkan
endif

DORENOM_CC=$(CC) $(FINAL_CFLAGS)
DORENOM_LD=$(CC) $(FINAL_LDFLAGS)

DORENOM_EXECUTABLE=dorenom
CRYPTONIGHT_OBJS=crypto/blake.o crypto/jh.o crypto/groestl.o crypto/cryptonight/cryptonight.o crypto/keccak-tiny.o crypto/skein.o crypto/cryptonight_implode_spv.o  crypto/cryptonight_init_spv.o crypto/cryptonight_keccak_spv.o crypto/cryptonight_explode_spv.o crypto/cryptonight_memloop_spv.o
MONERO_OBJS=monero/monero_config.o monero/monero_job.o monero/monero_miner.o monero/monero_solver.o monero/monero_stratum.o  monero/monero_solver_cl.o monero/monero_solver_cpu.o monero/monero_solver_vk.o $(CRYPTONIGHT_OBJS)
DORENOM_OBJS=buffer.o cli_opts.o config.o connection.o console.o currency.o cJSON/cJSON.o dorenom.o foreman.o miner.o stratum.o utils/opencl_err.o $(MONERO_OBJS)

CRYPTO_TESTS=crypto-tests
CRYPTO_TESTS_OBJS=crypto/crypto-tests.o $(CRYPTONIGHT_OBJS) console.o

all: $(DORENOM_EXECUTABLE)
.PHONY: all

test: $(CRYPTO_TESTS)
.PHONY: all

%.o: %.c
	$(DORENOM_CC) -c $< -o $@

$(DORENOM_EXECUTABLE): $(DORENOM_OBJS)
	$(DORENOM_LD) -o $@ $(DORENOM_OBJS) $(FINAL_LIBS)

$(CRYPTO_TESTS): $(CRYPTO_TESTS_OBJS)
	$(DORENOM_LD) -o $@ $^ $(FINAL_LIBS)

.PHONY: clean
clean:
	$(RM) $(DORENOM_EXECUTABLE) $(DORENOM_OBJS) $(CRYPTO_TESTS) $(CRYPTO_TESTS_OBJS)


release:
	$(MAKE) DEBUG="-DNDEBUG" OPTIMIZATION="-O3"

debug:
	$(MAKE) DEBUG="-g -ggdb" OPTIMIZATION="-O0"

-include $(DEPS)
