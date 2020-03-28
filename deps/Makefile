CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/conf.mk

# Suppress all warnings from dependencies
CFLAGS += -w

.PHONY: print all unity clean

all: unity

unity: $(BUILDDIR_DEPS)/unity/unity.o

prefix_dirs:
	@$(MKDIRP) $(BUILDDIR_DEPS)/unity

$(BUILDDIR_DEPS)/unity/%.o:	$(CWD)/unity/src/%.c | prefix_dirs
	$(info [deps.unity] CC $@ $(BUILD_TEST))
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	$(info [deps] Removing dependencies prefix directory)
	@$(RMRF) $(BUILDDIR_DEPS)
