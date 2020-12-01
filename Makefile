CMAKE_ARGS ?=

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

ifeq ($(shell uname -s),Darwin)
MAC_OS := yes
endif

MAKE := $(MAKE) --no-print-directory -j$(shell nproc)


export CMAKE_PREFIX_PATH = $(HOME)/Qt5.14.2/5.14.2/clang_64

submodule_update:
	git submodule update --init --recursive

configure_debug:
	mkdir -p $(DEBUG)
	cd $(DEBUG) && cmake $(ROOT) -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=YES \
		-DBUILD_TESTS=$(DEBUG_TESTS) -DMAC_BUNDLE=NO $(CMAKE_ARGS)
	ln -f $(DEBUG)/compile_commands.json

configure_release:
	mkdir -p $(RELEASE)
	cd $(RELEASE) && cmake $(ROOT) -DCMAKE_BUILD_TYPE=RelWithDebInfo $(CMAKE_ARGS)

configure: configure_debug configure_release

reconfigure:
ifneq (,$(wildcard $(DEBUG)))
	$(MAKE) configure_debug
endif
ifneq (,$(wildcard $(RELEASE)))
	$(MAKE) configure_release
endif

clean_reconfigure:
ifneq (,$(wildcard $(DEBUG)))
	rm -rf $(DEBUG) && $(MAKE) configure_debug
endif
ifneq (,$(wildcard $(RELEASE)))
	rm -rf $(RELEASE) && $(MAKE) configure_release
endif

build_debug:
	$(MAKE) -C $(DEBUG)
	[ -f $(DEBUG)/tests/glogg_syntax_tests ] && $(DEBUG)/tests/glogg_syntax_tests --quiet || true
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
ifdef MAC_OS
	$(MAKE) -C $(RELEASE) install
else
	$(MAKE) -C $(RELEASE)
endif

distclean_debug:
	rm -rf $(DEBUG)

distclean_release:
	rm -rf $(RELEASE)

install:
ifdef MAC_OS
	[[ "$$OSTYPE" == "darwin"* ]] && rm -rf /Applications/glogg.app && cp -r build/release/output/glogg.app /Applications
else
	$(MAKE) -C $(RELEASE) install
endif

cpack: install
	cd $(RELEASE) && cpack $(if $(VERBOSE),--verbose)

clean_debug:
ifneq (,$(wildcard $(DEBUG)))
	$(MAKE) -C $(DEBUG) clean
endif

clean_release:
ifneq (,$(wildcard $(RELEASE)))
	$(MAKE) -C $(RELEASE) clean
endif

clean: clean_debug clean_release
