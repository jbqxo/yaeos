CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include $(ROOT)/confmk/common.mk
.SECONDEXPANSION:
.SUFFIXES:
.PHONY: all clean tests prefix_dirs install-headers compile_commands tests_dirs offsets

DIR_ARCH := arch/$(ARCH)

CFLAGS_COMPLETE := \
    $(CFLAGS_COMMON) -ffreestanding \
    $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE)) \
    $(CFLAGS)

CPPFLAGS_COMPLETE := \
    $(CPPFLAGS_COMMON) \
    $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE)) \
    -I$(CWD)/include -I$(CWD)/arch/$(ARCH)/include \
    -I$(ROOT)/$(DIR_DEPS) -I$(ROOT)/$(DIR_DEPS)/unity/src \
    -I$(BUILDDIR_KERNEL)/gen_include \
    -D__libk__ \
    $(CPPFLAGS)

LDFLAGS_COMPLETE := \
    $(LDFLAGS_COMMON) \
    $(if $(BUILD_DEBUG),$(LDFLAGS_DEBUG),$(LDFLAGS_RELEASE)) \
    $(LDFLAGS)

FILES_TESTS_OBJ :=\
    $(addsuffix .o, \
    $(shell $(FIND) * -name '*_tests.c'))

OFFSETS_SOURCES := $(shell $(FIND) * -type f -name '*_offsets.c')
OFFSETS_HEADERS := $(patsubst %_offsets.c,$(BUILDDIR_KERNEL)/gen_include/offsets/%.h, $(OFFSETS_SOURCES))

# Generate options to ignore all architectures except for current TARGET
FIND_IGNORE_ARCHS :=\
    $(patsubst %, -not \( -path % -prune \), \
    $(shell $(FIND) * -type d -path 'arch/*' -prune | $(GREP) -v $(ARCH)))
FILES_OBJ :=\
    $(filter-out $(OFFSETS_SOURCES), \
    $(filter-out $(FILES_TESTS_OBJ) test_fakes/%.c.o, \
    $(addsuffix .o, \
    $(shell $(FIND) * $(FIND_IGNORE_ARCHS) -name '*.c' -or -name '*.asm'))))

###### Phony rules

all: $(BUILDDIR_KERNEL)/kernel.bin install-headers

install-headers:
	$(info [kernel] Copying headers to the build directory)
	@$(MKDIRP) $(BUILDDIR_KERNEL)/include
	@$(CPRP) $(CWD)/include/. $(BUILDDIR_KERNEL)/include/.
	$(info [kernel] Copying platform-dependent headers ($(ARCH)) to the build directory)
	@$(MKDIRP) $(BUILDDIR_KERNEL)/include/arch
	@$(CPRP) $(CWD)/arch/$(ARCH)/include/. $(BUILDDIR_KERNEL)/include/.

prefix_dirs $(BUILDDIR_KERNEL)/kernel.bin: FILES_OBJ := $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ))

clean:
	$(info [kernel] Removing kernel prefix directory)
	@$(RMRF) $(BUILDDIR_KERNEL)

prefix_dirs:
	@$(MKDIRP) $(dir $(FILES_OBJ))

###### Kernel rules

$(BUILDDIR_KERNEL)/kernel.bin: LDFLAGS_COMPLETE += -nostdlib
$(BUILDDIR_KERNEL)/kernel.bin: $$(FILES_OBJ) $(DIR_ARCH)/linker.ld
	$(info [kernel] LD $@)
	@$(LD) -Wl,-T,$(DIR_ARCH)/linker.ld $(FILES_OBJ) $(LDFLAGS_COMPLETE) -o $@

$(BUILDDIR_KERNEL)/%.c.o: $(CWD)/%.c | prefix_dirs
	$(info [kernel] CC $@)
	@$(CC) $(CFLAGS_COMPLETE) $(CPPFLAGS_COMPLETE) -c $< -o $@

$(BUILDDIR_KERNEL)/%.asm.o: $(CWD)/%.asm offsets | prefix_dirs
	$(info [kernel] AS $@)
	@$(AS) $(CFLAGS_COMPLETE) $(CPPFLAGS_COMPLETE) $< -o $@

###### Compilation commands rules

compile_commands: $(addprefix $(BUILDDIR_KERNEL)/, \
    $(filter-out %.asm.json, \
    $(FILES_OBJ:.o=.json) $(FILES_TESTS_OBJ:.o=.json)))

$(BUILDDIR_KERNEL)/%.c.json: $(CWD)/%.c | prefix_dirs
	$(info [kernel] MJ $@)
	@$(CC) $(CFLAGS_COMPLETE) $(CPPFLAGS_COMPLETE) -MJ $@ -c $< -o /dev/null

###### Tests rules

tests_dirs tests: FILES_TESTS_OBJ := $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_TESTS_OBJ))
tests_dirs:
	@$(MKDIRP) $(dir $(FILES_TESTS_OBJ))

tests: $$(patsubst %.c.o,%, $$(FILES_TESTS_OBJ)) | tests_dirs

$(BUILDDIR_KERNEL)/%_tests: $(BUILDDIR_KERNEL)/%_tests.c.o\
    $(BUILDDIR_DEPS)/unity/unity.o $$(call get_test_dependencies, $(CWD)/%_tests.c) | tests_dirs
	$(info [kernel.tests] LD $@)
	@$(LD) $(CPPFLAGS_COMPLETE) $^ $(LDFLAGS_COMPLETE) -o $@

###### Offset-files rules

offsets: $(OFFSETS_HEADERS)

$(BUILDDIR_KERNEL)/%_offsets: $(CWD)/%_offsets.c | prefix_dirs
	$(info [kernel.offsets] CC $@)
	@$(HOST_CC) $(CPPFLAGS_COMPLETE) $^ -o $@

$(BUILDDIR_KERNEL)/gen_include/offsets/%.h: $(BUILDDIR_KERNEL)/%_offsets
	$(info [kernel.offsets] GEN $@)
	@$(MKDIRP) $(dir $@)
	@$^ | $(AWK) '{print "#define " toupper($$1) " " $$2}' > $@

###### Dependencies rules

$(BUILDDIR_KERNEL)/%.c.d: $(CWD)/%.c $(OFFSETS_HEADERS) | prefix_dirs tests_dirs
	$(info [kernel] MD $@)
	@$(CC) $(CPPFLAGS_COMPLETE) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_KERNEL)/%.asm.d: $(CWD)/%.asm $(OFFSETS_HEADERS) | prefix_dirs tests_dirs
	$(info [kernel] MD $@)
	@$(AS) $(CPPFLAGS_COMPLETE) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_DEPS)/unity/unity.o:
	$(error You need to build unity framework first)

ifneq ($(MAKECMDGOALS),clean)
    include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ:.o=.d))
    include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_TESTS_OBJ:.o=.d))
    include $(addprefix $(BUILDDIR_KERNEL)/, $(OFFSETS_SOURCES:.c=.c.d))
endif
