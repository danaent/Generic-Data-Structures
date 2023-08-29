# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -ggdb3

# Directories
SRC_DIR = ../modules

# Files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, %.o, $(SRC_FILES))
LIB = libgds

# Targets
.PHONY:
	lib clean

lib:
	$(CC) $(CFLAGS) -c $(SRC_FILES)
	ar -cvq $(LIB).a *.o
	rm -rf $(OBJ_FILES)

clean:
	rm -rf $(LIB).a