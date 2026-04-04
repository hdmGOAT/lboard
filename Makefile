CC = gcc
CFLAGS = -Wall -Wextra -O2
CPPFLAGS = -MMD -MP -Isrc -D_POSIX_C_SOURCE=200809L -D_DEFAULT_SOURCE

DEPS_DIR = deps
LIBSODIUM_DIR = $(DEPS_DIR)/libsodium
LIBSODIUM_PREFIX = $(abspath $(OUT)/deps/libsodium)
LIBSODIUM_LIB = $(LIBSODIUM_PREFIX)/lib/libsodium.a
LIBSODIUM_INC = $(LIBSODIUM_PREFIX)/include

CPPFLAGS += -I$(LIBSODIUM_INC)
LDLIBS += $(LIBSODIUM_LIB)

OUT = out
BIN = $(OUT)/lboard
OBJDIR = $(OUT)/obj

SRC = $(shell find src -name '*.c')
OBJ = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)

.PHONY: build deps run clean help

build: deps $(BIN)

deps: $(LIBSODIUM_LIB)

run: $(BIN)
	@echo "Running lboard"
	@$(BIN)

$(BIN): $(OBJ) $(LIBSODIUM_LIB) | $(OUT)
	@echo "Linking lboard"
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDLIBS)

$(LIBSODIUM_LIB): | $(OUT)
	@echo "Building external dependency: libsodium"
	@mkdir -p $(LIBSODIUM_PREFIX)
	@cd $(LIBSODIUM_DIR) && ./configure --prefix=$(LIBSODIUM_PREFIX)
	@$(MAKE) -C $(LIBSODIUM_DIR)
	@$(MAKE) -C $(LIBSODIUM_DIR) install

$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -rf $(OUT)
	@$(MAKE) -C $(LIBSODIUM_DIR) clean >/dev/null 2>&1 || true

help:
	@echo "Targets:"
	@echo "  build  - compile and link the lboard binary"
	@echo "  deps   - build and install bundled external dependencies"
	@echo "  run    - build (if needed) and run lboard"
	@echo "  clean  - remove all build output"

-include $(DEP)
