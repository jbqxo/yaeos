CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/confmk/common.mk

# Suppress all warnings from dependencies
CFLAGS_COMMON += -w

CFLAGS_COMPLETE := \
    $(CFLAGS_COMMON) \
    $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE)) \
    $(CFLAGS)

CPPFLAGS_COMPLETE := \
    $(CPPFLAGS_COMMON) \
    $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE)) \
    $(CPPFLAGS)

.PHONY: all unity extract-offsets prefix_dirs clean

all: unity extract-offsets

extract-offsets: $(BUILDDIR_DEPS)/extract-offsets/extract_offsets.so

$(BUILDDIR_DEPS)/extract-offsets/extract_offsets.so: $(CWD)/extract-offsets
	$(info [deps.extract_offsets] MAKE -C $< install)
	@$(MAKE) -C $< DESTDIR=$(BUILDDIR_DEPS)/extract-offsets TARGET_GCC=$(CC) install

unity: $(BUILDDIR_DEPS)/unity/unity.o

prefix_dirs:
	@$(MKDIRP) $(BUILDDIR_DEPS)/unity
	@$(MKDIRP) $(BUILDDIR_DEPS)/unity

$(BUILDDIR_DEPS)/unity/%.o: $(CWD)/unity/src/%.c | prefix_dirs
	$(info [deps.unity] CC $@ $(BUILD_TEST))
	@$(CC) $(CFLAGS_COMPLETE) $(CPPFLAGS_COMPLETE) -c $< -o $@

clean:
	$(info [deps] Removing dependencies prefix directory)
	@$(RMRF) $(BUILDDIR_DEPS)
