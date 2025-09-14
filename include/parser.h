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
void parseDeclaration();  // 新增：解析顶层声明
void parseBlock();
void parseStatement();
void parseVariableDeclaration();
void parseAssignment();
void parseIfStatement();
void parseWhileStatement();
void parseExpression();
void parseTerm();
void parseFactor();

// 函数相关解析
void parseFunctionDeclaration();
void parseFunctionDefinition();
void parseParameterList();
void parseParameter();
void parseFunctionCall();
void parseArgumentList();
void parseReturnStatement();

// 主解析函数，类似于lexer中的lexer函数
int parse(const char* src);
int parse_debug(const char* src); // 调试版本的解析函数

#endif // PARSER_H
