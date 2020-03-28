CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include ../conf.mk
.PHONY: clean dirs_libc dirs_libk dirs_tests all install-headers test test_libc test_libk
.SECONDEXPANSION:

DIR_ARCH := $(CWD)/arch/$(TARGET)

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
FILES_LIBC_OBJ := $(addprefix $(BUILDDIR_LIBC)/, $(FILES_REL_OBJ))
FILES_LIBK_OBJ := $(addprefix $(BUILDDIR_LIBK)/, $(FILES_REL_OBJ))

include $(DIR_ARCH)/Makefile

###### Phony rules

all: $(BUILDDIR_LIBC)/libc.a $(BUILDDIR_LIBK)/libk.a install-headers

dirs_libc:
	$(info [libc] Creating dir tree)
	@$(MKDIRP) $(dir $(FILES_LIBC_OBJ))

dirs_libk:
	$(info [libk] Creating dir tree)
	@$(MKDIRP) $(dir $(FILES_LIBK_OBJ))

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
	$(info [libc] Copying platform-dependent headers ($(TARGET)) to the build directory)
	@$(MKDIRP) $(BUILDDIR_LIBC)/include/arch
	@$(CPRP) $(CWD)/arch/$(TARGET)/include/. $(BUILDDIR_LIBC)/include/.

###### Kernel rules

$(BUILDDIR_LIBC)/libc.a: $(FILES_LIBC_OBJ) | dirs_libc
	$(info [libc] Archiving $(notdir $(FILES_LIBC_OBJ)))
	@$(AR) rcs $@ $^

$(BUILDDIR_LIBK)/libk.a: $(FILES_LIBK_OBJ) | dirs_libk
	$(info [libk] Archiving $(notdir $(FILES_LIBK_OBJ)))
	@$(AR) rcs $@ $^

$(BUILDDIR_LIBC)/%.c.o: $(CWD)/%.c | dirs_libc
	$(info [libc] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBC) -c $< -o $@

$(BUILDDIR_LIBC)/%.nasm.o: $(CWD)/%.nasm | dirs_libc
	$(info [libc] NASM $@)
	@$(AS) -D__libc__ $< -o $@

$(BUILDDIR_LIBK)/%.c.o: $(CWD)/%.c | dirs_libk
	$(info [libk] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBK) -c $< -o $@

$(BUILDDIR_LIBK)/%.nasm.o: $(CWD)/%.nasm | dirs_libk
	$(info [libk] NASM $@)
	@$(AS) -D__libk__ $< -o $@

###### Tests rules

test_libk test_libc: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
dirs_tests test_libk test_libc: TESTS := $(patsubst %.c.o,%,$(FILES_TESTS_OBJ))
dirs_tests test_libc: TESTS_LIBC := $(addprefix libc/, $(TESTS))
dirs_tests test_libk: TESTS_LIBK := $(addprefix libk/, $(TESTS))
dirs_tests test_libc: TESTS_LIBC := $(addprefix $(BUILDDIR_TESTS)/, $(TESTS_LIBC))
dirs_tests test_libk: TESTS_LIBK := $(addprefix $(BUILDDIR_TESTS)/, $(TESTS_LIBK))
dirs_tests:
	$(info [libk.tests] Creating dir tree)
	$(info [libc.tests] Creating dir tree)
	@$(MKDIRP) $(dir $(TESTS_LIBC) $(TESTS_LIBK))
test: test_libc test_libk

test_libk:  $$(TESTS_LIBK) | dirs_tests
	$(info [libk] Running tests for kernel-space)
	@$(foreach t,$^, echo "[libk.tests] Running $t"; $t || true;)

test_libc: $$(TESTS_LIBC) | dirs_tests
	$(info [libc] Running tests for user-space)
	@$(foreach t,$^, echo "[libc.tests] Running $t"; $t || true;)

$(BUILDDIR_TESTS)/libc/%_tests.c.o: $(CWD)/%_tests.c | dirs_tests
	$(info [libc.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBC) $^ -c -o $@

$(BUILDDIR_TESTS)/libk/%_tests.c.o: $(CWD)/%_tests.c | dirs_tests
	$(info [libk.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBK) $^ -c -o $@

$(BUILDDIR_TESTS)/libc/%_tests: $(BUILDDIR_TESTS)/libc/%_tests.c.o $(BUILDDIR_LIBC)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(info [libc.tests] LD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBC) $^ -o $@

$(BUILDDIR_TESTS)/libk/%_tests: $(BUILDDIR_TESTS)/libk/%_tests.c.o $(BUILDDIR_LIBK)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(info [libk.tests] LD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBK) $^ -o $@

###### Dependencies rules

$(BUILDDIR_LIBC)/%.c.d: $(CWD)/%.c | dirs_libc
	$(info [libc] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBC) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBK)/%.c.d: $(CWD)/%.c | dirs_libk
	$(info [libk] MD $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(CPPFLAGS_LIBK) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBC)/%.nasm.d: $(CWD)/%.nasm | dirs_libc
	$(info [libc] MD $@)
	@$(AS) -D__libc__ -MG -MT $(@:.d=.o) -MD $@ $< -o /dev/null

$(BUILDDIR_LIBK)/%.nasm.d: $(CWD)/%.nasm | dirs_libk
	$(info [libk] MD $@)
	@$(AS) -D__libk__ -MG -MT $(@:.d=.o) -MD $@ $< -o /dev/null

ifneq ($(MAKECMDGOALS),clean)
    -include $(FILES_LIBC_OBJ:.o=.d) $(FILES_LIBK_OBJ:.o=.d)
endif
