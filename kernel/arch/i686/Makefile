CFLAGS +=
CPPFLAGS +=
LDFLAGS +=

FILES_OBJ_CRT_BEGIN := \
    $(PREFIX_KERNEL_ARCH)/crti.o \
    $(PREFIX_KERNEL_ARCH)/crtbegin.o
FILES_OBJ_CRT_END := \
    $(PREFIX_KERNEL_ARCH)/crtend.o \
    $(PREFIX_KERNEL_ARCH)/crtn.o
FILES_OBJ_CRT_BEGIN :=
FILES_OBJ_CRT_END :=

# Gather all source files in the directory
# Change their suffixes to *.o
# Remove anything related to CRT
FILES_OBJ_ARCH := \
    $(filter-out $(FILES_OBJ_CRT_BEGIN) $(FILES_OBJ_CRT_END),\
    $(subst $(CWD)/, $(PREFIX_KERNEL)/,\
    $(addsuffix .o, \
    $(shell $(FIND) $(DIR_ARCH) -name '*.c' -or -name '*.S'))))

# Properly reorder TU files
FILES_OBJ := \
    $(FILES_OBJ_CRT_BEGIN) \
    $(FILES_OBJ_ARCH) \
    $(FILES_OBJ) \
    $(FILES_OBJ_CRT_END)
