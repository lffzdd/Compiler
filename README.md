# C Compiler Project

这是一个简单的C语言编译器项目，包含词法分析器和语法分析器。

## 项目结构

```
Compiler/
├── include/           # 头文件目录
│   ├── token.h       # Token类型定义
│   ├── keyword.h     # 关键字处理
│   ├── lexer.h       # 词法分析器头文件
│   └── parser.h      # 语法分析器头文件
├── src/              # 源文件目录
│   ├── token.c       # Token相关实现
│   ├── keyword.c     # 关键字处理实现
│   ├── lexer.c       # 词法分析器实现
│   └── parser.c      # 语法分析器实现
├── 1-lex/            # 原始词法分析器（保留）
├── 2-cfg/            # 原始语法分析器（保留）
├── lexer_main.c      # 词法分析器主程序
├── parser_main.c     # 语法分析器主程序
├── Makefile          # 构建脚本
├── template.c        # 测试用源文件
└── README.md         # 项目说明

## 构建方法

### 使用Makefile（推荐）
```bash
# 构建所有程序
make all

# 只构建词法分析器
make bin/lexer

# 只构建语法分析器
make bin/parser

# 清理构建文件
make clean
```

### 手动编译
```bash
# 编译词法分析器
gcc -Wall -Wextra -std=c99 -Iinclude -o bin/lexer src/token.c src/keyword.c src/lexer.c lexer_main.c

# 编译语法分析器  
gcc -Wall -Wextra -std=c99 -Iinclude -o bin/parser src/token.c src/keyword.c src/lexer.c src/parser.c parser_main.c
```

## 使用方法

### 词法分析
```bash
./bin/lexer template.c
```

### 语法分析
```bash
./bin/parser template.c
```

### 测试
```bash
make test-lexer    # 测试词法分析器
make test-parser   # 测试语法分析器
```

## 支持的语法

### 数据类型
- `int`, `float`, `char`, `string`, `bool`

### 语句类型
- 变量声明：`int x = 5;`
- 赋值语句：`x = 10;`
- 条件语句：`if (x > 0) { ... } else { ... }`
- 循环语句：`while (x > 0) { ... }`

### 表达式
- 算术运算：`+`, `-`, `*`, `/`
- 比较运算：`>`, `<`
- 括号表达式：`(expression)`

## 项目特点

1. **模块化设计**：头文件和源文件分离，便于维护
2. **避免左递归**：语法分析器使用循环而非递归处理表达式
3. **错误处理**：提供详细的语法错误信息
4. **可扩展性**：易于添加新的语法规则和功能
