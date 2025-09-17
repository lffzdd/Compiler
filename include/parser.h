#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

// 全局变量声明
extern Token currentToken;
extern int pos;
extern const char* src;

// 解析器函数声明
void nextToken();
void expect(TokenType type, const char* value);

// 原有的解析函数（语法检查）
void parseProgram();
void parseDeclaration();
void parseBlock();
void parseStatement();

// 新增：构建AST的解析函数
ASTNode* parseProgram_AST();
ASTNode* parseDeclaration_AST();
ASTNode* parseStatement_AST();
ASTNode* parseBlock_AST();
ASTNode* parseExpression_AST();
ASTNode* parseTerm_AST();
ASTNode* parseFactor_AST();
ASTNode* parseVariableDeclaration_AST();
ASTNode* parseAssignment_AST();
ASTNode* parseIfStatement_AST();
ASTNode* parseWhileStatement_AST();
ASTNode* parseReturnStatement_AST();
ASTNode* parseFunctionDeclaration_AST();
ASTNode* parseFunctionDefinition_AST();
ASTNode* parseParameterList_AST();
ASTNode* parseArgumentList_AST();

// 辅助函数
BinaryOperatorType tokenToBinaryOperator(Token token);

// 主接口函数
void parse(const char* source);
void parse_debug(const char* source);
ASTNode* parse_and_build_ast(const char* source);  // 新增：解析并构建AST

// 原有的解析函数
void parseParameter();
void parseFunctionCall();
void parseArgumentList();
void parseReturnStatement();

#endif // PARSER_H
