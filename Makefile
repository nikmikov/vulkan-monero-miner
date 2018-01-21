# Top level makefile, the real shit is at src/Makefile

default: all

.DEFAULT:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

clang-format:
	find src/ -iname *.h -o -iname *.c | xargs clang-format -i

.PHONY: install
