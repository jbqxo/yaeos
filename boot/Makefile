CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/conf.mk

KERNEL_BIN := $(BUILDDIR_ISODIR)/boot/kernel.bin
GRUB_CFG := $(BUILDDIR_ISODIR)/boot/grub/grub.cfg
DIR_USR := $(BUILDDIR_ISODIR)/usr

GRUB := grub-mkrescue

.PHONY: isodir_tree clean headers

$(BUILDDIR_BUILD)/grub.iso: $(KERNEL_BIN) $(GRUB_CFG) headers
	$(call log, [boot] Generating grub image)
	@$(GRUB) -o $@ $(BUILDDIR_ISODIR)

$(KERNEL_BIN): $(BUILDDIR_KERNEL)/kernel.bin | isodir_tree
	$(call log, [boot] Copying kernel binary into isodir/boot)
	@$(CPRP) $(BUILDDIR_KERNEL)/kernel.bin $(KERNEL_BIN)

$(GRUB_CFG): $(CWD)/grub.cfg | isodir_tree
	$(call log, [boot] Copying grub.cfg into isodir/boot/grub)
	@$(CPRP) $(CWD)/grub.cfg $(GRUB_CFG)

headers: $(BUILDDIR_KERNEL)/include $(BUILDDIR_LIBC)/include
	$(call log, [boot] Copying kernel and libc headers into isodir/usr/include)
	@$(MKDIRP) $(DIR_USR)/include
	@$(CPRP) $(BUILDDIR_LIBC)/include/. $(DIR_USR)/include/.
	@$(CPRP) $(BUILDDIR_KERNEL)/include/. $(DIR_USR)/include/.

isodir_tree:
	$(call log, [boot] Creating iso dir tree)
	@$(MKDIRP) $(BUILDDIR_ISODIR)
	@$(MKDIRP) $(BUILDDIR_ISODIR)/boot
	@$(MKDIRP) $(BUILDDIR_ISODIR)/boot/grub

clean:
	$(call log, [boot] Removing iso directory tree)
	@$(RMRF) $(BUILDDIR_ISODIR)
	$(call log, [boot] Removing grub.iso)
	@$(RMRF) $(BUILDDIR_BUILD)/grub.iso

$(BUILDDIR_KERNEL)/kernel.bin:
	$(warning [boot] kernel.bin is missing. Trying to build the kernel target.)
	@$(MAKE) -C ../$(DIR_KERNEL)
