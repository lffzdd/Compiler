/**
 * lexer.c - 词法分析器实现
 * 
 * 这是编译器的第一个阶段！
 * 
 * 词法分析的核心算法：
 * 1. 跳过空白字符和注释
 * 2. 看当前字符，判断这是什么类型的 Token
 * 3. 收集完整的 Token 内容
 * 4. 返回 Token
 */

#include "../include/lexer.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>  // isalpha, isdigit, isspace 等函数

// ========== 关键字表 ==========
// 我们支持的所有关键字
static const char* keywords[] = {
    "int", "float", "char", "void",     // 类型
    "if", "else", "while", "for",       // 控制流
    "return",                           // 返回
    "true", "false",                    // 布尔值（虽然C没有，但方便学习）
    NULL  // 结束标记
};

/**
 * is_keyword - 检查一个词是否是关键字
 * @word: 要检查的词
 * 
 * 返回: 1 如果是关键字，0 如果不是
 * 
 * 算法很简单：遍历关键字表，逐个比较
 */
static int is_keyword(const char* word) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(word, keywords[i]) == 0) {
            return 1;  // 找到了，是关键字
        }
    }
    return 0;  // 没找到，不是关键字
}

// ========== 辅助函数 ==========

/**
 * current_char - 获取当前字符
 * 
 * 为什么封装成函数？
 * - 代码更清晰
 * - 方便之后添加边界检查
 */
static char current_char(Lexer* lexer) {
    return lexer->source[lexer->pos];
}

/**
 * advance - 前进一个字符
 * 
 * 同时更新：
 * - pos（位置）
 * - line 和 column（用于错误报告）
 */
