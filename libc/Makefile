CWD := $(shell pwd)

CFLAGS += \
	  -ffreestanding
CPPFLAGS += \
	  -I$(CWD)/include

FILES_REL_OBJ :=\
    $(addsuffix .o, \
    $(basename \
    $(shell $(FIND) lib -name '*.c' -or -name '*.S')))
FILES_LIBC_OBJ := $(addprefix $(PREFIX_LIBC)/, $(FILES_REL_OBJ))
FILES_LIBK_OBJ := $(addprefix $(PREFIX_LIBK)/, $(FILES_REL_OBJ))

DIR_ARCH := $(CWD)/arch/$(TARGET)
include $(DIR_ARCH)/Makefile

.PHONY: clean dirs_libc dirs_libk all install-headers

all: $(PREFIX_LIBC)/lib.a $(PREFIX_LIBK)/lib.a install-headers

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
	$(info [libc] Copying headers to the build directory)
	@$(MKDIRP) $(PREFIX_LIBC)/include
	@$(CPRP) $(CWD)/include/. $(PREFIX_LIBC)/include/.

$(PREFIX_LIBC)/lib.a: $(FILES_LIBC_OBJ) | dirs_libc
	$(info [libc] Archiving $(notdir $(FILES_LIBC_OBJ)))
	@$(AR) rcs $@ $^

$(PREFIX_LIBK)/lib.a: $(FILES_LIBK_OBJ) | dirs_libk
	$(info [libk] Archiving $(notdir $(FILES_LIBK_OBJ)))
	@$(AR) rcs $@ $^

$(PREFIX_LIBC)/%.o: $(CWD)/%.c | dirs_libc
	$(info [libc] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libc__ -c $< -o $@

$(PREFIX_LIBC)/%.o: $(CWD)/%.S | dirs_libc
	$(info [libc] AS $@)
	@$(AS) $(CFLAGS) $(CPPFLAGS) -D__libc__ -c $< -o $@

$(PREFIX_LIBK)/%.o: $(CWD)/%.c | dirs_libk
	$(info [libk] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ -c $< -o $@

$(PREFIX_LIBK)/%.o: $(CWD)/%.S | dirs_libk
	$(info [libk] AS $@)
	@$(AS) $(CFLAGS) $(CPPFLAGS) -D__libk__ -c $< -o $@

$(PREFIX_LIBC)/%.d: $(CWD)/%.c | dirs_libc
	$(info [libc] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libc__ -MM -MG -MF $@ $<

$(PREFIX_LIBK)/%.d: $(CWD)/%.c | dirs_libk
	$(info [libk] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ -MM -MG -MF $@ $<


-include $(FILES_LIBC_OBJ:.o=.d) $(FILES_LIBK_OBJ:.o=.d)
