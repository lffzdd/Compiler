# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -Iinclude
SRCDIR = src
INCDIR = include
OBJDIR = obj
BINDIR = bin

# Create directories
$(shell mkdir -p $(OBJDIR) $(BINDIR))

# Source files
LEXER_SRCS = $(SRCDIR)/token.c $(SRCDIR)/keyword.c $(SRCDIR)/lexer.c
PARSER_SRCS = $(LEXER_SRCS) $(SRCDIR)/parser.c

# Object files
LEXER_OBJS = $(LEXER_SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
PARSER_OBJS = $(PARSER_SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Targets
all: $(BINDIR)/lexer $(BINDIR)/parser

# Lexer executable
$(BINDIR)/lexer: $(LEXER_OBJS) $(OBJDIR)/lexer_main.o
	$(CC) $(CFLAGS) -o $@ $^

# Parser executable
$(BINDIR)/parser: $(PARSER_OBJS) $(OBJDIR)/parser_main.o
	$(CC) $(CFLAGS) -o $@ $^

# Object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/lexer_main.o: lexer_main.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/parser_main.o: parser_main.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Test
test-lexer: $(BINDIR)/lexer
	@echo "Testing lexer with template.c:"
	./$(BINDIR)/lexer template.c

test-parser: $(BINDIR)/parser
	@echo "Testing parser with template.c:"
	./$(BINDIR)/parser template.c

.PHONY: all clean test-lexer test-parser
