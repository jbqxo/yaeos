CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/confmk/common.mk
.PHONY: clean dirs_libc dirs_libk dirs_tests all install-headers test test_libc test_libk compile_commands_libk compile_commands_libk_tests
.SECONDEXPANSION:

DIR_ARCH := $(CWD)/arch/$(TARGET_ARCH)
include $(DIR_ARCH)/Makefile

CFLAGS += \
    $(CFLAGS_COMMON) \
    $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE)) \
    -ffreestanding

CPPFLAGS += \
    -I$(CWD)/include \
    -I$(CWD)/arch/$(TARGET_ARCH)/include \
    -I$(ROOT)/$(DIR_KERNEL)/include \
    -I$(ROOT)/$(DIR_KERNEL)/arch/$(TARGET_ARCH)/include \
    $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE)) \
    -I$(ROOT)/$(DIR_DEPS)

LDFLAGS += \
    $(LDFLAGS_COMMON) \
    $(if $(BUILD_DEBUG),$(LDFLAGS_DEBUG),$(LDFLAGS_RELEASE))

# TODO Ignore all architectures except for current TARGET
FILES_TESTS_OBJ :=\
    $(addsuffix .o, \
    $(shell $(FIND) lib -name '*_tests.c'))
FILES_REL_OBJ :=\
    $(filter-out $(FILES_TESTS_OBJ), \
    $(addsuffix .o, \
    $(shell $(FIND) lib -name '*.c' -or -name '*.asm')))

dirs_libc $(BUILDDIR_LIBC)/libc.a: FILES_OBJ := $(addprefix $(BUILDDIR_LIBC)/, $(FILES_REL_OBJ))
dirs_libk $(BUILDDIR_LIBK)/libk.a: FILES_OBJ := $(addprefix $(BUILDDIR_LIBK)/, $(FILES_REL_OBJ))

###### Phony rules

all: $(BUILDDIR_LIBC)/libc.a $(BUILDDIR_LIBK)/libk.a install-headers

dirs_libc:
	$(call log, [libc] Creating dir tree)
	@$(MKDIRP) $(dir $(FILES_OBJ))

dirs_libk:
	$(call log, [libk] Creating dir tree)
	@$(MKDIRP) $(dir $(FILES_OBJ))

clean:
	$(call log, [libc] Removing libc prefix directory)
	@$(RMRF) $(BUILDDIR_LIBC)
	$(call log, [libk] Removing libk prefix directory)
	@$(RMRF) $(BUILDDIR_LIBK)
	$(call log, [lib] Removing tests prefix directory)
	@$(RMRF) $(BUILDDIR_TESTS)

install-headers:
	@$(MKDIRP) $(BUILDDIR_LIBC)/include
	$(call log, [libc] Copying headers to the build directory)
	@$(CPRP) $(CWD)/include/. $(BUILDDIR_LIBC)/include/.
	$(call log, [libc] Copying platform-dependent headers ($(TARGET_ARCH)) to the build directory)
	@$(MKDIRP) $(BUILDDIR_LIBC)/include/arch
	@$(CPRP) $(CWD)/arch/$(TARGET_ARCH)/include/. $(BUILDDIR_LIBC)/include/.

###### Libc rules

$(BUILDDIR_LIBC)/libc.a: $$(FILES_OBJ) | dirs_libc
	$(call log, [libc] Archiving $(notdir $^))
	@$(AR) rcs $@ $^

