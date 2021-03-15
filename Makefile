export ROOT := $(shell pwd)

include confmk/common.mk

.PHONY: all build_dir kernel grub-iso tests clean raw.bin

all: kernel grub-iso tests build_dir

build_dir:
	@$(MKDIRP) $(BUILDDIR_BUILD)

clean:
	@$(MAKE) -C $(DIR_KERNEL) clean
	@$(MAKE) -C $(DIR_BOOT) clean
	@$(MAKE) -C $(DIR_DEPS) clean

raw.bin: kernel | build_dir
	$(OBJCOPY) -O binary $(BUILDDIR_KERNEL)/kernel.bin $(BUILDDIR_BUILD)/raw.bin

grub-iso: kernel | build_dir
	$(call log, [build] make grub iso)
	@$(MAKE) -C $(DIR_BOOT) $(BUILDDIR_BUILD)/grub.iso

kernel: | build_dir
	$(call log, [build] make kernel)
	@$(MAKE) -C $(DIR_DEPS) extract-offsets
	@$(MAKE) -C $(DIR_KERNEL)

# Add unity headers
tests: | build_dir
	$(call log, [build] make tests)
	@$(MAKE) -C $(DIR_DEPS) unity
	@$(MAKE) -C $(DIR_DEPS) extract-offsets
	@$(MAKE) -C $(DIR_KERNEL) tests
