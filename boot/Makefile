CWD := $(shell pwd)

KERNEL_BIN := $(PREFIX_ISODIR)/boot/kernel.bin
GRUB_CFG := $(PREFIX_ISODIR)/boot/grub/grub.cfg
DIR_USR := $(PREFIX_ISODIR)/usr

GRUB := grub-mkrescue

.PHONY: isodir_tree clean headers

$(PREFIX_BUILD)/grub.iso: $(KERNEL_BIN) $(GRUB_CFG) headers
	$(info [boot] Generating grub image)
	@$(GRUB) -o $@ $(PREFIX_ISODIR)

$(KERNEL_BIN): $(PREFIX_KERNEL)/kernel.bin | isodir_tree
	$(info [boot] Copying kernel binary into isodir/boot)
	@$(CPRP) $(PREFIX_KERNEL)/kernel.bin $(KERNEL_BIN)

$(GRUB_CFG): $(CWD)/grub.cfg | isodir_tree
	$(info [boot] Copying grub.cfg into isodir/boot/grub)
	@$(CPRP) $(CWD)/grub.cfg $(GRUB_CFG)

headers: $(PREFIX_KERNEL)/include $(PREFIX_LIBC)/include
	$(info [boot] Copying kernel and libc headers into isodir/usr/include)
	@$(MKDIRP) $(DIR_USR)/include
	@$(CPRP) $(PREFIX_LIBC)/include/. $(DIR_USR)/include/.
	@$(CPRP) $(PREFIX_KERNEL)/include/. $(DIR_USR)/include/.

isodir_tree:
	$(info [boot] Creating iso dir tree)
	@$(MKDIRP) $(PREFIX_ISODIR)
	@$(MKDIRP) $(PREFIX_ISODIR)/boot
	@$(MKDIRP) $(PREFIX_ISODIR)/boot/grub

clean:
	$(info [boot] Removing iso directory tree)
	@$(RMRF) $(PREFIX_ISODIR)
	$(info [boot] Removing grub.iso)
	@$(RMRF) $(PREFIX_BUILD)/grub.iso

$(PREFIX_KERNEL)/kernel.bin:
	$(warning [boot] kernel.bin is missing. Trying to build the kernel target.)
	@$(MAKE) -C ../$(DIR_KERNEL)
