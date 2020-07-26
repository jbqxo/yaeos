CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/confmk/common.mk
.SECONDEXPANSION:
.SUFFIXES:
.PHONY: all clean tests prefix_dirs install-headers compile_commands

DIR_ARCH := arch/$(TARGET_ARCH)

CFLAGS += \
    $(CFLAGS_COMMON) -ffreestanding \
    $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE))

CPPFLAGS += \
    $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE)) \
    -I$(CWD)/include -I$(CWD)/arch/$(TARGET_ARCH)/include \
    -I$(ROOT)/$(DIR_DEPS) -I$(ROOT)/$(DIR_DEPS)/unity/src \
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
    $(filter-out $(FILES_TESTS_OBJ), \
    $(addsuffix .o, \
    $(shell $(FIND) * $(FIND_IGNORE_ARCHS) -name '*.c' -or -name '*.asm')))

###### Phony rules

all: $(BUILDDIR_KERNEL)/kernel.bin install-headers

install-headers:
	$(info [kernel] Copying headers to the build directory)
	@$(MKDIRP) $(BUILDDIR_KERNEL)/include
	@$(CPRP) $(CWD)/include/. $(BUILDDIR_KERNEL)/include/.
	$(info [kernel] Copying platform-dependent headers ($(TARGET_ARCH)) to the build directory)
	@$(MKDIRP) $(BUILDDIR_KERNEL)/include/arch
	@$(CPRP) $(CWD)/arch/$(TARGET_ARCH)/include/. $(BUILDDIR_KERNEL)/include/.

prefix_dirs $(BUILDDIR_KERNEL)/kernel.bin: FILES_OBJ := $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ))

clean:
	$(info [kernel] Removing kernel prefix directory)
	@$(RMRF) $(BUILDDIR_KERNEL)

prefix_dirs:
	@$(MKDIRP) $(dir $(FILES_OBJ))

###### Kernel rules

$(BUILDDIR_KERNEL)/kernel.bin: LDFLAGS += -nostdlib
$(BUILDDIR_KERNEL)/kernel.bin: $$(FILES_OBJ) $(DIR_ARCH)/linker.ld
	$(info [kernel] LD $@)
	@$(LD) -Wl,-T,$(DIR_ARCH)/linker.ld $(FILES_OBJ) $(LDFLAGS) -o $@

$(BUILDDIR_KERNEL)/%.c.o: $(CWD)/%.c | prefix_dirs
	$(info [kernel] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILDDIR_KERNEL)/%.asm.o: $(CWD)/%.asm | prefix_dirs
	$(info [kernel] AS $@)
	@$(AS) $(CFLAGS) $(CPPFLAGS) $< -o $@

###### Compilation commands rules

compile_commands: $(addprefix $(BUILDDIR_KERNEL)/, \
    $(filter-out %.asm.json, \
    $(FILES_OBJ:.o=.json)))

$(BUILDDIR_KERNEL)/%.c.json: $(CWD)/%.c | prefix_dirs
	$(info [kernel] MJ $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -MJ $@ -c $< -o /dev/null

###### Tests rules

dirs_tests tests: FILES_TESTS_OBJ := $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_TESTS_OBJ))
dirs_tests:
	@$(MKDIRP) $(dir $(FILES_TESTS_OBJ))

tests: $$(patsubst %.c.o,%, $$(FILES_TESTS_OBJ)) | dirs_tests

$(BUILDDIR_KERNEL)/%_tests: $(BUILDDIR_KERNEL)/%_tests.c.o $(BUILDDIR_KERNEL)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(info [kernel.tests] LD $@)
	@$(LD) $(CPPFLAGS) $^ $(LDFLAGS) -o $@

###### Dependencies rules

$(BUILDDIR_KERNEL)/%.c.d: $(CWD)/%.c | prefix_dirs
	$(info [kernel] MD $@)
	@$(CC) $(CPPFLAGS) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_KERNEL)/%.asm.d: $(CWD)/%.asm | prefix_dirs
	$(info [kernel] MD $@)
	@$(AS) $(CPPFLAGS) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_DEPS)/unity/unity.o:
	$(error You need to build unity framework first)

ifneq ($(MAKECMDGOALS),clean)
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ:.o=.d))
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_TESTS_OBJ:.o=.d))
endif
