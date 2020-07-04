CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/confmk/common.mk
.SECONDEXPANSION:
.SUFFIXES:
.PHONY: all clean prefix_dirs install-headers compile_commands_kernel compile_commands_tests

DIR_ARCH := arch/$(TARGET_ARCH)
BUILDDIR_KERNEL_ARCH := $(BUILDDIR_KERNEL)/$(DIR_ARCH)
include $(DIR_ARCH)/Makefile

CFLAGS += \
    $(CFLAGS_COMMON) -ffreestanding \
    $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE))

CPPFLAGS += \
    $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE)) \
    -I$(CWD)/include -I$(CWD)/arch/$(TARGET_ARCH)/include \
    -I$(ROOT)/$(DIR_LIBC)/include -I$(ROOT)/$(DIR_LIBC)/arch/$(TARGET_ARCH)/include \
    -I$(ROOT)/$(DIR_DEPS) \
    -D__libk__

LDFLAGS += \
    $(LDFLAGS_COMMON) \
    $(if $(BUILD_DEBUG),$(LDFLAGS_DEBUG),$(LDFLAGS_RELEASE))

FILES_TESTS_OBJ :=\
    $(addsuffix .o, \
    $(shell $(FIND) * -name '*_tests.c'))

# Generate options to ignore all architectures except for current TARGET
FIND_IGNORE_ARCHS :=\
    $(patsubst %, -not \( -path % -prune \), \
    $(shell $(FIND) * -type d -path 'arch/*' -prune | $(GREP) -v $(TARGET_ARCH)))
FILES_OBJ :=\
    $(filter-out $(FILES_TESTS_OBJ) $(FILES_OBJ_ARCH_BEGIN) $(FILES_OBJ_ARCH_END) $(FILES_OBJ_ARCH), \
    $(addsuffix .o, \
    $(shell $(FIND) * $(FIND_IGNORE_ARCHS) -name '*.c' -or -name '*.asm')))

###### Phony rules

all: $(BUILDDIR_KERNEL)/kernel.bin install-headers

install-headers:
	$(call log, [kernel] Copying headers to the build directory)
	@$(MKDIRP) $(BUILDDIR_KERNEL)/include
	@$(CPRP) $(CWD)/include/. $(BUILDDIR_KERNEL)/include/.
	$(call log, [kernel] Copying platform-dependent headers ($(TARGET_ARCH)) to the build directory)
	@$(MKDIRP) $(BUILDDIR_KERNEL)/include/arch
	@$(CPRP) $(CWD)/arch/$(TARGET_ARCH)/include/. $(BUILDDIR_KERNEL)/include/.

prefix_dirs $(BUILDDIR_KERNEL)/kernel.bin: FILES_OBJ := $(FILES_OBJ_ARCH_BEGIN) \
    $(FILES_OBJ_ARCH) \
    $(FILES_OBJ) \
    $(FILES_OBJ_ARCH_END)
prefix_dirs $(BUILDDIR_KERNEL)/kernel.bin: FILES_OBJ := $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ))

clean:
	$(call log, [kernel] Removing kernel prefix directory)
	@$(RMRF) $(BUILDDIR_KERNEL)
	$(call log, [kernel] Removing tests prefix directory)
	@$(RMRF) $(BUILDDIR_TESTS)

prefix_dirs:
	@$(MKDIRP) $(dir $(FILES_OBJ))

###### Kernel rules

$(BUILDDIR_KERNEL)/kernel.bin: LDFLAGS += -nostdlib
$(BUILDDIR_KERNEL)/kernel.bin: $$(FILES_OBJ) $(DIR_ARCH)/linker.ld $(BUILDDIR_LIBK)/libk.a
	$(call log, [kernel] LD $@)
	@$(LD) -Wl,-T,$(DIR_ARCH)/linker.ld $(FILES_OBJ) $(BUILDDIR_LIBK)/libk.a $(LDFLAGS) -o $@

$(BUILDDIR_KERNEL)/%.c.o: $(CWD)/%.c | prefix_dirs
	$(call log, [kernel] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILDDIR_KERNEL)/%.asm.o: $(CWD)/%.asm | prefix_dirs
	$(call log, [kernel] AS $@)
	@$(AS) $(CFLAGS) $(CPPFLAGS) $< -o $@

###### Compilation commands rules

compile_commands_kernel: $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ:.o=.json))
compile_commands_tests: $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_TESTS_OBJ:.o=.json))

$(BUILDDIR_KERNEL)/%.c.json: $(CWD)/%.c | prefix_dirs
	$(call log, [kernel] MJ $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -MJ $@ -c $< -o /dev/null

$(BUILDDIR_KERNEL)/kernel/%_tests.c.json: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
$(BUILDDIR_KERNEL)/kernel/%_tests.c.json: $(CWD)/%_tests.c | dirs_tests
	$(call log, [kernel.tests] MJ $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -MJ $@ $< -c -o /dev/null

###### Tests rules

dirs_tests test: FILES_TESTS_OBJ := $(addprefix $(BUILDDIR_TESTS)/kernel/, $(FILES_TESTS_OBJ))
dirs_tests:
	@$(MKDIRP) $(dir $(FILES_TESTS_OBJ))

test: $$(patsubst %.c.o,%, $$(FILES_TESTS_OBJ)) | dirs_tests
	@$(foreach t, $^, echo "[kernel.tests] Running $t"; $t;)

$(BUILDDIR_TESTS)/kernel/%_tests.c.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
$(BUILDDIR_TESTS)/kernel/%_tests.c.o: $(CWD)/%_tests.c | dirs_tests
	$(call log, [kernel.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $^ -c -o $@

$(BUILDDIR_TESTS)/kernel/%_tests: $(BUILDDIR_TESTS)/kernel/%_tests.c.o $(BUILDDIR_KERNEL)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(call log, [kernel.tests] LD $@)
	@$(LD) $(CPPFLAGS) $^ $(LDFLAGS) -o $@

###### Dependencies rules

$(BUILDDIR_KERNEL)/%.c.d: $(CWD)/%.c | prefix_dirs
	$(call log, [kernel] MD $@)
	@$(CC) $(CPPFLAGS) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_KERNEL)/%.asm.d: $(CWD)/%.asm | prefix_dirs
	$(call log, [kernel] MD $@)
	@$(AS) $(CPPFLAGS) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_LIBK)/libk.a:
	$(error You need to build libk first)

$(BUILDDIR_DEPS)/unity/unity.o:
	$(error You need to build unity framework first)

$(BUILDDIR_KERNEL_ARCH)/crtbegin.o $(BUILDDIR_KERNEL_ARCH)/crtend.o: | prefix_dirs
	$(call log, [kernel] Copy $(notdir $@) to the build directory)
	@CRT_O=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$CRT_O" $@

ifneq ($(MAKECMDGOALS),clean)
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ_ARCH:.o=.d))
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ_ARCH_BEGIN:.o=.d))
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ_ARCH_END:.o=.d))
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ:.o=.d))
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_TESTS_OBJ:.o=.d))
endif
