/**
 * token.c - Token 相关函数实现
 */

#include "../include/token.h"
#include <stdio.h>

/**
 * token_type_to_string - 将 TokenType 转换为可读字符串
 * 
 * 这个函数主要用于调试，让我们能看到 Token 的类型名称
 * 而不是一个数字。
 */
const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_KEYWORD:     return "KEYWORD";
        case TOKEN_IDENTIFIER:  return "IDENTIFIER";
        case TOKEN_INTEGER:     return "INTEGER";
        case TOKEN_FLOAT:       return "FLOAT";
        case TOKEN_CHAR:        return "CHAR";
        case TOKEN_STRING:      return "STRING";
        case TOKEN_OPERATOR:    return "OPERATOR";
        case TOKEN_LPAREN:      return "LPAREN";
        case TOKEN_RPAREN:      return "RPAREN";
        case TOKEN_LBRACE:      return "LBRACE";
        case TOKEN_RBRACE:      return "RBRACE";
        case TOKEN_LBRACKET:    return "LBRACKET";
        case TOKEN_RBRACKET:    return "RBRACKET";
        case TOKEN_SEMICOLON:   return "SEMICOLON";
        case TOKEN_COMMA:       return "COMMA";
        case TOKEN_EOF:         return "EOF";
        case TOKEN_UNKNOWN:     return "UNKNOWN";
        default:                return "???";
    }
}

/**
 * print_token - 打印一个 Token
 * 
 * 格式: [类型] "值"
 * 例如: [KEYWORD] "int"
 */
void print_token(Token token) {
    printf("[%-12s] \"%s\"\n", token_type_to_string(token.type), token.value);
}
