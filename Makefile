CMAKE_ARGS :=

ifdef PREFIX
CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=$(shell readlink -f $(PREFIX))
endif

ROOT := $(shell pwd)
DEBUG := build/debug
RELEASE := build/release

DEBUG_TESTS ?= 1

build_all:
ifneq (,$(wildcard $(DEBUG)))
	$(MAKE) build_debug
endif
ifneq (,$(wildcard $(RELEASE)))
	$(MAKE) build_release
endif

ifdef g
GDB_PREFIX := gdb -ex run --args
endif

MAKE := $(MAKE) --no-print-directory -j$(shell nproc)

configure_debug:
	mkdir -p $(DEBUG)
	cd $(DEBUG) && cmake $(ROOT) -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
		-DBUILD_TESTS=$(DEBUG_TESTS) $(CMAKE_ARGS)

configure_release:
	mkdir -p $(RELEASE)
	cd $(RELEASE) && cmake $(ROOT) -DCMAKE_BUILD_TYPE=RelWithDebInfo $(CMAKE_ARGS)

configure: configure_debug configure_release

build_debug:
	$(MAKE) -C $(DEBUG)
	[ -f $(DEBUG)/tests/glogg_syntax_tests ] && $(DEBUG)/tests/glogg_syntax_tests --quiet
	./validate-yaml.py colors.schema.json config/*.glogg-colors.yaml
	./validate-yaml.py syntax.schema.json config/*.glogg-syntax.yaml
	$(DEBUG)/glogg --check-config ./config

run_debug: build_debug
	-pkill -f $(DEBUG)/glogg
	$(GDB_PREFIX) $(DEBUG)/glogg --server=debug -t -ddd sample/* -l log

syntax_tests:
	$(MAKE) -C $(DEBUG)/tests glogg_syntax_tests
	$(DEBUG)/tests/glogg_syntax_tests --gtest_filter="*$(f)*" -ddd

build_release:
	$(MAKE) -C $(RELEASE)

distclean_debug:
	rm -rf $(DEBUG)

distclean_release:
	rm -rf $(RELEASE)

install:
	$(MAKE) -C $(RELEASE) install

clean_debug:
ifneq (,$(wildcard $(DEBUG)))
	$(MAKE) -C $(DEBUG) clean
endif

clean_release:
ifneq (,$(wildcard $(RELEASE)))
	$(MAKE) -C $(RELEASE) clean
endif

clean: clean_debug clean_release