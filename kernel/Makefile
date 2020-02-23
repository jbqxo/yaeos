CWD := $(shell pwd)

CFLAGS += \
	  -ffreestanding \
	  -fno-builtin \
	  -nostdinc

CPPFLAGS += \
	    -D__kernel__ \
	    -I$(CWD)/include \
	    -I$(CWD)/arch/$(TARGET)/include \
	    -I$(ROOT)/$(DIR_LIBC)/include \
	    -I$(ROOT)/$(DIR_LIBC)/arch/$(TARGET)/include
LDFLAGS += -nostdlib -lgcc

FILES_OBJ :=\
    $(addprefix $(PREFIX_KERNEL)/, \
    $(addsuffix .o, \
    $(shell $(FIND) kernel -name '*.c')))

DIR_ARCH := $(CWD)/arch/$(TARGET)
PREFIX_KERNEL_ARCH := $(subst $(CWD), $(PREFIX_KERNEL), $(DIR_ARCH))
include $(DIR_ARCH)/Makefile

.PHONY: all clean prefix_dirs install-headers
.SUFFIXES: .c .nasm .o .d

all: $(PREFIX_KERNEL)/kernel.bin install-headers

install-headers:
	$(info [kernel] Copying headers to the build directory)
	@$(MKDIRP) $(PREFIX_KERNEL)/include
	@$(CPRP) $(CWD)/include/. $(PREFIX_KERNEL)/include/.
	$(info [kernel] Copying platform-dependent headers ($(TARGET)) to the build directory)
	@$(MKDIRP) $(PREFIX_KERNEL)/include/arch
	@$(CPRP) $(CWD)/arch/$(TARGET)/include/. $(PREFIX_KERNEL)/include/.

clean:
	$(info [kernel] Removing kernel prefix directory)
	@$(RMRF) $(PREFIX_KERNEL)

prefix_dirs:
	@$(MKDIRP) $(dir $(FILES_OBJ))

$(PREFIX_KERNEL)/kernel.bin: $(FILES_OBJ) $(DIR_ARCH)/linker.ld $(PREFIX_LIBK)/libk.a
	$(info [kernel] LD $@)
	@$(CC) $(CFLAGS) $(LDFLAGS) -Wl,-T,$(DIR_ARCH)/linker.ld $(FILES_OBJ) $(PREFIX_LIBK)/libk.a -o $@

$(PREFIX_KERNEL_ARCH)/crtbegin.o $(PREFIX_KERNEL_ARCH)/crtend.o: | prefix_dirs
	$(info [kernel] Copy $(notdir $@) to the build directory)
	@CRT_O=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$CRT_O" $@

$(PREFIX_KERNEL)/%.c.o: $(CWD)/%.c | prefix_dirs
	$(info [kernel] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(PREFIX_KERNEL)/%.nasm.o: $(CWD)/%.nasm | prefix_dirs
	$(info [kernel] NASM $@)
	@$(AS) $< -o $@

$(PREFIX_KERNEL)/%.c.d: $(CWD)/%.c | prefix_dirs
	$(info [kernel] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -MM -MG -MT $(@:.d=.o) -MF $@ $<

-include $(FILES_OBJ:.o=.d)
