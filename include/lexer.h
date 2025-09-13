#ifndef LEXER_H
#define LEXER_H

#include "token.h"

// 词法分析器函数声明
Token getNextToken(const char* src, int* pos);
int lexer(const char* src);

#endif // LEXER_H
