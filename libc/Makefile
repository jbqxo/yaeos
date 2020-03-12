CWD := $(shell pwd)

CFLAGS += \
	  -ffreestanding
CPPFLAGS += \
	  -I$(CWD)/include \
	  -I$(CWD)/arch/$(TARGET)/include

CPPFLAGS_LIBC :=\
	-D__libc__

CPPFLAGS_LIBK :=\
	-D__libk__ \
	-I$(ROOT)/$(DIR_KERNEL)/include \
	-I$(ROOT)/$(DIR_KERNEL)/arch/$(TARGET)/include

FILES_TESTS_OBJ :=\
	$(addsuffix .o, \
	$(shell $(FIND) lib -name '*_tests.c'))
FILES_REL_OBJ :=\
	$(filter-out $(FILES_TESTS_OBJ), \
	$(addsuffix .o, \
	$(shell $(FIND) lib -name '*.c' -or -name '*.nasm')))
FILES_LIBC_OBJ := $(addprefix $(PREFIX_LIBC)/, $(FILES_REL_OBJ))
FILES_LIBK_OBJ := $(addprefix $(PREFIX_LIBK)/, $(FILES_REL_OBJ))

DIR_ARCH := $(CWD)/arch/$(TARGET)
include $(DIR_ARCH)/Makefile

.PHONY: clean dirs_libc dirs_libk dirs_tests all install-headers test

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
	$(info [lib] Removing tests prefix directory)
	@$(RMRF) $(PREFIX_TESTS)

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
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBC) -c $< -o $@

$(PREFIX_LIBC)/%.nasm.o: $(CWD)/%.nasm | dirs_libc
	$(info [libc] NASM $@)
	@$(AS) -D__libc__ $< -o $@

$(PREFIX_LIBK)/%.c.o: $(CWD)/%.c | dirs_libk
	$(info [libk] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBK) -c $< -o $@

$(PREFIX_LIBK)/%.nasm.o: $(CWD)/%.nasm | dirs_libk
	$(info [libk] NASM $@)
	@$(AS) -D__libk__ $< -o $@

$(PREFIX_LIBC)/%.c.d: $(CWD)/%.c | dirs_libc
	$(info [libc] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBC) -MM -MG -MT $(@:.d=.o) -MF $@ $<

$(PREFIX_LIBK)/%.c.d: $(CWD)/%.c | dirs_libk
	$(info [libk] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBK) -MM -MG -MT $(@:.d=.o) -MF $@ $<

$(PREFIX_LIBC)/%.nasm.d: $(CWD)/%.nasm | dirs_libc
	$(info [libc] MD $@)
	@$(AS) -D__libc__ -MG -MT $(@:.d=.o) -MD $@ $<

$(PREFIX_LIBK)/%.nasm.d: $(CWD)/%.nasm | dirs_libk
	$(info [libk] MD $@)
	@$(AS) -D__libk__ -MG -MT $(@:.d=.o) -MD $@ $<

.SECONDEXPANSION:
dirs_tests test: TESTS := $(patsubst %.c.o,%,$(FILES_TESTS_OBJ))
dirs_tests test: TESTS_LIBC := $(addprefix libc/, $(TESTS))
dirs_tests test: TESTS_LIBK := $(addprefix libk/, $(TESTS))
dirs_tests test: TESTS_LIBC := $(addprefix $(PREFIX_TESTS)/, $(TESTS_LIBC))
dirs_tests test: TESTS_LIBK := $(addprefix $(PREFIX_TESTS)/, $(TESTS_LIBK))
dirs_tests:
	$(info [libk.tests] Creating dir tree)
	$(info [libc.tests] Creating dir tree)
	@$(MKDIRP) $(dir $(TESTS_LIBC) $(TESTS_LIBK))
test: $$(TESTS_LIBC) $$(TESTS_LIBK) | dirs_tests
	@$(foreach t,$^, echo "[lib.tests] Running $t"; $t;)

$(PREFIX_TESTS)/libc/%_tests: $(CWD)/%_tests.c $(CWD)/%.c $(PREFIX_DEPS)/unity/unity.o | dirs_tests
	$(info [libc.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBC) $^ -o $@

$(PREFIX_TESTS)/libk/%_tests: $(CWD)/%_tests.c $(CWD)/%.c $(PREFIX_DEPS)/unity/unity.o | dirs_tests
	$(info [libk.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBK) $^ -o $@

-include $(FILES_LIBC_OBJ:.o=.d) $(FILES_LIBK_OBJ:.o=.d)