static void advance(Lexer* lexer) {
    if (current_char(lexer) == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    lexer->pos++;
}

/**
 * skip_whitespace - 跳过空白字符
 * 
 * 空白字符包括：空格、制表符、换行符、回车符
 */
static void skip_whitespace(Lexer* lexer) {
    while (isspace(current_char(lexer))) {
        advance(lexer);
    }
}

/**
 * skip_line_comment - 跳过单行注释
 * 
 * 单行注释以 // 开头，到行尾结束
 */
static void skip_line_comment(Lexer* lexer) {
    // 跳过 //
    advance(lexer);
    advance(lexer);
    
    // 跳过直到行尾
    while (current_char(lexer) != '\0' && current_char(lexer) != '\n') {
        advance(lexer);
    }
}

/**
 * skip_block_comment - 跳过块注释
 * 
 * 块注释以 /* 开头，以 * / 结束（中间没有空格）
 */
static void skip_block_comment(Lexer* lexer) {
    // 跳过 /*
    advance(lexer);
    advance(lexer);
    
    // 找到 */
    while (current_char(lexer) != '\0') {
        if (current_char(lexer) == '*' && lexer->source[lexer->pos + 1] == '/') {
            advance(lexer);  // 跳过 *
            advance(lexer);  // 跳过 /
            return;
        }
        advance(lexer);
    }
    // 如果到了文件末尾还没找到 */，这是一个错误
    // 这里简化处理，实际编译器应该报错
}

/**
 * skip_whitespace_and_comments - 跳过所有空白和注释
 * 
 * 注意：这是一个循环，因为注释后面可能还有空白，
 * 空白后面可能还有注释...
 */
static void skip_whitespace_and_comments(Lexer* lexer) {
    while (1) {
        // 跳过空白
        skip_whitespace(lexer);
        
        // 检查是否是注释
        if (current_char(lexer) == '/' && lexer->source[lexer->pos + 1] == '/') {
            skip_line_comment(lexer);
        } else if (current_char(lexer) == '/' && lexer->source[lexer->pos + 1] == '*') {
            skip_block_comment(lexer);
        } else {
            break;  // 既不是空白也不是注释，退出
        }
    }
}

// ========== Token 读取函数 ==========

/**
 * read_identifier_or_keyword - 读取标识符或关键字
 * 
 * 规则：
 * - 以字母或下划线开头
 * - 后续可以是字母、数字或下划线
 * 
 * 读完后检查是否是关键字
 */
static Token read_identifier_or_keyword(Lexer* lexer) {
    Token token;
    int i = 0;
    
    // 读取所有合法字符
    while (isalnum(current_char(lexer)) || current_char(lexer) == '_') {
        if (i < MAX_TOKEN_LENGTH - 1) {
            token.value[i++] = current_char(lexer);
        }
        advance(lexer);
    }
    token.value[i] = '\0';  // 字符串结束符
    
    // 判断是关键字还是标识符
    token.type = is_keyword(token.value) ? TOKEN_KEYWORD : TOKEN_IDENTIFIER;
    
    return token;
}

/**
 * read_number - 读取数字（整数或浮点数）
 * 
 * 简化版本，只支持：
 * - 整数: 123
 * - 浮点数: 3.14
 * 
 * 不支持科学计数法(1e10)、十六进制(0xFF)等
 */
static Token read_number(Lexer* lexer) {
    Token token;
    int i = 0;
    token.type = TOKEN_INTEGER;  // 先假设是整数
    
    // 读取整数部分
    while (isdigit(current_char(lexer))) {
        if (i < MAX_TOKEN_LENGTH - 1) {
            token.value[i++] = current_char(lexer);
        }
        advance(lexer);
    }
    
    // 检查是否有小数点
    if (current_char(lexer) == '.' && isdigit(lexer->source[lexer->pos + 1])) {
        token.type = TOKEN_FLOAT;  // 改为浮点数
        token.value[i++] = '.';
        advance(lexer);
        
        // 读取小数部分
        while (isdigit(current_char(lexer))) {
            if (i < MAX_TOKEN_LENGTH - 1) {
                token.value[i++] = current_char(lexer);
            }
            advance(lexer);
        }
    }
    
    token.value[i] = '\0';
    return token;
}

/**
 * read_string - 读取字符串字面量
 * 
 * 字符串以双引号开始和结束
 * 支持转义字符: \n, \t, \", \\
 */
static Token read_string(Lexer* lexer) {
    Token token;
    token.type = TOKEN_STRING;
    int i = 0;
    
    advance(lexer);  // 跳过开始的双引号
    
    while (current_char(lexer) != '"' && current_char(lexer) != '\0') {
        if (current_char(lexer) == '\\') {
            // 转义字符
            advance(lexer);
            switch (current_char(lexer)) {
                case 'n':  token.value[i++] = '\n'; break;
                case 't':  token.value[i++] = '\t'; break;
                case '"':  token.value[i++] = '"';  break;
                case '\\': token.value[i++] = '\\'; break;
                default:   token.value[i++] = current_char(lexer); break;
            }
        } else {
            token.value[i++] = current_char(lexer);
        }
        advance(lexer);
    }
    
    if (current_char(lexer) == '"') {
        advance(lexer);  // 跳过结束的双引号
    }
    // 如果到达文件末尾但没有结束引号，这是错误（简化处理）
    
    token.value[i] = '\0';
    return token;
}

/**
 * read_char - 读取字符字面量
 * 
 * 字符以单引号开始和结束
 */
static Token read_char(Lexer* lexer) {
    Token token;
    token.type = TOKEN_CHAR;
    int i = 0;
    
    advance(lexer);  // 跳过开始的单引号
    
    if (current_char(lexer) == '\\') {
        // 转义字符
        advance(lexer);
        switch (current_char(lexer)) {
            case 'n':  token.value[i++] = '\n'; break;
            case 't':  token.value[i++] = '\t'; break;
            case '\'': token.value[i++] = '\''; break;
            case '\\': token.value[i++] = '\\'; break;
            default:   token.value[i++] = current_char(lexer); break;
        }
        advance(lexer);
    } else if (current_char(lexer) != '\'' && current_char(lexer) != '\0') {
        token.value[i++] = current_char(lexer);
        advance(lexer);
    }
    
    if (current_char(lexer) == '\'') {
        advance(lexer);  // 跳过结束的单引号
    }
    
    token.value[i] = '\0';
    return token;
}

/**
 * read_operator - 读取运算符
 * 
 * 注意：要先检查双字符运算符（==, !=, <=, >=）
 * 再检查单字符运算符
 */
static Token read_operator(Lexer* lexer) {
    Token token;
    token.type = TOKEN_OPERATOR;
    
    char c = current_char(lexer);
    char next = lexer->source[lexer->pos + 1];
    
    // 检查双字符运算符
    if ((c == '=' && next == '=') ||  // ==
        (c == '!' && next == '=') ||  // !=
        (c == '<' && next == '=') ||  // <=
        (c == '>' && next == '=') ||  // >=
        (c == '&' && next == '&') ||  // &&
        (c == '|' && next == '|')) {  // ||
        token.value[0] = c;
        token.value[1] = next;
        token.value[2] = '\0';
        advance(lexer);
        advance(lexer);
    } else {
        // 单字符运算符
        token.value[0] = c;
        token.value[1] = '\0';
        advance(lexer);
    }
    
    return token;
}

// ========== 主要接口 ==========

/**
 * lexer_init - 初始化词法分析器
 */
Lexer lexer_init(const char* source) {
    Lexer lexer;
    lexer.source = source;
    lexer.pos = 0;
    lexer.line = 1;
    lexer.column = 1;
    return lexer;
}

/**
 * lexer_next_token - 获取下一个 Token
 * 
 * 这是词法分析器的核心！
 * 通过首字符判断 Token 类型，然后调用相应的读取函数
 */
Token lexer_next_token(Lexer* lexer) {
    Token token;
    memset(token.value, 0, sizeof(token.value));
    
    // 1. 跳过空白和注释
    skip_whitespace_and_comments(lexer);
    
    // 2. 检查是否到达文件末尾
    if (current_char(lexer) == '\0') {
        token.type = TOKEN_EOF;
        token.value[0] = '\0';
        return token;
    }
    
    char c = current_char(lexer);
    
    // 3. 根据首字符判断 Token 类型
    
    // 标识符或关键字: 以字母或下划线开头
    if (isalpha(c) || c == '_') {
        return read_identifier_or_keyword(lexer);
    }
    
    // 数字: 以数字开头
    if (isdigit(c)) {
        return read_number(lexer);
    }
    
    // 字符串: 以双引号开头
    if (c == '"') {
        return read_string(lexer);
    }
    
    // 字符: 以单引号开头
    if (c == '\'') {
        return read_char(lexer);
    }
    
    // 运算符
    if (strchr("+-*/%=<>!&|", c)) {
        return read_operator(lexer);
    }
    
    // 分隔符（单字符）
    switch (c) {
        case '(':
            token.type = TOKEN_LPAREN;
            token.value[0] = c;
            advance(lexer);
            return token;
        case ')':
            token.type = TOKEN_RPAREN;
            token.value[0] = c;
            advance(lexer);
            return token;
        case '{':
            token.type = TOKEN_LBRACE;
            token.value[0] = c;
            advance(lexer);
            return token;
        case '}':
            token.type = TOKEN_RBRACE;
            token.value[0] = c;
            advance(lexer);
            return token;
        case '[':
            token.type = TOKEN_LBRACKET;
            token.value[0] = c;
            advance(lexer);
            return token;
        case ']':
            token.type = TOKEN_RBRACKET;
            token.value[0] = c;
            advance(lexer);
            return token;
        case ';':
            token.type = TOKEN_SEMICOLON;
            token.value[0] = c;
            advance(lexer);
            return token;
        case ',':
            token.type = TOKEN_COMMA;
            token.value[0] = c;
            advance(lexer);
            return token;
    }
    
    // 未知字符
    token.type = TOKEN_UNKNOWN;
    token.value[0] = c;
    advance(lexer);
    return token;
}

/**
 * lexer_peek_token - 预览下一个 Token（不消耗）
 * 
 * 实现方法：保存状态 → 读取 → 恢复状态
 */
Token lexer_peek_token(Lexer* lexer) {
    // 保存当前状态
    int saved_pos = lexer->pos;
    int saved_line = lexer->line;
    int saved_column = lexer->column;
    
    // 读取 Token
    Token token = lexer_next_token(lexer);
    
    // 恢复状态
    lexer->pos = saved_pos;
    lexer->line = saved_line;
    lexer->column = saved_column;
    
    return token;
}
