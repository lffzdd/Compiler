#include "../include/ast.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// 字符串复制辅助函数
char* strdup_safe(const char* str) {
    if (!str) return NULL;
    char* copy = malloc(strlen(str) + 1);
    strcpy(copy, str);
    return copy;
}

// ========== AST节点构造函数 ==========

ASTNode* createProgramNode() {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_PROGRAM;
    node->data.program.declarations = NULL;
    node->data.program.declaration_count = 0;
    return node;
}

ASTNode* createFunctionDeclarationNode(const char* return_type, const char* name, ASTNode* parameters) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION_DECLARATION;
    node->data.function.return_type = strdup_safe(return_type);
    node->data.function.name = strdup_safe(name);
    node->data.function.parameters = parameters;
    node->data.function.body = NULL;
    return node;
}

ASTNode* createFunctionDefinitionNode(const char* return_type, const char* name, ASTNode* parameters, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION_DEFINITION;
    node->data.function.return_type = strdup_safe(return_type);
    node->data.function.name = strdup_safe(name);
    node->data.function.parameters = parameters;
    node->data.function.body = body;
    return node;
}

ASTNode* createVariableDeclarationNode(const char* type, const char* name, ASTNode* initializer) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_VARIABLE_DECLARATION;
    node->data.variable_declaration.type = strdup_safe(type);
    node->data.variable_declaration.name = strdup_safe(name);
    node->data.variable_declaration.initializer = initializer;
    return node;
}

ASTNode* createBlockStatementNode() {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_BLOCK_STATEMENT;
    node->data.block.statements = NULL;
    node->data.block.statement_count = 0;
    return node;
}

ASTNode* createAssignmentStatementNode(const char* variable, ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_ASSIGNMENT_STATEMENT;
    node->data.assignment.variable = strdup_safe(variable);
    node->data.assignment.value = value;
    return node;
}

ASTNode* createIfStatementNode(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_IF_STATEMENT;
    node->data.if_statement.condition = condition;
    node->data.if_statement.then_stmt = then_stmt;
    node->data.if_statement.else_stmt = else_stmt;
    return node;
}

ASTNode* createWhileStatementNode(ASTNode* condition, ASTNode* body) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_WHILE_STATEMENT;
    node->data.while_statement.condition = condition;
    node->data.while_statement.body = body;
    return node;
}

ASTNode* createReturnStatementNode(ASTNode* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_RETURN_STATEMENT;
    node->data.return_statement.value = value;
    return node;
}

ASTNode* createExpressionStatementNode(ASTNode* expression) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_EXPRESSION_STATEMENT;
    // 表达式语句只是包装一个表达式，这里简化处理
    // 实际实现中可能需要专门的字段
    return node;
}

ASTNode* createBinaryExpressionNode(BinaryOperatorType operator, ASTNode* left, ASTNode* right) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_BINARY_EXPRESSION;
    node->data.binary_expression.operator = operator;
    node->data.binary_expression.left = left;
    node->data.binary_expression.right = right;
    return node;
}

ASTNode* createFunctionCallNode(const char* function_name, ASTNode* arguments) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_FUNCTION_CALL;
    node->data.function_call.function_name = strdup_safe(function_name);
    node->data.function_call.arguments = arguments;
    return node;
}

ASTNode* createIdentifierNode(const char* name) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_IDENTIFIER;
    node->data.identifier.name = strdup_safe(name);
    return node;
}

ASTNode* createLiteralNode(TokenType literal_type, const char* value) {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_LITERAL;
    node->data.literal.literal_type = literal_type;
    node->data.literal.value = strdup_safe(value);
    return node;
}

ASTNode* createListNode() {
    ASTNode* node = malloc(sizeof(ASTNode));
    node->type = AST_PARAMETER_LIST; // 或 AST_ARGUMENT_LIST，根据使用情况
    node->data.list.items = NULL;
    node->data.list.count = 0;
    return node;
}

// ========== AST操作函数 ==========

void addDeclarationToProgram(ASTNode* program, ASTNode* declaration) {
    if (program->type != AST_PROGRAM) return;

    program->data.program.declaration_count++;
    program->data.program.declarations = realloc(
        program->data.program.declarations,
        sizeof(ASTNode*) * program->data.program.declaration_count
    );
    program->data.program.declarations[program->data.program.declaration_count - 1] = declaration;
}

