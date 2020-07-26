CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/confmk/common.mk

KERNEL_BIN := $(BUILDDIR_ISODIR)/boot/kernel.bin
GRUB_CFG := $(BUILDDIR_ISODIR)/boot/grub/grub.cfg
DIR_USR := $(BUILDDIR_ISODIR)/usr

GRUB := grub-mkrescue

.PHONY: isodir_tree clean headers

$(BUILDDIR_BUILD)/grub.iso: $(KERNEL_BIN) $(GRUB_CFG)
	$(info [boot] Generating grub image)
	@$(GRUB) -o $@ $(BUILDDIR_ISODIR)

$(KERNEL_BIN): $(BUILDDIR_KERNEL)/kernel.bin | isodir_tree
	$(info [boot] Copying kernel binary into isodir/boot)
	@$(CPRP) $(BUILDDIR_KERNEL)/kernel.bin $(KERNEL_BIN)

$(GRUB_CFG): $(CWD)/grub.cfg | isodir_tree
	$(info [boot] Copying grub.cfg into isodir/boot/grub)
	@$(CPRP) $(CWD)/grub.cfg $(GRUB_CFG)

isodir_tree:
	$(info [boot] Creating iso dir tree)
	@$(MKDIRP) $(BUILDDIR_ISODIR)
	@$(MKDIRP) $(BUILDDIR_ISODIR)/boot
	@$(MKDIRP) $(BUILDDIR_ISODIR)/boot/grub

clean:
	$(info [boot] Removing iso directory tree)
	@$(RMRF) $(BUILDDIR_ISODIR)
	$(info [boot] Removing grub.iso)
	@$(RMRF) $(BUILDDIR_BUILD)/grub.iso

$(BUILDDIR_KERNEL)/kernel.bin:
	$(warning [boot] kernel.bin is missing. Trying to build the kernel target.)
	@$(MAKE) -C ../$(DIR_KERNEL)