$(BUILDDIR_LIBC)/%.c.o: $(CWD)/%.c | dirs_libc
	$(call log, [libc] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libc__ -c $< -o $@

$(BUILDDIR_LIBC)/%.asm.o: $(CWD)/%.asm | dirs_libc
	$(call log, [libc] AS $@)
	@$(AS) $(CFLAGS) $(CPPFLAGS) -D__libc__ $< -o $@

$(BUILDDIR_LIBK)/libk.a: $$(FILES_OBJ) | dirs_libk
	$(call log, [libk] Archiving $(notdir $^))
	@$(AR) rcs $@ $^

$(BUILDDIR_LIBK)/%.c.o: $(CWD)/%.c | dirs_libk
	$(call log, [libk] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ -c $< -o $@

$(BUILDDIR_LIBK)/%.asm.o: $(CWD)/%.asm | dirs_libk
	$(call log, [libk] AS $@)
	@$(AS) $(CPPFLAGS) $(CFLAGS) -D__libk__ $< -o $@

###### Compilation commands rules

compile_commands_libk: $(addprefix $(BUILDDIR_LIBK)/,$(FILES_REL_OBJ:.o=.json))

compile_commands_libk_tests: $(addprefix $(BUILDDIR_TESTS)/libk/,$(FILES_TESTS_OBJ:.o=.json))

$(BUILDDIR_LIBK)/%.c.json: $(CWD)/%.c | dirs_libk
	$(call log, [libk] MJ $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ -MJ $@ -c $< -o /dev/null

$(BUILDDIR_TESTS)/libk/%_tests.c.json: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
$(BUILDDIR_TESTS)/libk/%_tests.c.json: $(CWD)/%_tests.c | dirs_tests
	$(call log, [libk.tests] MJ $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ -MJ $@ $^ -c -o /dev/null

###### Tests rules

dirs_tests test: TESTS := $(patsubst %.c.o,%,$(FILES_TESTS_OBJ))
dirs_tests test: TESTS_LIBC := $(addprefix libc/, $(TESTS))
dirs_tests test: TESTS_LIBK := $(addprefix libk/, $(TESTS))
dirs_tests test: TESTS_LIBC := $(addprefix $(BUILDDIR_TESTS)/, $(TESTS_LIBC))
dirs_tests test: TESTS_LIBK := $(addprefix $(BUILDDIR_TESTS)/, $(TESTS_LIBK))
dirs_tests:
	$(call log, [libk.tests] Creating dir tree)
	$(call log, [libc.tests] Creating dir tree)
	@$(MKDIRP) $(dir $(TESTS_LIBC) $(TESTS_LIBK))

test: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
test: $$(TESTS_LIBC) $$(TESTS_LIBK) | dirs_tests
	$(call log, [libk] Running tests for kernel-space)
	@$(foreach t,$(TESTS_LIBK), echo "[libk.tests] Running $t"; $t || true;)
	$(call log, [libc] Running tests for user-space)
	@$(foreach t,$(TESTS_LIBC), echo "[libc.tests] Running $t"; $t || true;)

$(BUILDDIR_TESTS)/libc/%_tests: $(BUILDDIR_TESTS)/libc/%_tests.c.o $(BUILDDIR_LIBC)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(call log, [libc.tests] LD $@)
	@$(LD) $^ $(LDFLAGS) -o $@

$(BUILDDIR_TESTS)/libc/%_tests.c.o: $(CWD)/%_tests.c | dirs_tests
	$(call log, [libc.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libc__ $^ -c -o $@

$(BUILDDIR_TESTS)/libk/%_tests: $(BUILDDIR_TESTS)/libk/%_tests.c.o $(BUILDDIR_LIBK)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(call log, [libk.tests] LD $@)
	@$(LD) $^ $(LDFLAGS) -o $@

$(BUILDDIR_TESTS)/libk/%_tests.c.o: $(CWD)/%_tests.c | dirs_tests
	$(call log, [libk.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ $^ -c -o $@

###### Dependencies rules

$(BUILDDIR_LIBC)/%.c.d: $(CWD)/%.c | dirs_libc
	$(call log, [libc] MD $@)
	@$(CC) $(CPPFLAGS) -D__libc__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBC)/%.asm.d: $(CWD)/%.asm | dirs_libc
	$(call log, [libc] MD $@)
	@$(AS) $(CPPFLAGS) -D__libc__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBK)/%.c.d: $(CWD)/%.c | dirs_libk
	$(call log, [libk] MD $@)
	@$(CC) $(CPPFLAGS) -D__libk__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBK)/%.asm.d: $(CWD)/%.asm | dirs_libk
	$(call log, [libk] MD $@)
	@$(AS) $(CPPFLAGS) -D__libk__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

ifneq ($(MAKECMDGOALS),clean)
    -include $(addprefix $(BUILDDIR_LIBC)/, $(FILES_REL_OBJ:.o=.d))
    -include $(addprefix $(BUILDDIR_LIBK)/, $(FILES_REL_OBJ:.o=.d))
endif
