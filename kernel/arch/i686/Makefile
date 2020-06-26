CFLAGS_ARCH +=
CPPFLAGS_ARCH +=
LDFLAGS_ARCH +=

FILES_OBJ_ARCH_BEGIN := \
    $(DIR_ARCH)/crti.asm.o
FILES_OBJ_ARCH_END := \
    $(DIR_ARCH)/crtn.asm.o

FILES_OBJ_ARCH_BEGIN += $(DIR_ARCH)/crtbegin.o
FILES_OBJ_ARCH_END   := $(DIR_ARCH)/crtend.o $(FILES_OBJ_ARCH_END)

# Gather all source files in the directory
# Change their suffixes to *.o
# Remove anything related to CRT
FILES_OBJ_ARCH := \
    $(filter-out $(FILES_OBJ_ARCH_BEGIN) $(FILES_OBJ_ARCH_END),\
    $(addsuffix .o, \
    $(shell $(FIND) $(DIR_ARCH) -name '*.c' -or -name '*.asm')))
