CWD := $(shell pwd)
ROOT := $(realpath $(CWD)/..)
include ../conf.mk
.SECONDEXPANSION:
.SUFFIXES:
.PHONY: all clean prefix_dirs install-headers

DIR_ARCH := arch/$(TARGET)
BUILDDIR_KERNEL_ARCH := $(BUILDDIR_KERNEL)/$(DIR_ARCH)

CPPFLAGS_KERNEL := -I$(CWD)/include -I$(CWD)/arch/$(TARGET)/include
CPPFLAGS_LIBC := -I$(ROOT)/$(DIR_LIBC)/include -I$(ROOT)/$(DIR_LIBC)/arch/$(TARGET)/include -D__libk__

include $(DIR_ARCH)/Makefile

FILES_TESTS_OBJ :=\
    $(addsuffix .o, \
    $(shell $(FIND) * -name '*_tests.c'))

FILES_OBJ :=\
    $(filter-out $(FILES_TESTS_OBJ) $(FILES_OBJ_ARCH_BEGIN) $(FILES_OBJ_ARCH_END) $(FILES_OBJ_ARCH), \
    $(addsuffix .o, \
    $(shell $(FIND) * -name '*.c' -or -name '*.nasm')))


###### Phony rules

all: $(BUILDDIR_KERNEL)/kernel.bin install-headers

install-headers:
	$(info [kernel] Copying headers to the build directory)
	@$(MKDIRP) $(BUILDDIR_KERNEL)/include
	@$(CPRP) $(CWD)/include/. $(BUILDDIR_KERNEL)/include/.
	$(info [kernel] Copying platform-dependent headers ($(TARGET)) to the build directory)
	@$(MKDIRP) $(BUILDDIR_KERNEL)/include/arch
	@$(CPRP) $(CWD)/arch/$(TARGET)/include/. $(BUILDDIR_KERNEL)/include/.

prefix_dirs $(BUILDDIR_KERNEL)/kernel.bin: FILES_OBJ := $(FILES_OBJ_ARCH_BEGIN) \
    $(FILES_OBJ_ARCH) \
    $(FILES_OBJ) \
    $(FILES_OBJ_ARCH_END)
prefix_dirs $(BUILDDIR_KERNEL)/kernel.bin: FILES_OBJ := $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ))

clean:
	$(info [kernel] Removing kernel prefix directory)
	@$(RMRF) $(BUILDDIR_KERNEL)
	$(info [kernel] Removing tests prefix directory)
	@$(RMRF) $(BUILDDIR_TESTS)

prefix_dirs:
	@$(MKDIRP) $(dir $(FILES_OBJ))

###### Kernel rules

$(BUILDDIR_KERNEL)/kernel.bin: LDFLAGS += -nostdlib $(LDFLAGS_COMMON)
$(BUILDDIR_KERNEL)/kernel.bin: LDFLAGS += $(if $(BUILD_DEBUG),$(LDFLAGS_DEBUG),$(LDFLAGS_RELEASE))
$(BUILDDIR_KERNEL)/kernel.bin: $$(FILES_OBJ) $(DIR_ARCH)/linker.ld $(BUILDDIR_LIBK)/libk.a
	$(info [kernel] LD $@)
	@$(CC) -Wl,-T,$(DIR_ARCH)/linker.ld $(FILES_OBJ) $(BUILDDIR_LIBK)/libk.a $(LDFLAGS) -o $@