void addStatementToBlock(ASTNode* block, ASTNode* statement) {
    if (block->type != AST_BLOCK_STATEMENT) return;

    block->data.block.statement_count++;
    block->data.block.statements = realloc(
        block->data.block.statements,
        sizeof(ASTNode*) * block->data.block.statement_count
    );
    block->data.block.statements[block->data.block.statement_count - 1] = statement;
}

void addItemToList(ASTNode* list, ASTNode* item) {
    if (list->type != AST_PARAMETER_LIST && list->type != AST_ARGUMENT_LIST) return;

    list->data.list.count++;
    list->data.list.items = realloc(
        list->data.list.items,
        sizeof(ASTNode*) * list->data.list.count
    );
    list->data.list.items[list->data.list.count - 1] = item;
}

// ========== AST打印函数 ==========

const char* astNodeTypeToString(ASTNodeType type) {
    switch (type) {
    case AST_PROGRAM: return "Program";
    case AST_FUNCTION_DECLARATION: return "FunctionDeclaration";
    case AST_FUNCTION_DEFINITION: return "FunctionDefinition";
    case AST_VARIABLE_DECLARATION: return "VariableDeclaration";
    case AST_BLOCK_STATEMENT: return "BlockStatement";
    case AST_ASSIGNMENT_STATEMENT: return "AssignmentStatement";
    case AST_IF_STATEMENT: return "IfStatement";
    case AST_WHILE_STATEMENT: return "WhileStatement";
    case AST_RETURN_STATEMENT: return "ReturnStatement";
    case AST_EXPRESSION_STATEMENT: return "ExpressionStatement";
    case AST_BINARY_EXPRESSION: return "BinaryExpression";
    case AST_FUNCTION_CALL: return "FunctionCall";
    case AST_IDENTIFIER: return "Identifier";
    case AST_LITERAL: return "Literal";
    case AST_PARAMETER_LIST: return "ParameterList";
    case AST_ARGUMENT_LIST: return "ArgumentList";
    default: return "Unknown";
    }
}

const char* binaryOperatorToString(BinaryOperatorType op) {
    switch (op) {
    case BINARY_ADD: return "+";
    case BINARY_SUB: return "-";
    case BINARY_MUL: return "*";
    case BINARY_DIV: return "/";
    case BINARY_GT: return ">";
    case BINARY_LT: return "<";
    case BINARY_EQ: return "==";
    case BINARY_ASSIGN: return "=";
    default: return "?";
    }
}

