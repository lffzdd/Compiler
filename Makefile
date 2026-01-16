# Makefile - 编译器项目构建脚本
#
# 使用方法:
#   make          - 构建编译器
#   make run      - 运行演示
#   make clean    - 清理构建文件
#   make test     - 测试词法分析器

# 编译器设置
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g

# 目录
SRC_DIR = src
INC_DIR = include
BIN_DIR = bin
OBJ_DIR = obj

# 源文件
SRCS = main.c \
       $(SRC_DIR)/token.c \
       $(SRC_DIR)/lexer.c

# 目标文件
OBJS = $(OBJ_DIR)/main.o \
       $(OBJ_DIR)/token.o \
       $(OBJ_DIR)/lexer.o

# 输出文件
TARGET = $(BIN_DIR)/compiler

# 默认目标
all: dirs $(TARGET)

# 创建目录
dirs:
	@if not exist $(BIN_DIR) mkdir $(BIN_DIR)
	@if not exist $(OBJ_DIR) mkdir $(OBJ_DIR)

# 链接
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# 编译规则
$(OBJ_DIR)/main.o: main.c $(INC_DIR)/lexer.h $(INC_DIR)/token.h
	$(CC) $(CFLAGS) -c -o $@ main.c

$(OBJ_DIR)/token.o: $(SRC_DIR)/token.c $(INC_DIR)/token.h
	$(CC) $(CFLAGS) -c -o $@ $(SRC_DIR)/token.c

$(OBJ_DIR)/lexer.o: $(SRC_DIR)/lexer.c $(INC_DIR)/lexer.h $(INC_DIR)/token.h
	$(CC) $(CFLAGS) -c -o $@ $(SRC_DIR)/lexer.c

# 运行
run: all
	$(TARGET)

# 测试
test: all
	@echo Testing lexer with sample file...
	$(TARGET) test_file/sample.c

# 清理
clean:
	@if exist $(BIN_DIR) rmdir /s /q $(BIN_DIR)
	@if exist $(OBJ_DIR) rmdir /s /q $(OBJ_DIR)

.PHONY: all dirs run test clean
