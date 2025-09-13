#include "../1-lex/lexer.c"
#include <stdlib.h>
#include <string.h>

Token currentToken;
int pos = 0;
const char *src; // 全局保存源码

// 辅助函数
void nextToken() { currentToken = getNextToken(src, &pos); }

void expect(TokenType type, const char *value) {
  if (currentToken.type != type || (value && strcmp(currentToken.value, value) != 0)) {
    printf("Syntax Error: Expected %s '%s', got %s '%s'\n", tokenTypeToString(type), value ? value : "",
           tokenTypeToString(currentToken.type), currentToken.value);
    exit(1);
  }
  nextToken();
}

// 递归下降解析器
void parseProgram();

void parseStatement();
void parseVariableDeclaration();
void parseAssignment();
void parseIfStatement();
void parseWhileStatement();

void parseExpression();

// program ::= statement | statement program
void parseProgram() {
  while (currentToken.type != TOKEN_EOF) {
    parseStatement();
  }
}

// statement ::= variable_declaration | assignment | if_statement |
// while_statement
void parseStatement() {
  if (currentToken.type == TOKEN_KEYWORD &&
      (strcmp(currentToken.value, "int") == 0 || strcmp(currentToken.value, "float") == 0 ||
       strcmp(currentToken.value, "string") == 0 || strcmp(currentToken.value, "bool") == 0 ||
       strcmp(currentToken.value, "char") == 0)) {
    parseVariableDeclaration();
  } else if (currentToken.type == TOKEN_IDENTIFIER && strcmp(currentToken.value, "=") == 0) {
    parseAssignment();
  } else if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "if") == 0) {
    parseIfStatement();
  } else if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "while") == 0) {
    parseWhileStatement();
  } else {
    printf("Syntax Error: Unexpected token %s '%s'\n", tokenTypeToString(currentToken.type), currentToken.value);
    exit(1);
  }
}

// variable_declaration ::= type identifier ('=' expression)? ';'
void parseVariableDeclaration() {
  nextToken(); // 跳过类型
  expect(TOKEN_IDENTIFIER, NULL);
  if (currentToken.type == TOKEN_OPERATOR && strcmp(currentToken.value, "=") == 0) {
    nextToken();
    parseExpression();
  }
  expect(TOKEN_SEMICOLON, ";");
}

// assignment ::= identifier '=' expression ';'
void parseAssignment() {
  expect(TOKEN_IDENTIFIER, NULL);
  expect(TOKEN_OPERATOR, "=");
  parseExpression();
  expect(TOKEN_SEMICOLON, ";");
}

// 你还需要写 parseIfStatement, parseWhileStatement, parseExpression, parseTerm,
// parseFactor 等
void parseIfStatement() {
  expect(TOKEN_KEYWORD, "if");
  expect(TOKEN_LEFT_PAREN, "(");
  parseExpression();
  expect(TOKEN_RIGHT_PAREN, ")");
  parseStatement();
  if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "else") == 0) {
    nextToken();
    parseStatement();
  }
}
