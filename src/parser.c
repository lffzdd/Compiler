#include "../include/parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Token currentToken;
int pos = 0;
const char* src; // 全局保存源码
static int debug_mode = 0; // 调试模式标志
static int indent_level = 0; // 缩进级别

// 打印调试信息
void debug_print(const char* function_name) {
    if (debug_mode) {
        for (int i = 0; i < indent_level; i++) {
            printf("  ");
        }
        printf("-> %s (current token: %s '%s')\n",
            function_name,
            tokenTypeToString(currentToken.type),
            currentToken.value);
    }
}

// 辅助函数
void nextToken() {
    currentToken = getNextToken(src, &pos);
    if (debug_mode) {
        printf("Next token: %s '%s'\n",
            tokenTypeToString(currentToken.type),
            currentToken.value);
    }
}

// 前瞻函数：读取token但不输出调试信息
Token peekToken(int* peek_pos) {
    return getNextToken(src, peek_pos);
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

// program ::= declaration_list
// declaration_list ::= declaration | declaration declaration_list
void parseProgram() {
    debug_print("parseProgram");
    indent_level++;

    while (currentToken.type != TOKEN_EOF) {
        parseDeclaration();
    }

    indent_level--;
    if (debug_mode) {
        printf("Program parsing completed.\n");
    }
}

// 解析顶层声明：全局变量声明、函数声明或函数定义
// declaration ::= global_variable_declaration | function_declaration | function_definition
void parseDeclaration() {
    debug_print("parseDeclaration");
    indent_level++;

    if (currentToken.type == TOKEN_KEYWORD &&
        (strcmp(currentToken.value, "int") == 0 || strcmp(currentToken.value, "float") == 0 ||
            strcmp(currentToken.value, "string") == 0 || strcmp(currentToken.value, "bool") == 0 ||
            strcmp(currentToken.value, "char") == 0 || strcmp(currentToken.value, "void") == 0)) {

        // 使用前瞻判断类型
        int peek_pos = pos;
        Token type_token = currentToken;

        // 前瞻：类型 -> 标识符 -> ?
        Token next1 = peekToken(&peek_pos);  // 跳过类型，应该是标识符
        Token next2 = peekToken(&peek_pos);  // 跳过标识符，检查下一个token

        if (next1.type != TOKEN_IDENTIFIER) {
            printf("Syntax Error: Expected identifier after type\n");
            exit(1);
        }

        if (next2.type == TOKEN_LEFT_PAREN) {
            // 这是函数，继续前瞻判断是声明还是定义
            int temp_pos = peek_pos;

            // 跳过参数列表
            int paren_count = 1;
            Token temp_token = peekToken(&temp_pos);
            while (paren_count > 0 && temp_token.type != TOKEN_EOF) {
                if (temp_token.type == TOKEN_LEFT_PAREN) paren_count++;
                else if (temp_token.type == TOKEN_RIGHT_PAREN) paren_count--;
                temp_token = peekToken(&temp_pos);
            }

            if (temp_token.type == TOKEN_SEMICOLON) {
                // 函数声明
                parseFunctionDeclaration();
            }
            else if (temp_token.type == TOKEN_LEFT_BRACE) {
                // 函数定义
                parseFunctionDefinition();
            }
            else {
                printf("Syntax Error: Expected ';' or '{' after function signature\n");
                exit(1);
            }
        }
        else {
            // 这是变量声明
            parseVariableDeclaration();
        }
    }
    else {
        printf("Syntax Error: Expected type specifier\n");
        exit(1);
    }

    indent_level--;
}

// 解析语句块（函数体内的语句列表）
// statement_list ::= statement | statement statement_list
void parseBlock() {
    while (currentToken.type != TOKEN_EOF &&
        !(currentToken.type == TOKEN_RIGHT_BRACE && strcmp(currentToken.value, "}") == 0)) {
        parseStatement();
    }
}

// 解析语句（只能在函数体内出现）
// statement ::= local_variable_declaration | assignment | if_statement | while_statement | return_statement | expression_statement
void parseStatement() {
    debug_print("parseStatement");
    indent_level++;

    if (currentToken.type == TOKEN_KEYWORD &&
        (strcmp(currentToken.value, "int") == 0 || strcmp(currentToken.value, "float") == 0 ||
            strcmp(currentToken.value, "string") == 0 || strcmp(currentToken.value, "bool") == 0 ||
            strcmp(currentToken.value, "char") == 0)) {
        parseVariableDeclaration();
    }
    else if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "if") == 0) {
        parseIfStatement();
    }
    else if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "while") == 0) {
        parseWhileStatement();
    }
    else if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "return") == 0) {
        parseReturnStatement();
    }
    else if (currentToken.type == TOKEN_IDENTIFIER) {
        // 使用前瞻判断是赋值还是表达式语句
        int peek_pos = pos;
        Token next_token = peekToken(&peek_pos);  // 向前看一个token

        if (next_token.type == TOKEN_OPERATOR && strcmp(next_token.value, "=") == 0) {
            // 这是赋值语句
            parseAssignment();
        }
        else {
            // 这是表达式语句
            if (debug_mode) {
                printf("  Parsing expression statement\n");
            }
            parseExpression();
            expect(TOKEN_SEMICOLON, ";");
        }
    }
    else {
        printf("Syntax Error: Unexpected token %s '%s'\n",
            tokenTypeToString(currentToken.type), currentToken.value);
        exit(1);
    }

    indent_level--;
}// variable_declaration ::= type identifier ('=' expression)? ';'
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
    debug_print("parseIfStatement");
    indent_level++;

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

    indent_level--;
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
    debug_print("parseExpression");
    indent_level++;

    parseTerm();
    while (currentToken.type == TOKEN_OPERATOR &&
        (strcmp(currentToken.value, "+") == 0 ||
            strcmp(currentToken.value, "-") == 0 ||
            strcmp(currentToken.value, ">") == 0 ||
            strcmp(currentToken.value, "<") == 0 ||
            strcmp(currentToken.value, ">=") == 0 ||
            strcmp(currentToken.value, "<=") == 0 ||
            strcmp(currentToken.value, "==") == 0 ||
            strcmp(currentToken.value, "!=") == 0)) {
        if (debug_mode) {
            printf("  Found operator: %s\n", currentToken.value);
        }
        nextToken();
        parseTerm();
    }

    indent_level--;
}

