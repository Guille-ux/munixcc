# Makefile de munixcc

SRC = src
BUILD = build
CC = gcc
CFLAGS = -c
AR = ar
ARFLAGS = rcs

TARGET_LIB = $(BUILD)/libmunixcc.a

SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(patsubst %.c, %.o, $(SOURCES))

$(BUILD)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) $< -o $@

.PHONY: all clean

$(TARGET_LIB): $(OBJECTS)
	$(AR) $(ARFLAGS) $@ $^

all: $(OBJECTS) $(TARGET_LIB) 
clean:
	@echo "Cleaning..."
	rm -rf $(OBJECTS)
	rm -rf $(TARGET_LIB)
