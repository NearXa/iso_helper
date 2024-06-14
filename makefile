CC = gcc
CFLAGS = -std=c99 -pedantic -Werror -Wall -Wextra -Wvla
SRC_DIR = src
SRC = $(SRC_DIR)/iso_helper.c
TARGET = $(SRC_DIR)/isohelper

all: $(TARGET)

$(TARGET): $(SRC)
	@$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)

.PHONY: all clean test