void parseTerm() {
    debug_print("parseTerm");
    indent_level++;

    parseFactor();
    while (currentToken.type == TOKEN_OPERATOR &&
        (strcmp(currentToken.value, "*") == 0 ||
            strcmp(currentToken.value, "/") == 0)) {
        if (debug_mode) {
            printf("  Found operator: %s\n", currentToken.value);
        }
        nextToken();
        parseFactor();
    }

    indent_level--;
}

void parseFactor() {
    debug_print("parseFactor");
    indent_level++;

    if (currentToken.type == TOKEN_INTEGER ||
        currentToken.type == TOKEN_FLOAT ||
        currentToken.type == TOKEN_CHAR ||
        currentToken.type == TOKEN_STRING ||
        currentToken.type == TOKEN_BOOL) {
        if (debug_mode) {
            printf("  Found literal: %s '%s'\n",
                tokenTypeToString(currentToken.type), currentToken.value);
        }
        nextToken();
    }
    else if (currentToken.type == TOKEN_IDENTIFIER) {
        // 使用前瞻判断是标识符还是函数调用
        int peek_pos = pos;
        Token next_token = peekToken(&peek_pos);  // 向前看一个token

        if (next_token.type == TOKEN_LEFT_PAREN) {
            // 这是函数调用
            if (debug_mode) {
                printf("  Found function call: %s\n", currentToken.value);
            }
            nextToken(); // 跳过函数名
            parseFunctionCall();
        }
        else {
            // 这是普通标识符
            if (debug_mode) {
                printf("  Found identifier: %s '%s'\n",
                    tokenTypeToString(currentToken.type), currentToken.value);
            }
            nextToken();
        }
    }
    else if (currentToken.type == TOKEN_LEFT_PAREN) {
        if (debug_mode) {
            printf("  Found parenthesized expression\n");
        }
        nextToken();
        parseExpression();
        expect(TOKEN_RIGHT_PAREN, ")");
    }
    else {
        printf("Syntax Error: Expected number, identifier, literal or '(' in factor, got %s '%s'\n",
            tokenTypeToString(currentToken.type), currentToken.value);
        exit(1);
    }

    indent_level--;
}

// 主解析函数，类似于lexer中的lexer函数
int parse(const char* source) {
    // 设置全局源码指针
    src = source;
    pos = 0;

    printf("=== Parsing ===\n");

    // 获取第一个token
    nextToken();

    // 开始解析
    parseProgram();

    printf("Parse completed successfully!\n");

    return 0;
}

