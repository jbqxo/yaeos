export ROOT := $(shell pwd)

include confmk/common.mk

.PHONY: all kernel libc run-qemu grub-iso deps test compiledb

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

# Add unity headers
test: export BUILD_TEST = 1
test: export BUILDDIR_BUILD = $(ROOT)/build/test
test: clean deps | build_dir
	$(call log, [build] make test)
	@$(MAKE) -C $(DIR_DEPS) unity
	@$(MAKE) -C $(DIR_LIBC) test
	@$(MAKE) -C $(DIR_KERNEL) test

compiledb:
	$(call log, [build] Creating compile_commands.json)
	@$(MAKE) clean
	@$(COMPILEDB) -o $(ROOT)/compile_commands.json make test
	@$(MAKE) clean
	@$(COMPILEDB) -o $(ROOT)/compile_commands.json make kernel
	@$(MAKE) clean
	@$(COMPILEDB) -o $(ROOT)/compile_commands.json make libc
