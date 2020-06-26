CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/confmk/common.mk
.PHONY: clean dirs_libc dirs_libk dirs_tests all install-headers test test_libc test_libk
.SECONDEXPANSION:

DIR_ARCH := $(CWD)/arch/$(TARGET)

include $(DIR_ARCH)/Makefile

CPPFLAGS_LIBC += \
      -I$(CWD)/include \
      -I$(CWD)/arch/$(TARGET)/include \
      -I$(ROOT)/$(DIR_KERNEL)/include \
      -I$(ROOT)/$(DIR_KERNEL)/arch/$(TARGET)/include

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
	$(call log, [libc] Copying platform-dependent headers ($(TARGET)) to the build directory)
	@$(MKDIRP) $(BUILDDIR_LIBC)/include/arch
	@$(CPRP) $(CWD)/arch/$(TARGET)/include/. $(BUILDDIR_LIBC)/include/.

###### Libc rules

$(BUILDDIR_LIBC)/libc.a: $$(FILES_OBJ) | dirs_libc
	$(call log, [libc] Archiving $(notdir $^))
	@$(AR) rcs $@ $^

$(BUILDDIR_LIBK)/libk.a: $$(FILES_OBJ) | dirs_libk
	$(call log, [libk] Archiving $(notdir $^))
	@$(AR) rcs $@ $^

$(BUILDDIR_LIBC)/%.c.o: CPPFLAGS += $(CPPFLAGS_LIBC) -D__libc__
$(BUILDDIR_LIBC)/%.c.o: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_LIBC)/%.c.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_LIBC)/%.c.o: CFLAGS += $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE)) -ffreestanding
$(BUILDDIR_LIBC)/%.c.o: $(CWD)/%.c | dirs_libc
	$(call log, [libc] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILDDIR_LIBC)/%.asm.o: CPPFLAGS += $(CPPFLAGS_LIBC) -D__libc__
$(BUILDDIR_LIBC)/%.asm.o: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_LIBC)/%.asm.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_LIBC)/%.asm.o: CFLAGS += $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE)) -ffreestanding
$(BUILDDIR_LIBC)/%.asm.o: $(CWD)/%.asm | dirs_libc
	$(call log, [libc] AS $@)
	@$(AS) -D__libc__ $< -o $@

$(BUILDDIR_LIBK)/%.c.o: CPPFLAGS += $(CPPFLAGS_LIBC) -D__libk__
$(BUILDDIR_LIBK)/%.c.o: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_LIBK)/%.c.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_LIBK)/%.c.o: CFLAGS += $(CFLAGS_COMMON) -ffreestanding
$(BUILDDIR_LIBK)/%.c.o: CFLAGS += $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE))
$(BUILDDIR_LIBK)/%.c.o: $(CWD)/%.c | dirs_libk
	$(call log, [libk] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILDDIR_LIBK)/%.asm.o: CPPFLAGS += $(CPPFLAGS_LIBC) -D__libk__
$(BUILDDIR_LIBK)/%.asm.o: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_LIBK)/%.asm.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_LIBK)/%.asm.o: CFLAGS += $(CFLAGS_COMMON) -ffreestanding
$(BUILDDIR_LIBK)/%.asm.o: CFLAGS += $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE))
$(BUILDDIR_LIBK)/%.asm.o: $(CWD)/%.asm | dirs_libk
	$(call log, [libk] AS $@)
	@$(AS) -D__libk__ $< -o $@

###### Tests rules

test: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
dirs_tests test: TESTS := $(patsubst %.c.o,%,$(FILES_TESTS_OBJ))
dirs_tests test: TESTS_LIBC := $(addprefix libc/, $(TESTS))
dirs_tests test: TESTS_LIBK := $(addprefix libk/, $(TESTS))
dirs_tests test: TESTS_LIBC := $(addprefix $(BUILDDIR_TESTS)/, $(TESTS_LIBC))
dirs_tests test: TESTS_LIBK := $(addprefix $(BUILDDIR_TESTS)/, $(TESTS_LIBK))
dirs_tests:
	$(call log, [libk.tests] Creating dir tree)
	$(call log, [libc.tests] Creating dir tree)
	@$(MKDIRP) $(dir $(TESTS_LIBC) $(TESTS_LIBK))

