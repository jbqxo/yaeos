CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/confmk/common.mk
.PHONY: dirs_libc dirs_libk dirs_tests \
    all clean install-headers \
    tests test_libc test_libk \
    compile_commands_libk compile_commands_libk_tests
.SECONDEXPANSION:

DIR_ARCH := $(CWD)/arch/$(TARGET_ARCH)

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

# Generate options to ignore all architectures except for current TARGET
FIND_IGNORE_ARCHS :=\
    $(patsubst %, -not \( -path % -prune \), \
    $(shell $(FIND) * -type d -path 'arch/*' -prune | $(GREP) -v $(TARGET_ARCH)))
FILES_TESTS_OBJ :=\
    $(addsuffix .o, \
    $(shell $(FIND) * $(FIND_IGNORE_ARCHS) -name '*_tests.c'))
FILES_REL_OBJ :=\
    $(filter-out $(FILES_TESTS_OBJ), \
    $(addsuffix .o, \
    $(shell $(FIND) * $(FIND_IGNORE_ARCHS) -name '*.c' -or -name '*.asm')))

dirs_libc $(BUILDDIR_LIBC)/libc.a: FILES_OBJ := $(addprefix $(BUILDDIR_LIBC)/, $(FILES_REL_OBJ))
dirs_libk $(BUILDDIR_LIBK)/libk.a: FILES_OBJ := $(addprefix $(BUILDDIR_LIBK)/, $(FILES_REL_OBJ))

###### Phony rules

all: $(BUILDDIR_LIBC)/libc.a $(BUILDDIR_LIBK)/libk.a install-headers

dirs_libc:
	$(info [libc] Creating dir tree)
	@$(MKDIRP) $(dir $(FILES_OBJ))

dirs_libk:
	$(info [libk] Creating dir tree)
	@$(MKDIRP) $(dir $(FILES_OBJ))

clean:
	$(info [libc] Removing libc prefix directory)
	@$(RMRF) $(BUILDDIR_LIBC)
	$(info [libk] Removing libk prefix directory)
	@$(RMRF) $(BUILDDIR_LIBK)
	$(info [lib] Removing tests prefix directory)
	@$(RMRF) $(BUILDDIR_TESTS)

install-headers:
	@$(MKDIRP) $(BUILDDIR_LIBC)/include
	$(info [libc] Copying headers to the build directory)
	@$(CPRP) $(CWD)/include/. $(BUILDDIR_LIBC)/include/.
	$(info [libc] Copying platform-dependent headers ($(TARGET_ARCH)) to the build directory)
	@$(MKDIRP) $(BUILDDIR_LIBC)/include/arch
	@$(CPRP) $(CWD)/arch/$(TARGET_ARCH)/include/. $(BUILDDIR_LIBC)/include/.

###### Libc rules

$(BUILDDIR_LIBC)/libc.a: $$(FILES_OBJ) | dirs_libc
	$(info [libc] Archiving $(notdir $^))
	@$(AR) rcs $@ $^

