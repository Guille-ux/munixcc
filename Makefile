# Makefile de munixcc

SRC = src
BUILD = build
CC ?= gcc
CFLAGS ?= -Wall
AR = ar
ARFLAGS = rcs

ASSEMBLER ?= miniasm

TARGET_LIB = $(BUILD)/libmunixcc.a

SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))

$(BUILD)/%.o: $(SRC)/%.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: all clean configure deconfigure

$(TARGET_LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

all: $(OBJECTS) $(TARGET_LIB) 

configure:
	@echo "Configuring..."
	rm -rf $(SRC)/parser.c
	rm -rf $(SRC)/symbols.c
	cp $(SRC)/$(ASSEMBLER)/* $(SRC)/
	@echo "Files are Ready!"

deconfigure:
	@echo "Removing Files..."
	rm -rf $(SRC)/parser.c
	rm -rf $(SRC)/symbols.c
	@echo "Files Removed!"

clean:
	@echo "Cleaning..."
	rm -rf $(OBJECTS)
	rm -rf $(TARGET_LIB)
