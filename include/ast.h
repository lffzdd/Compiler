/**
 * ast.h - 抽象语法树 (AST) 节点定义
 *
 * AST 是源代码的树形表示。每种语法结构对应一种节点类型。
 *
 * 设计思路：
 * - 使用枚举区分节点类型
 * - 使用联合体(union)存储不同类型的数据
 * - 这样可以用一个统一的 ASTNode 类型表示所有节点
 */

#ifndef AST_H
#define AST_H

#include "token.h"

// 前向声明
typedef struct ASTNode ASTNode;

/**
 * AST 节点类型枚举
 */
typedef enum {
  // ========== 顶层结构 ==========
  AST_PROGRAM, // 程序（包含多个声明）

  // ========== 声明 ==========
  AST_VAR_DECL,  // 变量声明: int x = 5;
  AST_FUNC_DECL, // 函数定义: int foo(int x) { ... }
  AST_PARAM,     // 函数参数: int x

  // ========== 语句 ==========
  AST_BLOCK,       // 代码块: { ... }
  AST_IF_STMT,     // if 语句
  AST_WHILE_STMT,  // while 语句
  AST_FOR_STMT,    // for 语句
  AST_RETURN_STMT, // return 语句
  AST_EXPR_STMT,   // 表达式语句: foo();

  // ========== 表达式 ==========
  AST_BINARY_EXPR, // 二元表达式: a + b
  AST_UNARY_EXPR,  // 一元表达式: -a, !b
  AST_CALL_EXPR,   // 函数调用: foo(1, 2)
  AST_ASSIGN_EXPR, // 赋值表达式: x = 5

  // ========== 基本元素 ==========
  AST_IDENTIFIER,     // 标识符: x, foo
  AST_INT_LITERAL,    // 整数字面量: 42
  AST_FLOAT_LITERAL,  // 浮点字面量: 3.14
  AST_STRING_LITERAL, // 字符串字面量: "hello"
  AST_CHAR_LITERAL    // 字符字面量: 'a'
} ASTNodeType;

/**
 * 二元运算符类型
 */
typedef enum {
  // 算术运算符
  OP_ADD, // +
  OP_SUB, // -
  OP_MUL, // *
  OP_DIV, // /
  OP_MOD, // %

  // 比较运算符
  OP_EQ, // ==
  OP_NE, // !=
  OP_LT, // <
  OP_GT, // >
  OP_LE, // <=
  OP_GE, // >=

  // 逻辑运算符
  OP_AND, // &&
  OP_OR   // ||
} BinaryOp;

/**
 * 一元运算符类型
 */
typedef enum {
  OP_NEG, // - (负号)
  OP_NOT  // ! (逻辑非)
} UnaryOp;

// ========== 各种节点的数据结构 ==========

/**
 * 程序节点数据
 * program → declaration*
 */
typedef struct {
  ASTNode **declarations; // 声明数组
  int count;              // 声明数量
  int capacity;           // 数组容量
} ProgramData;

/**
 * 变量声明节点数据
 * var_declaration → type IDENTIFIER ("=" expression)? ";"
 */
typedef struct {
  char *type;           // 类型: "int", "float", etc.
  char *name;           // 变量名
  ASTNode *initializer; // 初始值表达式（可为 NULL）
} VarDeclData;

/**
 * 函数声明节点数据
 * function → type IDENTIFIER "(" params ")" block
 */
typedef struct {
  char *return_type; // 返回类型
  char *name;        // 函数名
  ASTNode **params;  // 参数列表
  int param_count;   // 参数数量
  ASTNode *body;     // 函数体（Block 节点）
} FuncDeclData;

/**
 * 参数节点数据
 */
typedef struct {
  char *type; // 参数类型
  char *name; // 参数名
} ParamData;

/**
 * 代码块节点数据
 * block → "{" statement* "}"
 */
typedef struct {
  ASTNode **statements; // 语句数组
  int count;            // 语句数量
  int capacity;         // 数组容量
} BlockData;

/**
 * if 语句节点数据
 * if_stmt → "if" "(" expression ")" statement ("else" statement)?
 */
typedef struct {
  ASTNode *condition;   // 条件表达式
  ASTNode *then_branch; // then 分支
  ASTNode *else_branch; // else 分支（可为 NULL）
} IfStmtData;

/**
 * while 语句节点数据
 * while_stmt → "while" "(" expression ")" statement
 */
typedef struct {
  ASTNode *condition; // 条件表达式
  ASTNode *body;      // 循环体
} WhileStmtData;

/**
 * for 语句节点数据
 * for_stmt → "for" "(" expr? ";" expr? ";" expr? ")" statement
 */
typedef struct {
  ASTNode *init;      // 初始化（可为 NULL）
  ASTNode *condition; // 条件（可为 NULL，表示永真）
  ASTNode *update;    // 更新（可为 NULL）
  ASTNode *body;      // 循环体
} ForStmtData;

/**
 * return 语句节点数据
 * return_stmt → "return" expression? ";"
 */
