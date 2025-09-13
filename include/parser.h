#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// 全局变量声明
extern Token currentToken;
extern int pos;
extern const char* src;

// 解析器函数声明
void nextToken();
void expect(TokenType type, const char* value);

void parseProgram();
void parseBlock();
void parseStatement();
void parseVariableDeclaration();
void parseAssignment();
void parseIfStatement();
void parseWhileStatement();
void parseExpression();
void parseTerm();
void parseFactor();

#endif // PARSER_H