$(BUILDDIR_LIBC)/%.c.o: $(CWD)/%.c | dirs_libc
	$(info [libc] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libc__ -c $< -o $@

$(BUILDDIR_LIBC)/%.asm.o: $(CWD)/%.asm | dirs_libc
	$(info [libc] AS $@)
	@$(AS) $(CFLAGS) $(CPPFLAGS) -D__libc__ $< -o $@

$(BUILDDIR_LIBK)/libk.a: $$(FILES_OBJ) | dirs_libk
	$(info [libk] Archiving $(notdir $^))
	@$(AR) rcs $@ $^

$(BUILDDIR_LIBK)/%.c.o: $(CWD)/%.c | dirs_libk
	$(info [libk] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ -c $< -o $@

$(BUILDDIR_LIBK)/%.asm.o: $(CWD)/%.asm | dirs_libk
	$(info [libk] AS $@)
	@$(AS) $(CPPFLAGS) $(CFLAGS) -D__libk__ $< -o $@

###### Compilation commands rules

compile_commands_libk: $(addprefix $(BUILDDIR_LIBK)/,$(FILES_REL_OBJ:.o=.json))

compile_commands_libk_tests: $(addprefix $(BUILDDIR_TESTS)/libk/,$(FILES_TESTS_OBJ:.o=.json))

$(BUILDDIR_LIBK)/%.c.json: $(CWD)/%.c | dirs_libk
	$(info [libk] MJ $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ -MJ $@ -c $< -o /dev/null

$(BUILDDIR_TESTS)/libk/%_tests.c.json: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
$(BUILDDIR_TESTS)/libk/%_tests.c.json: $(CWD)/%_tests.c | dirs_tests
	$(info [libk.tests] MJ $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ -MJ $@ $^ -c -o /dev/null

###### Tests rules

dirs_tests tests: TESTS := $(patsubst %.c.o,%,$(FILES_TESTS_OBJ))
dirs_tests tests: TESTS_LIBC := $(addprefix libc/, $(TESTS))
dirs_tests tests: TESTS_LIBK := $(addprefix libk/, $(TESTS))
dirs_tests tests: TESTS_LIBC := $(addprefix $(BUILDDIR_TESTS)/, $(TESTS_LIBC))
dirs_tests tests: TESTS_LIBK := $(addprefix $(BUILDDIR_TESTS)/, $(TESTS_LIBK))
dirs_tests:
	$(info [libk.tests] Creating dir tree)
	$(info [libc.tests] Creating dir tree)
	@$(MKDIRP) $(dir $(TESTS_LIBC) $(TESTS_LIBK))

tests: $$(TESTS_LIBC) $$(TESTS_LIBK) | dirs_tests

$(BUILDDIR_TESTS)/libc/%_tests: $(BUILDDIR_TESTS)/libc/%_tests.c.o $(BUILDDIR_LIBC)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(info [libc.tests] LD $@)
	@$(LD) $^ $(LDFLAGS) -o $@

$(BUILDDIR_TESTS)/libc/%_tests.c.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
$(BUILDDIR_TESTS)/libc/%_tests.c.o: $(CWD)/%_tests.c | dirs_tests
	$(info [libc.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libc__ $< -c -o $@

$(BUILDDIR_TESTS)/libk/%_tests: $(BUILDDIR_TESTS)/libk/%_tests.c.o $(BUILDDIR_LIBK)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(info [libk.tests] LD $@)
	@$(LD) $^ $(LDFLAGS) -o $@

$(BUILDDIR_TESTS)/libk/%_tests.c.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
$(BUILDDIR_TESTS)/libk/%_tests.c.o: $(CWD)/%_tests.c | dirs_tests
	$(info [libk.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -D__libk__ $< -c -o $@

###### Dependencies rules

$(BUILDDIR_LIBC)/%.c.d: $(CWD)/%.c | dirs_libc
	$(info [libc] MD $@)
	@$(CC) $(CPPFLAGS) -D__libc__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBC)/%.asm.d: $(CWD)/%.asm | dirs_libc
	$(info [libc] MD $@)
	@$(AS) $(CPPFLAGS) -D__libc__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBK)/%.c.d: $(CWD)/%.c | dirs_libk
	$(info [libk] MD $@)
	@$(CC) $(CPPFLAGS) -D__libk__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBK)/%.asm.d: $(CWD)/%.asm | dirs_libk
	$(info [libk] MD $@)
	@$(AS) $(CPPFLAGS) -D__libk__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_TESTS)/libk/%_tests.c.d: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
$(BUILDDIR_TESTS)/libk/%_tests.c.d: $(CWD)/%_tests.c | dirs_tests
	$(info [libk.tests] MD $@)
	@$(CC) $(CPPFLAGS) -D__libk__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_TESTS)/libc/%_tests.c.d: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
$(BUILDDIR_TESTS)/libc/%_tests.c.d: $(CWD)/%_tests.c | dirs_tests
	$(info [libc.tests] MD $@)
	@$(CC) $(CPPFLAGS) -D__libc__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

ifneq ($(MAKECMDGOALS),clean)
    -include $(addprefix $(BUILDDIR_LIBC)/, $(FILES_REL_OBJ:.o=.d))
    -include $(addprefix $(BUILDDIR_LIBK)/, $(FILES_REL_OBJ:.o=.d))
    -include $(addprefix $(BUILDDIR_LIBC)/, $(FILES_TESTS_OBJ:.o=.d))
    -include $(addprefix $(BUILDDIR_LIBK)/, $(FILES_TESTS_OBJ:.o=.d))
endif
