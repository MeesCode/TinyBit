# SDL flags
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LDFLAGS := $(shell sdl2-config --libs)

# Compiler and flags
CC = gcc
CFLAGS = $(SDL_CFLAGS) -Wall -Isrc/ -Iinclude/ -Ilua/
LDFLAGS = $(SDL_LDFLAGS) -lm -lSDL2 -lSDL2_image 

# Directories
SRCDIR = src
LUADIR = src/tinybit/lua
ABCDIR = src/tinybit/ABC-parser
TBDIR = src/tinybit
PNGDIR = src/tinybit/pngle
INCDIR = include
BINDIR = bin
ASSETDIR = assets

# Target executable
TARGET = $(BINDIR)/tinybit

# Installation directories
PREFIX = /usr/local
BINDIR_INSTALL = $(PREFIX)/bin
ASSETDIR_INSTALL = $(PREFIX)/share/tinybit

# Source files
SRC_FILES = $(wildcard $(SRCDIR)/*.c)
LUA_FILES = $(wildcard $(LUADIR)/*.c)
TB_FILES = $(wildcard $(TBDIR)/*.c)
PNG_FILES = $(wildcard $(PNGDIR)/*.c)
# Exclude test files from ABC-parser (main.c and test_parser.c are standalone test programs)
ABC_FILES = $(filter-out $(ABCDIR)/main.c $(ABCDIR)/test_parser.c, $(wildcard $(ABCDIR)/*.c))

# Object files
OBJECTS = $(SRC_FILES:$(SRCDIR)/%.c=$(BINDIR)/%.o) $(LUA_FILES:$(LUADIR)/%.c=$(BINDIR)/%.o) \
		   $(TB_FILES:$(TBDIR)/%.c=$(BINDIR)/%.o) $(PNG_FILES:$(PNGDIR)/%.c=$(BINDIR)/%.o) \
		   $(ABC_FILES:$(ABCDIR)/%.c=$(BINDIR)/%.o)

# Default target
all: $(TARGET) copy-assets

# Rule to build the target
$(TARGET): $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Rule to build object files from src
$(BINDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build object files from lua
$(BINDIR)/%.o: $(LUADIR)/%.c
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build object files from tinybit
$(BINDIR)/%.o: $(TBDIR)/%.c
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build object files from ABC-parser
$(BINDIR)/%.o: $(ABCDIR)/%.c
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to build object files from pngle
$(BINDIR)/%.o: $(PNGDIR)/%.c
	@mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rule to copy assets
copy-assets:
	@mkdir -p $(BINDIR)/assets
	@cp -r $(ASSETDIR)/* $(BINDIR)/assets/

# Install the program and assets
install: all
	@mkdir -p $(BINDIR_INSTALL)
	@mkdir -p $(ASSETDIR_INSTALL)
	@cp $(TARGET) $(BINDIR_INSTALL)/
	@cp -r $(ASSETDIR)/* $(ASSETDIR_INSTALL)/

# Clean up generated files
clean:
	rm -rf $(BINDIR)

# Debug target to print variables
debug:
	@echo "SRC_FILES: $(SRC_FILES)"
	@echo "TB_FILES: $(TB_FILES)"
	@echo "OBJECTS: $(OBJECTS)"

# Phony targets
.PHONY: all clean copy-assets install debug
