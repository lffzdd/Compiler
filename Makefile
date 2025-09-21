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
PARSER_SRCS = $(LEXER_SRCS) $(SRCDIR)/parser.c $(SRCDIR)/ast.c
SEMANTIC_SRCS = $(PARSER_SRCS) $(SRCDIR)/semantic.c

# Object files
LEXER_OBJS = $(LEXER_SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
PARSER_OBJS = $(PARSER_SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
SEMANTIC_OBJS = $(SEMANTIC_SRCS:$(SRCDIR)/%.c=$(OBJDIR)/%.o)

# Targets
all: $(BINDIR)/lexer $(BINDIR)/parser $(BINDIR)/semantic

# Lexer executable
$(BINDIR)/lexer: $(LEXER_OBJS) $(OBJDIR)/lexer_main.o
	$(CC) $(CFLAGS) -o $@ $^

# Parser executable
$(BINDIR)/parser: $(PARSER_OBJS) $(OBJDIR)/parser_main.o
	$(CC) $(CFLAGS) -o $@ $^

# Semantic analyzer executable
$(BINDIR)/semantic: $(SEMANTIC_OBJS) $(OBJDIR)/semantic_main.o
	$(CC) $(CFLAGS) -o $@ $^

# Semantic test executable
$(BINDIR)/semantic_test: $(SEMANTIC_OBJS) $(OBJDIR)/semantic_test.o
	$(CC) $(CFLAGS) -o $@ $^

# Object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/lexer_main.o: lexer_main.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/parser_main.o: parser_main.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/semantic_main.o: semantic_main.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/semantic_test.o: semantic_test.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/ast_main.o: ast_main.c
	$(CC) $(CFLAGS) -c -o $@ $<

# Clean
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Test targets
test-lexer: $(BINDIR)/lexer
	@echo "Testing lexer with template.c:"
	./$(BINDIR)/lexer test_file/template.c

test-parser: $(BINDIR)/parser
	@echo "Testing parser with template.c:"
	./$(BINDIR)/parser test_file/template.c

test-semantic: $(BINDIR)/semantic
	@echo "Testing semantic analyzer:"
	./$(BINDIR)/semantic

test-semantic-suite: $(BINDIR)/semantic_test
	@echo "Running semantic analyzer test suite:"
	./$(BINDIR)/semantic_test

# Build and test all components
test-all: test-lexer test-parser test-semantic

.PHONY: all clean test-lexer test-parser test-semantic test-semantic-suite test-all
