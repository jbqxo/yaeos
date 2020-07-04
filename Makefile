export ROOT := $(shell pwd)

include confmk/common.mk

.PHONY: all build_dir kernel libc grub-iso test clean

all: kernel libc grub-iso test build_dir

build_dir:
	@$(MKDIRP) $(BUILDDIR_BUILD)

clean:
	@$(MAKE) -C $(DIR_KERNEL) clean
	@$(MAKE) -C $(DIR_LIBC) clean
	@$(MAKE) -C $(DIR_BOOT) clean
	@$(MAKE) -C $(DIR_DEPS) clean
	@$(RMRF) $(BUILDDIR_BUILD)/*

grub-iso: kernel libc | build_dir
	$(call log, [build] make grub iso)
	@$(MAKE) -C $(DIR_BOOT) $(BUILDDIR_BUILD)/grub.iso

kernel: libc | build_dir
	$(call log, [build] make kernel)
	@$(MAKE) -C $(DIR_KERNEL)

libc: | build_dir
	$(call log, [build] make libc)
	@$(MAKE) -C $(DIR_LIBC)

compile_commands:
	$(call log, [build] make compile commands entries)
	@$(MAKE) -C $(DIR_LIBC) compile_commands_libk
	@$(MAKE) -C $(DIR_KERNEL) compile_commands_kernel
	@$(MAKE) BUILD_TEST=1 BUILDDIR_BUILD=$(ROOT)/build/test -C $(DIR_LIBC) compile_commands_libk_tests
	@$(MAKE) BUILD_TEST=1 BUILDDIR_BUILD=$(ROOT)/build/test -C $(DIR_KERNEL) compile_commands_tests

# Add unity headers
test: export BUILD_TEST = 1
test: export BUILDDIR_BUILD = $(ROOT)/build/test
test: | build_dir
	$(call log, [build] make test)
	@$(MAKE) -C $(DIR_DEPS) unity
	@$(MAKE) -C $(DIR_LIBC) test
	@$(MAKE) -C $(DIR_KERNEL) test
