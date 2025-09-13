# Compiler Project Makefile

# Compiler settings
CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g
TARGET_DIR = bin
OBJ_DIR = obj

# Lexer targets
LEXER_DIR = 1-lex
LEXER_SOURCES = $(wildcard $(LEXER_DIR)/*.c)
LEXER_OBJECTS = $(LEXER_SOURCES:$(LEXER_DIR)/%.c=$(OBJ_DIR)/lexer_%.o)
LEXER_TARGET = $(TARGET_DIR)/lexer

# Parser targets
PARSER_DIR = 2-cfg
PARSER_SOURCES = $(wildcard $(PARSER_DIR)/*.c)
PARSER_OBJECTS = $(PARSER_SOURCES:$(PARSER_DIR)/%.c=$(OBJ_DIR)/parser_%.o)
PARSER_TARGET = $(TARGET_DIR)/parser

# Template target
TEMPLATE_SOURCE = template.c
TEMPLATE_OBJECT = $(OBJ_DIR)/template.o
TEMPLATE_TARGET = $(TARGET_DIR)/template

# Default target
all: directories $(LEXER_TARGET) $(PARSER_TARGET) $(TEMPLATE_TARGET)

# Create necessary directories
directories:
	@if not exist "$(TARGET_DIR)" mkdir "$(TARGET_DIR)"
	@if not exist "$(OBJ_DIR)" mkdir "$(OBJ_DIR)"

# Lexer compilation
$(LEXER_TARGET): $(LEXER_OBJECTS) | directories
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/lexer_%.o: $(LEXER_DIR)/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

# Parser compilation
$(PARSER_TARGET): $(PARSER_OBJECTS) | directories
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/parser_%.o: $(PARSER_DIR)/%.c | directories
	$(CC) $(CFLAGS) -c $< -o $@

# Template compilation
$(TEMPLATE_TARGET): $(TEMPLATE_OBJECT) | directories
	$(CC) $(CFLAGS) -o $@ $^

$(TEMPLATE_OBJECT): $(TEMPLATE_SOURCE) | directories
	$(CC) $(CFLAGS) -c $< -o $@

# Generate compile_commands.json for clangd
compile_commands:
	@echo Generating compile_commands.json...
	@echo [ > compile_commands.json
	@for %%f in ($(LEXER_SOURCES)) do @echo   { >> compile_commands.json && @echo     "directory": "$(shell cd)", >> compile_commands.json && @echo     "command": "$(CC) $(CFLAGS) -c %%f", >> compile_commands.json && @echo     "file": "%%f" >> compile_commands.json && @echo   }, >> compile_commands.json
	@for %%f in ($(PARSER_SOURCES)) do @echo   { >> compile_commands.json && @echo     "directory": "$(shell cd)", >> compile_commands.json && @echo     "command": "$(CC) $(CFLAGS) -c %%f", >> compile_commands.json && @echo     "file": "%%f" >> compile_commands.json && @echo   }, >> compile_commands.json
	@echo   { >> compile_commands.json
	@echo     "directory": "$(shell cd)", >> compile_commands.json
	@echo     "command": "$(CC) $(CFLAGS) -c $(TEMPLATE_SOURCE)", >> compile_commands.json
	@echo     "file": "$(TEMPLATE_SOURCE)" >> compile_commands.json
	@echo   } >> compile_commands.json
	@echo ] >> compile_commands.json

# Clean build artifacts
clean:
	@if exist "$(TARGET_DIR)" rmdir /s /q "$(TARGET_DIR)"
	@if exist "$(OBJ_DIR)" rmdir /s /q "$(OBJ_DIR)"

# Test targets
test-lexer: $(LEXER_TARGET)
	$(LEXER_TARGET) template.c

test-parser: $(PARSER_TARGET)
	$(PARSER_TARGET)

# Install clangd (requires scoop or chocolatey)
install-clangd:
	@echo Installing clangd via scoop...
	scoop install llvm

# Help target
help:
	@echo Available targets:
	@echo   all           - Build all targets
	@echo   lexer         - Build lexer only
	@echo   parser        - Build parser only
	@echo   template      - Build template only
	@echo   compile_commands - Generate compile_commands.json for clangd
	@echo   clean         - Clean build artifacts
	@echo   test-lexer    - Test lexer with template.c
	@echo   test-parser   - Test parser
	@echo   install-clangd - Install clangd via scoop
	@echo   help          - Show this help

.PHONY: all clean directories compile_commands test-lexer test-parser install-clangd help
