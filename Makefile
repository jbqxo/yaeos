export ROOT := $(shell pwd)

include confmk/common.mk

.PHONY: all build_dir kernel grub-iso test clean

all: kernel grub-iso test build_dir

build_dir:
	@$(MKDIRP) $(BUILDDIR_BUILD)

clean:
	@$(MAKE) -C $(DIR_KERNEL) clean
	@$(MAKE) -C $(DIR_BOOT) clean
	@$(MAKE) -C $(DIR_DEPS) clean

grub-iso: kernel | build_dir
	$(call log, [build] make grub iso)
	@$(MAKE) -C $(DIR_BOOT) $(BUILDDIR_BUILD)/grub.iso

kernel: | build_dir
	$(call log, [build] make kernel)
	@$(MAKE) -C $(DIR_KERNEL)

compile_commands:
	$(call log, [build] make compile commands entries)
	@$(MAKE) -C $(DIR_KERNEL) compile_commands

# Add unity headers
tests: | build_dir
	$(call log, [build] make tests)
	@$(MAKE) -C $(DIR_DEPS) unity
	@$(MAKE) -C $(DIR_KERNEL) tests