// 带调试模式的解析函数
int parse_debug(const char* source) {
    // 启用调试模式
    debug_mode = 1;
    indent_level = 0;

    // 设置全局源码指针
    src = source;
    pos = 0;

    printf("=== Parsing (Debug Mode) ===\n");

    // 获取第一个token
    nextToken();

    // 开始解析
    parseProgram();

    printf("Parse completed successfully!\n");

    debug_mode = 0; // 关闭调试模式
    return 0;
}

// 函数声明解析：type identifier(parameter_list);
void parseFunctionDeclaration() {
    debug_print("parseFunctionDeclaration");
    indent_level++;

    // 跳过类型（在parseDeclaration中已确认是类型）
    nextToken();

    // 解析函数名
    expect(TOKEN_IDENTIFIER, NULL);
    if (debug_mode) {
        printf("  Function name parsed\n");
    }

    // 解析参数列表
    expect(TOKEN_LEFT_PAREN, "(");
    parseParameterList();
    expect(TOKEN_RIGHT_PAREN, ")");

    // 函数声明以分号结束
    expect(TOKEN_SEMICOLON, ";");

    indent_level--;
}

// 函数定义解析：type identifier(parameter_list) { ... }
void parseFunctionDefinition() {
    debug_print("parseFunctionDefinition");
    indent_level++;

    // 跳过类型（在parseDeclaration中已确认是类型）
    nextToken();

    // 解析函数名
    expect(TOKEN_IDENTIFIER, NULL);
    if (debug_mode) {
        printf("  Function name parsed\n");
    }

    // 解析参数列表
    expect(TOKEN_LEFT_PAREN, "(");
    parseParameterList();
    expect(TOKEN_RIGHT_PAREN, ")");

    // 解析函数体
    expect(TOKEN_LEFT_BRACE, "{");
    parseBlock();
    expect(TOKEN_RIGHT_BRACE, "}");

    indent_level--;
}

// 参数列表解析：parameter | parameter, parameter_list | 空
void parseParameterList() {
    debug_print("parseParameterList");
    indent_level++;

    // 空参数列表
    if (currentToken.type == TOKEN_RIGHT_PAREN) {
        if (debug_mode) {
            printf("  Empty parameter list\n");
        }
        indent_level--;
        return;
    }

    // 解析第一个参数
    parseParameter();

    // 解析后续参数
    while (currentToken.type == TOKEN_COMMA) {
        nextToken(); // 跳过逗号
        parseParameter();
    }

    indent_level--;
}

// 单个参数解析：type identifier
void parseParameter() {
    debug_print("parseParameter");
    indent_level++;

    // 解析参数类型
    if (currentToken.type == TOKEN_KEYWORD &&
        (strcmp(currentToken.value, "int") == 0 || strcmp(currentToken.value, "float") == 0 ||
            strcmp(currentToken.value, "string") == 0 || strcmp(currentToken.value, "bool") == 0 ||
            strcmp(currentToken.value, "char") == 0 || strcmp(currentToken.value, "void") == 0)) {
        if (debug_mode) {
            printf("  Parameter type: %s\n", currentToken.value);
        }
        nextToken();
    }
    else {
        printf("Syntax Error: Expected parameter type\n");
        exit(1);
    }

    // 解析参数名
    expect(TOKEN_IDENTIFIER, NULL);
    if (debug_mode) {
        printf("  Parameter name parsed\n");
    }

    indent_level--;
}

// return语句解析：return expression?;
void parseReturnStatement() {
    debug_print("parseReturnStatement");
    indent_level++;

    expect(TOKEN_KEYWORD, "return");

    // 检查是否有返回值
    if (currentToken.type != TOKEN_SEMICOLON) {
        parseExpression();
    }

    expect(TOKEN_SEMICOLON, ";");

    indent_level--;
}

// 函数调用解析：identifier(argument_list)
void parseFunctionCall() {
    debug_print("parseFunctionCall");
    indent_level++;

    // 函数名已经在parseFactor中处理
    expect(TOKEN_LEFT_PAREN, "(");

    // 解析参数列表
    if (currentToken.type != TOKEN_RIGHT_PAREN) {
        parseArgumentList();
    }

    expect(TOKEN_RIGHT_PAREN, ")");

    indent_level--;
}

// 参数列表解析：expression | expression, argument_list
void parseArgumentList() {
    debug_print("parseArgumentList");
    indent_level++;

    // 解析第一个参数
    parseExpression();

    // 解析后续参数
    while (currentToken.type == TOKEN_COMMA) {
        nextToken(); // 跳过逗号
        parseExpression();
    }

    indent_level--;
}