typedef struct {
  ASTNode *value; // 返回值（可为 NULL）
} ReturnStmtData;

/**
 * 表达式语句节点数据
 * expr_stmt → expression ";"
 */
typedef struct {
  ASTNode *expression; // 表达式
} ExprStmtData;

/**
 * 二元表达式节点数据
 * binary_expr → expression op expression
 */
typedef struct {
  BinaryOp op;    // 运算符
  ASTNode *left;  // 左操作数
  ASTNode *right; // 右操作数
} BinaryExprData;

/**
 * 一元表达式节点数据
 * unary_expr → op expression
 */
typedef struct {
  UnaryOp op;       // 运算符
  ASTNode *operand; // 操作数
} UnaryExprData;

/**
 * 函数调用节点数据
 * call_expr → IDENTIFIER "(" arguments ")"
 */
typedef struct {
  char *callee;        // 被调用的函数名
  ASTNode **arguments; // 参数列表
  int arg_count;       // 参数数量
} CallExprData;

/**
 * 赋值表达式节点数据
 * assign_expr → IDENTIFIER "=" expression
 */
typedef struct {
  char *name;     // 变量名
  ASTNode *value; // 赋值表达式
} AssignExprData;

/**
 * 标识符节点数据
 */
typedef struct {
  char *name; // 标识符名称
} IdentifierData;

/**
 * 整数字面量节点数据
 */
typedef struct {
  int value; // 整数值
} IntLiteralData;

/**
 * 浮点字面量节点数据
 */
typedef struct {
  double value; // 浮点值
} FloatLiteralData;

/**
 * 字符串字面量节点数据
 */
typedef struct {
  char *value; // 字符串内容
} StringLiteralData;

/**
 * 字符字面量节点数据
 */
typedef struct {
  char value; // 字符值
} CharLiteralData;

// ========== AST 节点结构体 ==========

/**
 * 统一的 AST 节点结构
 *
 * 使用联合体(union)来存储不同类型的数据。
 * 这样所有节点都可以用 ASTNode* 类型表示，方便统一处理。
 */
struct ASTNode {
  ASTNodeType type; // 节点类型
  int line;         // 源代码行号（用于错误报告）
  int column;       // 源代码列号

  // 使用联合体存储不同类型的数据
  // 同一时间只会使用其中一个
  union {
    ProgramData program;
    VarDeclData var_decl;
    FuncDeclData func_decl;
    ParamData param;
    BlockData block;
    IfStmtData if_stmt;
    WhileStmtData while_stmt;
    ForStmtData for_stmt;
    ReturnStmtData return_stmt;
    ExprStmtData expr_stmt;
    BinaryExprData binary_expr;
    UnaryExprData unary_expr;
    CallExprData call_expr;
    AssignExprData assign_expr;
    IdentifierData identifier;
    IntLiteralData int_literal;
    FloatLiteralData float_literal;
    StringLiteralData string_literal;
    CharLiteralData char_literal;
  } data;
};

// ========== AST 操作函数声明 ==========

// 创建各种节点
ASTNode *ast_create_program(void);
ASTNode *ast_create_var_decl(const char *type, const char *name, ASTNode *init);
ASTNode *ast_create_func_decl(const char *ret_type, const char *name,
                              ASTNode **params, int param_count, ASTNode *body);
ASTNode *ast_create_param(const char *type, const char *name);
ASTNode *ast_create_block(void);
ASTNode *ast_create_if_stmt(ASTNode *cond, ASTNode *then_br, ASTNode *else_br);
ASTNode *ast_create_while_stmt(ASTNode *cond, ASTNode *body);
ASTNode *ast_create_for_stmt(ASTNode *init, ASTNode *cond, ASTNode *update,
                             ASTNode *body);
ASTNode *ast_create_return_stmt(ASTNode *value);
ASTNode *ast_create_expr_stmt(ASTNode *expr);
ASTNode *ast_create_binary_expr(BinaryOp op, ASTNode *left, ASTNode *right);
ASTNode *ast_create_unary_expr(UnaryOp op, ASTNode *operand);
ASTNode *ast_create_call_expr(const char *callee, ASTNode **args,
                              int arg_count);
ASTNode *ast_create_assign_expr(const char *name, ASTNode *value);
ASTNode *ast_create_identifier(const char *name);
ASTNode *ast_create_int_literal(int value);
ASTNode *ast_create_float_literal(double value);
ASTNode *ast_create_string_literal(const char *value);
ASTNode *ast_create_char_literal(char value);

// 添加子节点
void ast_program_add(ASTNode *program, ASTNode *decl);
void ast_block_add(ASTNode *block, ASTNode *stmt);

// 打印 AST（用于调试）
void ast_print(ASTNode *node, int indent);

// 释放 AST 内存
void ast_free(ASTNode *node);

// 辅助函数
const char *ast_node_type_to_string(ASTNodeType type);
const char *ast_binary_op_to_string(BinaryOp op);
const char *ast_unary_op_to_string(UnaryOp op);

#endif // AST_H
