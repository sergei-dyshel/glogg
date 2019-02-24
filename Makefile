CMAKE_ARGS :=

ifdef PREFIX
CMAKE_ARGS += -DCMAKE_INSTALL_PREFIX=$(shell readlink -f $(PREFIX))
endif

ROOT := $(shell pwd)
DEBUG := build/debug
RELEASE := build/release

all:
ifneq (,$(wildcard $(DEBUG)))
	$(MAKE) debug
endif
ifneq (,$(wildcard $(RELEASE)))
	$(MAKE) release
endif

MAKE := $(MAKE) --no-print-directory

configure_debug:
	mkdir -p $(DEBUG)
	cd $(DEBUG) && cmake $(ROOT) -DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=YES $(CMAKE_ARGS)

configure_release:
	mkdir -p $(RELEASE)
	cd $(RELEASE) && cmake $(ROOT) -DCMAKE_BUILD_TYPE=RelWithDebInfo $(CMAKE_ARGS)

configure: configure_debug configure_release

debug:
	$(MAKE) -C $(DEBUG)
	$(DEBUG)/tests/glogg_syntax_tests --quiet

release:
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