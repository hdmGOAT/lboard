CC = gcc
CFLAGS = -Wall -Wextra -O2
CPPFLAGS = -MMD -MP

OUT = out
BIN = $(OUT)/lboard
OBJDIR = $(OUT)/obj

SRC = $(shell find src -name '*.c')
OBJ = $(patsubst src/%.c,$(OBJDIR)/%.o,$(SRC))
DEP = $(OBJ:.o=.d)

.PHONY: build clean help

build: $(BIN)

$(BIN): $(OBJ) | $(OUT)
	@echo "Linking lboard"
	$(CC) $(CFLAGS) -o $@ $(OBJ)

$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -rf $(OUT)

help:
	@echo "Targets:"
	@echo "  build  - compile and link the lboard binary"
	@echo "  clean  - remove all build output"

-include $(DEP)
