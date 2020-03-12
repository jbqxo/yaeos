CWD := $(shell pwd)

# Suppress all warnings
CFLAGS += -w

.PHONY: print all unity

all: unity

unity: $(PREFIX_DEPS)/unity/unity.o

prefix_dirs:
	@$(MKDIRP) $(PREFIX_DEPS)/unity

$(PREFIX_DEPS)/unity/%.o:	$(CWD)/unity/src/%.c | prefix_dirs
	$(info [deps.unity] CC $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

clean:
	$(info [deps] Removing dependencies prefix directory)
	@$(RMRF) $(PREFIX_DEPS)
