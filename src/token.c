#include "../include/token.h"

const char* tokenTypeToString(TokenType type) {
    switch (type) {
    case TOKEN_KEYWORD:
        return "KEYWORD";
    case TOKEN_OPERATOR:
        return "OPERATOR";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_INTEGER:
        return "INTEGER";
    case TOKEN_FLOAT:
        return "FLOAT";
    case TOKEN_CHAR:
        return "CHAR";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_BOOL:
        return "BOOL";
    case TOKEN_LEFT_PAREN:
        return "LEFT_PAREN";
    case TOKEN_RIGHT_PAREN:
        return "RIGHT_PAREN";
    case TOKEN_LEFT_BRACE:
        return "LEFT_BRACE";
    case TOKEN_RIGHT_BRACE:
        return "RIGHT_BRACE";
    case TOKEN_LEFT_BRACKET:
        return "LEFT_BRACKET";
    case TOKEN_RIGHT_BRACKET:
        return "RIGHT_BRACKET";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_UNKNOWN:
        return "UNKNOWN";
    default:
        return "UNKNOWN";
    }
}
