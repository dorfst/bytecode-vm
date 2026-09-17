CC ?= gcc
SRC_DIR := src
OUTPUT_DIR := output
TARGET := $(OUTPUT_DIR)/vm

SRCS := $(wildcard $(SRC_DIR)/*.c)

CFLAGS ?= -Wall -Wextra

# Default build is a debug build (includes -g, no optimisation).
# Run `make` or `make debug` for a debug build.
# Run `make release` for an optimised build with no debug symbols.
.PHONY: all debug release clean

all: debug

debug: CFLAGS += -g -O0
debug: $(TARGET)

release: CFLAGS += -O2
release: $(TARGET)

$(TARGET): $(SRCS) | $(OUTPUT_DIR)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

$(OUTPUT_DIR):
	mkdir -p $(OUTPUT_DIR)

clean:
	rm -f $(TARGET)