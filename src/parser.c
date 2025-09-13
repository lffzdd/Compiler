#include "../include/parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Token currentToken;
int pos = 0;
const char* src; // 全局保存源码

// 辅助函数
void nextToken() {
    currentToken = getNextToken(src, &pos);
}

void expect(TokenType type, const char* value) {
    if (currentToken.type != type || (value && strcmp(currentToken.value, value) != 0)) {
        printf("Syntax Error: Expected %s '%s', got %s '%s'\n",
            tokenTypeToString(type), value ? value : "",
            tokenTypeToString(currentToken.type), currentToken.value);
        exit(1);
    }
    nextToken();
}

// program ::= statement | statement program
void parseProgram() {
    while (currentToken.type != TOKEN_EOF) {
        parseStatement();
    }
}

// 解析大括号内的语句块，直到遇到右大括号
void parseBlock() {
    while (currentToken.type != TOKEN_EOF &&
        !(currentToken.type == TOKEN_RIGHT_BRACE && strcmp(currentToken.value, "}") == 0)) {
        parseStatement();
    }
}

// statement ::= variable_declaration | assignment | if_statement | while_statement
void parseStatement() {
    if (currentToken.type == TOKEN_KEYWORD &&
        (strcmp(currentToken.value, "int") == 0 || strcmp(currentToken.value, "float") == 0 ||
            strcmp(currentToken.value, "string") == 0 || strcmp(currentToken.value, "bool") == 0 ||
            strcmp(currentToken.value, "char") == 0)) {
        parseVariableDeclaration();
    }
    else if (currentToken.type == TOKEN_IDENTIFIER) {
        // 需要向前看一个token来判断是否是赋值语句
        parseAssignment();
    }
    else if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "if") == 0) {
        parseIfStatement();
    }
    else if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "while") == 0) {
        parseWhileStatement();
    }
    else {
        printf("Syntax Error: Unexpected token %s '%s'\n",
            tokenTypeToString(currentToken.type), currentToken.value);
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

void parseIfStatement() {
    expect(TOKEN_KEYWORD, "if");
    expect(TOKEN_LEFT_PAREN, "(");
    parseExpression();
    expect(TOKEN_RIGHT_PAREN, ")");
    expect(TOKEN_LEFT_BRACE, "{");
    parseBlock();
    expect(TOKEN_RIGHT_BRACE, "}");
    if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "else") == 0) {
        nextToken();
        expect(TOKEN_LEFT_BRACE, "{");
        parseBlock();
        expect(TOKEN_RIGHT_BRACE, "}");
    }
}

void parseWhileStatement() {
    expect(TOKEN_KEYWORD, "while");
    expect(TOKEN_LEFT_PAREN, "(");
    parseExpression();
    expect(TOKEN_RIGHT_PAREN, ")");
    expect(TOKEN_LEFT_BRACE, "{");
    parseBlock();
    expect(TOKEN_RIGHT_BRACE, "}");
}

// 使用循环避免左递归
void parseExpression() {
    parseTerm();
    while (currentToken.type == TOKEN_OPERATOR &&
        (strcmp(currentToken.value, "+") == 0 ||
            strcmp(currentToken.value, "-") == 0 ||
            strcmp(currentToken.value, ">") == 0 ||
            strcmp(currentToken.value, "<") == 0)) {
        nextToken();
        parseTerm();
    }
}

void parseTerm() {
    parseFactor();
    while (currentToken.type == TOKEN_OPERATOR &&
        (strcmp(currentToken.value, "*") == 0 ||
            strcmp(currentToken.value, "/") == 0)) {
        nextToken();
        parseFactor();
    }
}

void parseFactor() {
    if (currentToken.type == TOKEN_INTEGER ||
        currentToken.type == TOKEN_FLOAT ||
        currentToken.type == TOKEN_IDENTIFIER) {
        nextToken();
    }
    else if (currentToken.type == TOKEN_LEFT_PAREN) {
        nextToken();
        parseExpression();
        expect(TOKEN_RIGHT_PAREN, ")");
    }
    else {
        printf("Syntax Error: Expected number, identifier or '(' in factor, got %s '%s'\n",
            tokenTypeToString(currentToken.type), currentToken.value);
        exit(1);
    }
}
