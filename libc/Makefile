CWD := $(shell pwd)

CFLAGS += \
	  -ffreestanding
CPPFLAGS += \
	  -I$(CWD)/include \
	  -I$(CWD)/arch/$(TARGET)/include

FILES_REL_OBJ :=\
    $(addsuffix .o, \
    $(shell $(FIND) lib -name '*.c' -or -name '*.nasm'))
FILES_LIBC_OBJ := $(addprefix $(PREFIX_LIBC)/, $(FILES_REL_OBJ))
FILES_LIBK_OBJ := $(addprefix $(PREFIX_LIBK)/, $(FILES_REL_OBJ))

DIR_ARCH := $(CWD)/arch/$(TARGET)
include $(DIR_ARCH)/Makefile

.PHONY: clean dirs_libc dirs_libk all install-headers

all: $(PREFIX_LIBC)/libc.a $(PREFIX_LIBK)/libk.a install-headers

dirs_libc:
	$(info [libc] Creating dir tree)
	@$(MKDIRP) $(dir $(FILES_LIBC_OBJ))

dirs_libk:
	$(info [libk] Creating dir tree)
	@$(MKDIRP) $(dir $(FILES_LIBK_OBJ))

clean:
	$(info [libc] Removing libc prefix directory)
	@$(RMRF) $(PREFIX_LIBC)
	$(info [libk] Removing libk prefix directory)
	@$(RMRF) $(PREFIX_LIBK)

install-headers:
	@$(MKDIRP) $(PREFIX_LIBC)/include
	$(info [libc] Copying headers to the build directory)
	@$(CPRP) $(CWD)/include/. $(PREFIX_LIBC)/include/.
	$(info [libc] Copying platform-dependent headers ($(TARGET)) to the build directory)
	@$(MKDIRP) $(PREFIX_LIBC)/include/arch
	@$(CPRP) $(CWD)/arch/$(TARGET)/include/. $(PREFIX_LIBC)/include/.

$(PREFIX_LIBC)/libc.a: $(FILES_LIBC_OBJ) | dirs_libc
	$(info [libc] Archiving $(notdir $(FILES_LIBC_OBJ)))
	@$(AR) rcs $@ $^

$(PREFIX_LIBK)/libk.a: $(FILES_LIBK_OBJ) | dirs_libk
	$(info [libk] Archiving $(notdir $(FILES_LIBK_OBJ)))
	@$(AR) rcs $@ $^

$(PREFIX_LIBC)/%.c.o: $(CWD)/%.c | dirs_libc
	$(info [libc] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libc__ -c $< -o $@

$(PREFIX_LIBC)/%.nasm.o: $(CWD)/%.nasm | dirs_libc
	$(info [libc] NASM $@)
	@$(AS) -D__libc__ $< -o $@

$(PREFIX_LIBK)/%.c.o: $(CWD)/%.c | dirs_libk
	$(info [libk] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ -c $< -o $@

$(PREFIX_LIBK)/%.nasm.o: $(CWD)/%.nasm | dirs_libk
	$(info [libk] NASM $@)
	@$(AS) -D__libk__ $< -o $@

$(PREFIX_LIBC)/%.c.d: $(CWD)/%.c | dirs_libc
	$(info [libc] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libc__ -MM -MG -MT $(@:.d=.o) -MF $@ $<

$(PREFIX_LIBK)/%.c.d: $(CWD)/%.c | dirs_libk
	$(info [libk] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ -MM -MG -MT $(@:.d=.o) -MF $@ $<

$(PREFIX_LIBC)/%.nasm.d: $(CWD)/%.nasm | dirs_libc
	$(info [libc] MD $@)
	@$(AS) -D__libc__ -MG -MT $(@:.d=.o) -MD $@ $<

$(PREFIX_LIBK)/%.nasm.d: $(CWD)/%.nasm | dirs_libk
	$(info [libk] MD $@)
	@$(AS) -D__libk__ -MG -MT $(@:.d=.o) -MD $@ $<


-include $(FILES_LIBC_OBJ:.o=.d) $(FILES_LIBK_OBJ:.o=.d)
