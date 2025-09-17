#ifndef AST_H
#define AST_H

#include "token.h"

// AST节点类型
typedef enum {
    // 程序和声明
    AST_PROGRAM,
    AST_FUNCTION_DECLARATION,
    AST_FUNCTION_DEFINITION,
    AST_VARIABLE_DECLARATION,

    // 语句
    AST_BLOCK_STATEMENT,
    AST_ASSIGNMENT_STATEMENT,
    AST_IF_STATEMENT,
    AST_WHILE_STATEMENT,
    AST_RETURN_STATEMENT,
    AST_EXPRESSION_STATEMENT,

    // 表达式
    AST_BINARY_EXPRESSION,    // 二元表达式 (a + b, a > b)
    AST_FUNCTION_CALL,        // 函数调用
    AST_IDENTIFIER,           // 标识符
    AST_LITERAL,              // 字面值 (数字、字符串等)

    // 其他
    AST_PARAMETER_LIST,
    AST_ARGUMENT_LIST
} ASTNodeType;

// 二元操作符类型
typedef enum {
    BINARY_ADD,     // +
    BINARY_SUB,     // -
    BINARY_MUL,     // *
    BINARY_DIV,     // /
    BINARY_GT,      // >
    BINARY_LT,      // <
    BINARY_EQ,      // ==
    BINARY_ASSIGN   // =
} BinaryOperatorType;

// 前向声明
struct ASTNode;

// AST节点结构
typedef struct ASTNode {
    ASTNodeType type;

    // 节点数据（联合体，根据type使用不同字段）
    union {
        // 程序节点
        struct {
            struct ASTNode** declarations;  // 声明列表
            int declaration_count;
        } program;

        // 函数声明/定义
        struct {
            char* return_type;              // 返回类型
            char* name;                     // 函数名
            struct ASTNode* parameters;     // 参数列表
            struct ASTNode* body;           // 函数体 (仅定义有)
        } function;

        // 变量声明
        struct {
            char* type;                     // 变量类型
            char* name;                     // 变量名
            struct ASTNode* initializer;    // 初始化表达式 (可选)
        } variable_declaration;

        // 语句块
        struct {
            struct ASTNode** statements;    // 语句列表
            int statement_count;
        } block;

        // 赋值语句
        struct {
            char* variable;                 // 变量名
            struct ASTNode* value;          // 赋值表达式
        } assignment;

        // if语句
        struct {
            struct ASTNode* condition;      // 条件表达式
            struct ASTNode* then_stmt;      // then分支
            struct ASTNode* else_stmt;      // else分支 (可选)
        } if_statement;

        // while语句
        struct {
            struct ASTNode* condition;      // 条件表达式
            struct ASTNode* body;           // 循环体
        } while_statement;

        // return语句
        struct {
            struct ASTNode* value;          // 返回值表达式 (可选)
        } return_statement;

        // 二元表达式
        struct {
            BinaryOperatorType operator;    // 操作符
            struct ASTNode* left;           // 左操作数
            struct ASTNode* right;          // 右操作数
        } binary_expression;

        // 函数调用
        struct {
            char* function_name;            // 函数名
            struct ASTNode* arguments;      // 参数列表
        } function_call;

        // 标识符
        struct {
            char* name;                     // 标识符名称
        } identifier;

        // 字面值
        struct {
            TokenType literal_type;         // 字面值类型 (INTEGER, STRING等)
            char* value;                    // 字面值
        } literal;

        // 参数/参数列表
        struct {
            struct ASTNode** items;         // 参数/参数项列表
            int count;
        } list;
    } data;
} ASTNode;

// AST构造函数
ASTNode* createProgramNode();
ASTNode* createFunctionDeclarationNode(const char* return_type, const char* name, ASTNode* parameters);
ASTNode* createFunctionDefinitionNode(const char* return_type, const char* name, ASTNode* parameters, ASTNode* body);
ASTNode* createVariableDeclarationNode(const char* type, const char* name, ASTNode* initializer);
ASTNode* createBlockStatementNode();
ASTNode* createAssignmentStatementNode(const char* variable, ASTNode* value);
ASTNode* createIfStatementNode(ASTNode* condition, ASTNode* then_stmt, ASTNode* else_stmt);
ASTNode* createWhileStatementNode(ASTNode* condition, ASTNode* body);
ASTNode* createReturnStatementNode(ASTNode* value);
ASTNode* createExpressionStatementNode(ASTNode* expression);
ASTNode* createBinaryExpressionNode(BinaryOperatorType operator, ASTNode* left, ASTNode* right);
ASTNode* createFunctionCallNode(const char* function_name, ASTNode* arguments);
ASTNode* createIdentifierNode(const char* name);
ASTNode* createLiteralNode(TokenType literal_type, const char* value);
ASTNode* createListNode();

// AST操作函数
void addDeclarationToProgram(ASTNode* program, ASTNode* declaration);
void addStatementToBlock(ASTNode* block, ASTNode* statement);
void addItemToList(ASTNode* list, ASTNode* item);

// AST打印函数 (用于调试)
void printAST(ASTNode* node, int indent);
const char* astNodeTypeToString(ASTNodeType type);
const char* binaryOperatorToString(BinaryOperatorType op);

// AST内存管理
void freeAST(ASTNode* node);

#endif // AST_H