CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/confmk/common.mk

# Suppress all warnings from dependencies
CFLAGS += -w

.PHONY: all unity prefix_dirs clean

all: unity

unity: $(BUILDDIR_DEPS)/unity/unity.o

prefix_dirs:
	@$(MKDIRP) $(BUILDDIR_DEPS)/unity

$(BUILDDIR_DEPS)/unity/%.o: $(CWD)/unity/src/%.c | prefix_dirs
	$(call log, [deps.unity] CC $@ $(BUILD_TEST))
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	$(call log, [deps] Removing dependencies prefix directory)
	@$(RMRF) $(BUILDDIR_DEPS)