void printAST(ASTNode* node, int indent) {
    if (!node) return;

    // 打印缩进
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    printf("%s", astNodeTypeToString(node->type));

    // 根据节点类型打印具体信息
    switch (node->type) {
    case AST_FUNCTION_DECLARATION:
    case AST_FUNCTION_DEFINITION:
        printf(" (%s %s)", node->data.function.return_type, node->data.function.name);
        break;
    case AST_VARIABLE_DECLARATION:
        printf(" (%s %s)", node->data.variable_declaration.type, node->data.variable_declaration.name);
        break;
    case AST_ASSIGNMENT_STATEMENT:
        printf(" (%s)", node->data.assignment.variable);
        break;
    case AST_BINARY_EXPRESSION:
        printf(" (%s)", binaryOperatorToString(node->data.binary_expression.operator));
        break;
    case AST_FUNCTION_CALL:
        printf(" (%s)", node->data.function_call.function_name);
        break;
    case AST_IDENTIFIER:
        printf(" (%s)", node->data.identifier.name);
        break;
    case AST_LITERAL:
        printf(" (%s)", node->data.literal.value);
        break;
    }

    printf("\n");

    // 递归打印子节点
    switch (node->type) {
    case AST_PROGRAM:
        for (int i = 0; i < node->data.program.declaration_count; i++) {
            printAST(node->data.program.declarations[i], indent + 1);
        }
        break;
    case AST_FUNCTION_DEFINITION:
        if (node->data.function.parameters) {
            printAST(node->data.function.parameters, indent + 1);
        }
        if (node->data.function.body) {
            printAST(node->data.function.body, indent + 1);
        }
        break;
    case AST_FUNCTION_DECLARATION:
        if (node->data.function.parameters) {
            printAST(node->data.function.parameters, indent + 1);
        }
        break;
    case AST_VARIABLE_DECLARATION:
        if (node->data.variable_declaration.initializer) {
            printAST(node->data.variable_declaration.initializer, indent + 1);
        }
        break;
    case AST_BLOCK_STATEMENT:
        for (int i = 0; i < node->data.block.statement_count; i++) {
            printAST(node->data.block.statements[i], indent + 1);
        }
        break;
    case AST_ASSIGNMENT_STATEMENT:
        printAST(node->data.assignment.value, indent + 1);
        break;
    case AST_IF_STATEMENT:
        printAST(node->data.if_statement.condition, indent + 1);
        printAST(node->data.if_statement.then_stmt, indent + 1);
        if (node->data.if_statement.else_stmt) {
            printAST(node->data.if_statement.else_stmt, indent + 1);
        }
        break;
    case AST_WHILE_STATEMENT:
        printAST(node->data.while_statement.condition, indent + 1);
        printAST(node->data.while_statement.body, indent + 1);
        break;
    case AST_RETURN_STATEMENT:
        if (node->data.return_statement.value) {
            printAST(node->data.return_statement.value, indent + 1);
        }
        break;
    case AST_BINARY_EXPRESSION:
        printAST(node->data.binary_expression.left, indent + 1);
        printAST(node->data.binary_expression.right, indent + 1);
        break;
    case AST_FUNCTION_CALL:
        if (node->data.function_call.arguments) {
            printAST(node->data.function_call.arguments, indent + 1);
        }
        break;
    case AST_PARAMETER_LIST:
    case AST_ARGUMENT_LIST:
        for (int i = 0; i < node->data.list.count; i++) {
            printAST(node->data.list.items[i], indent + 1);
        }
        break;
    }
}

// ========== AST内存管理 ==========

void freeAST(ASTNode* node) {
    if (!node) return;

    // 根据节点类型释放相应的内存
    switch (node->type) {
    case AST_PROGRAM:
        for (int i = 0; i < node->data.program.declaration_count; i++) {
            freeAST(node->data.program.declarations[i]);
        }
        free(node->data.program.declarations);
        break;
    case AST_FUNCTION_DECLARATION:
    case AST_FUNCTION_DEFINITION:
        free(node->data.function.return_type);
        free(node->data.function.name);
        freeAST(node->data.function.parameters);
        freeAST(node->data.function.body);
        break;
    case AST_VARIABLE_DECLARATION:
        free(node->data.variable_declaration.type);
        free(node->data.variable_declaration.name);
        freeAST(node->data.variable_declaration.initializer);
        break;
    case AST_BLOCK_STATEMENT:
        for (int i = 0; i < node->data.block.statement_count; i++) {
            freeAST(node->data.block.statements[i]);
        }
        free(node->data.block.statements);
        break;
    case AST_ASSIGNMENT_STATEMENT:
        free(node->data.assignment.variable);
        freeAST(node->data.assignment.value);
        break;
    case AST_IF_STATEMENT:
        freeAST(node->data.if_statement.condition);
        freeAST(node->data.if_statement.then_stmt);
        freeAST(node->data.if_statement.else_stmt);
        break;
    case AST_WHILE_STATEMENT:
        freeAST(node->data.while_statement.condition);
        freeAST(node->data.while_statement.body);
        break;
    case AST_RETURN_STATEMENT:
        freeAST(node->data.return_statement.value);
        break;
    case AST_BINARY_EXPRESSION:
        freeAST(node->data.binary_expression.left);
        freeAST(node->data.binary_expression.right);
        break;
    case AST_FUNCTION_CALL:
        free(node->data.function_call.function_name);
        freeAST(node->data.function_call.arguments);
        break;
    case AST_IDENTIFIER:
        free(node->data.identifier.name);
        break;
    case AST_LITERAL:
        free(node->data.literal.value);
        break;
    case AST_PARAMETER_LIST:
    case AST_ARGUMENT_LIST:
        for (int i = 0; i < node->data.list.count; i++) {
            freeAST(node->data.list.items[i]);
        }
        free(node->data.list.items);
        break;
    }

    free(node);
}