test: $$(TESTS_LIBC) $$(TESTS_LIBK) | dirs_tests
	$(call log, [libk] Running tests for kernel-space)
	@$(foreach t,$(TESTS_LIBK), echo "[libk.tests] Running $t"; $t || true;)
	$(call log, [libc] Running tests for user-space)
	@$(foreach t,$(TESTS_LIBC), echo "[libc.tests] Running $t"; $t || true;)

$(BUILDDIR_TESTS)/libc/%_tests.c.o: CPPFLAGS +=	$(CPPFLAGS_LIBC) -D__libc__
$(BUILDDIR_TESTS)/libc/%_tests.c.o: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_TESTS)/libc/%_tests.c.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_TESTS)/libc/%_tests.c.o: CFLAGS += $(CFLAGS_COMMON)
$(BUILDDIR_TESTS)/libc/%_tests.c.o: CFLAGS += $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE))
$(BUILDDIR_TESTS)/libc/%_tests.c.o: $(CWD)/%_tests.c | dirs_tests
	$(call log, [libc.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $^ -c -o $@

$(BUILDDIR_TESTS)/libk/%_tests.c.o: CPPFLAGS +=	$(CPPFLAGS_LIBC) -D__libk__
$(BUILDDIR_TESTS)/libk/%_tests.c.o: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_TESTS)/libk/%_tests.c.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_TESTS)/libk/%_tests.c.o: CFLAGS += $(CFLAGS_COMMON)
$(BUILDDIR_TESTS)/libk/%_tests.c.o: CFLAGS += $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE)) -ffreestanding
$(BUILDDIR_TESTS)/libk/%_tests.c.o: $(CWD)/%_tests.c | dirs_tests
	$(call log, [libk.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $^ -c -o $@

$(BUILDDIR_TESTS)/libc/%_tests: LDFLAGS += $(LDFLAGS_COMMON)
$(BUILDDIR_TESTS)/libc/%_tests: LDFLAGS += $(if $(BUILD_DEBUG),$(LDFLAGS_DEBUG),$(LDFLAGS_RELEASE))
$(BUILDDIR_TESTS)/libc/%_tests: $(BUILDDIR_TESTS)/libc/%_tests.c.o $(BUILDDIR_LIBC)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(call log, [libc.tests] LD $@)
	@$(LD) $^ $(LDFLAGS) -o $@

$(BUILDDIR_TESTS)/libk/%_tests: LDFLAGS += $(LDFLAGS_COMMON)
$(BUILDDIR_TESTS)/libk/%_tests: LDFLAGS += $(if $(BUILD_DEBUG),$(LDFLAGS_DEBUG),$(LDFLAGS_RELEASE))
$(BUILDDIR_TESTS)/libk/%_tests: $(BUILDDIR_TESTS)/libk/%_tests.c.o $(BUILDDIR_LIBK)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(call log, [libk.tests] LD $@)
	@$(LD) $^ $(LDFLAGS) -o $@

###### Dependencies rules

$(BUILDDIR_LIBC)/%.c.d: CPPFLAGS += $(CPPFLAGS_LIBC) -D__libk__
$(BUILDDIR_LIBC)/%.c.d: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_LIBC)/%.c.d: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_LIBC)/%.c.d: $(CWD)/%.c | dirs_libc
	$(call log, [libc] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBC) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBK)/%.c.d: CPPFLAGS += $(CPPFLAGS_LIBC) -D__libk__
$(BUILDDIR_LIBK)/%.c.d: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_LIBK)/%.c.d: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_LIBK)/%.c.d: $(CWD)/%.c | dirs_libk
	$(call log, [libk] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBK) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBC)/%.asm.d: CPPFLAGS += $(CPPFLAGS_LIBC) -D__libk__
$(BUILDDIR_LIBC)/%.asm.d: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_LIBC)/%.asm.d: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_LIBC)/%.asm.d: $(CWD)/%.asm | dirs_libc
	$(call log, [libc] MD $@)
	@$(AS) -D__libc__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBK)/%.asm.d: CPPFLAGS += $(CPPFLAGS_LIBC) -D__libk__
$(BUILDDIR_LIBK)/%.asm.d: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_LIBK)/%.asm.d: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_LIBK)/%.asm.d: $(CWD)/%.asm | dirs_libk
	$(call log, [libk] MD $@)
	@$(AS) -D__libk__ -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

ifneq ($(MAKECMDGOALS),clean)
    -include $(addprefix $(BUILDDIR_LIBC)/, $(FILES_REL_OBJ:.o=.d))
    -include $(addprefix $(BUILDDIR_LIBK)/, $(FILES_REL_OBJ:.o=.d))
endif
