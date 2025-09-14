#ifndef TOKEN_H
#define TOKEN_H

// 词法分析器相关的类型定义
typedef enum {
    TOKEN_KEYWORD,
    TOKEN_OPERATOR,
    TOKEN_IDENTIFIER,

    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_CHAR,
    TOKEN_STRING,
    TOKEN_BOOL,

    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,

    TOKEN_SEMICOLON,
    TOKEN_COMMA,
    TOKEN_EOF,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    TokenType type;
    char value[1024];
} Token;

// 函数声明
const char* tokenTypeToString(TokenType type);

#endif // TOKEN_H
