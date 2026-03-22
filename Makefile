CC = gcc
CFLAGS = -Wall -Wextra -O2

OUT = out
BIN = $(OUT)/lboard
SRC = src/main.c src/networking/discovery.c src/networking/types.c src/store/node.c

build: $(OUT) $(BIN)

$(BIN): $(SRC)
	@echo "Building lboard"
	$(CC) $(CFLAGS) -o $(BIN) $(SRC)

$(OUT):
	mkdir -p $(OUT)

clean:
	rm -f $(BIN)

help:
	@echo "Targets:"
	@echo "  build  - build the lboard binary"
	@echo "  clean  - remove build output"
