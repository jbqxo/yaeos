CFLAGS += -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize-memory-track-origins
LDFLAGS += -fsanitize=address
CPPFLAGS += -D__i686__ -DUNITY_INCLUDE_PRINT_FORMATTED
# Silence unity output
CPPFLAGS += -DUNITY_OUTPUT_CHAR=""

CC := clang
LD := clang
AS := gcc -xassembler-with-cpp -c
AR := ar

ARCH := i686