$(BUILDDIR_KERNEL)/%.c.o: CPPFLAGS += $(CPPFLAGS_KERNEL) $(CPPFLAGS_LIBC)
$(BUILDDIR_KERNEL)/%.c.o: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_KERNEL)/%.c.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_KERNEL)/%.c.o: CFLAGS += $(CFLAGS_COMMON) -ffreestanding
$(BUILDDIR_KERNEL)/%.c.o: CFLAGS += $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE))
$(BUILDDIR_KERNEL)/%.c.o: $(CWD)/%.c | prefix_dirs
	$(info [kernel] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

$(BUILDDIR_KERNEL)/%.nasm.o: $(CWD)/%.nasm | prefix_dirs
	$(info [kernel] NASM $@)
	@$(NASM) $< -o $@

###### Tests rules

dirs_tests test: FILES_TESTS_OBJ := $(addprefix $(BUILDDIR_TESTS)/kernel/, $(FILES_TESTS_OBJ))
dirs_tests:
	@$(MKDIRP) $(dir $(FILES_TESTS_OBJ))

test: $$(patsubst %.c.o,%, $$(FILES_TESTS_OBJ)) | dirs_tests
	@$(foreach t, $^, echo "[kernel.tests] Running $t"; $t;)

$(BUILDDIR_TESTS)/kernel/%_tests.c.o: CFLAGS += $(CFLAGS_COMMON) -ffreestanding
$(BUILDDIR_TESTS)/kernel/%_tests.c.o: CFLAGS += $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE))
$(BUILDDIR_TESTS)/kernel/%_tests.c.o: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)/unity/src
$(BUILDDIR_TESTS)/kernel/%_tests.c.o: CPPFLAGS += $(CPPFLAGS_KERNEL)
$(BUILDDIR_TESTS)/kernel/%_tests.c.o: $(CWD)/%_tests.c | dirs_tests
	$(info [kernel.tests] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) $^ -c -o $@

$(BUILDDIR_TESTS)/kernel/%_tests: LDFLAGS += $(LDFLAGS_COMMON)
$(BUILDDIR_TESTS)/kernel/%_tests: LDFLAGS += $(if $(BUILD_DEBUG),$(LDFLAGS_DEBUG),$(LDFLAGS_RELEASE))
$(BUILDDIR_TESTS)/kernel/%_tests: $(BUILDDIR_TESTS)/kernel/%_tests.c.o $(BUILDDIR_KERNEL)/%.c.o $(BUILDDIR_DEPS)/unity/unity.o | dirs_tests
	$(info [kernel.tests] LD $@)
	@$(CC) $(CPPFLAGS) $^ $(LDFLAGS) -o $@

###### Dependencies rules

$(BUILDDIR_KERNEL)/%.c.d: CPPFLAGS += $(CPPFLAGS_KERNEL) $(CPPFLAGS_LIBC)
$(BUILDDIR_KERNEL)/%.c.d: CPPFLAGS += $(if $(BUILD_DEBUG),$(CPPFLAGS_DEBUG),$(CPPFLAGS_RELEASE))
$(BUILDDIR_KERNEL)/%.c.d: CPPFLAGS += -I$(ROOT)/$(DIR_DEPS)
$(BUILDDIR_KERNEL)/%.c.d: $(CWD)/%.c | prefix_dirs
	$(info [kernel] MD $@)
	@$(CC) $(CPPFLAGS) -MM -MG -MT $(@:.d=.o) -MF $@ $< -o /dev/null

$(BUILDDIR_KERNEL)/%.nasm.d: $(CWD)/%.nasm | prefix_dirs
	$(info [kernel] MD $@)
	@$(NASM) -MG -MT $(@:.d=.o) -MD $@ $< -o /dev/null

$(BUILDDIR_LIBK)/libk.a:
	$(error You need to build libk first)

$(BUILDDIR_DEPS)/unity/unity.o:
	$(error You need to build unity framework first)

$(BUILDDIR_KERNEL_ARCH)/crtbegin.o $(BUILDDIR_KERNEL_ARCH)/crtend.o: CFLAGS += $(CFLAGS_COMMON) -ffreestanding
$(BUILDDIR_KERNEL_ARCH)/crtbegin.o $(BUILDDIR_KERNEL_ARCH)/crtend.o: CFLAGS += $(if $(BUILD_DEBUG),$(CFLAGS_DEBUG),$(CFLAGS_RELEASE))
$(BUILDDIR_KERNEL_ARCH)/crtbegin.o $(BUILDDIR_KERNEL_ARCH)/crtend.o: | prefix_dirs
	$(info [kernel] Copy $(notdir $@) to the build directory)
	@CRT_O=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$CRT_O" $@

ifneq ($(MAKECMDGOALS),clean)
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ_ARCH:.o=.d))
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ_ARCH_BEGIN:.o=.d))
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ_ARCH_END:.o=.d))
    -include $(addprefix $(BUILDDIR_KERNEL)/, $(FILES_OBJ:.o=.d))
endif
