/**
 * semantic.h - 语义分析器头文件
 *
 * 语义分析器的职责：
 * 1. 管理符号表 - 记录所有变量和函数
 * 2. 作用域管理 - 跟踪变量的可见范围
 * 3. 类型检查 - 确保操作数类型兼容
 * 4. 报告语义错误
 */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"

// ========== 类型系统 ==========

/**
 * 语义类型枚举
 *
 * 这是程序的"真实"类型，不同于 Token 类型。
 * 语义分析时用这个来做类型检查。
 */
typedef enum {
  TYPE_VOID,
  TYPE_INT,
  TYPE_FLOAT,
  TYPE_CHAR,
  TYPE_STRING,
  TYPE_BOOL,
  TYPE_UNKNOWN, // 类型未知（错误情况）
  TYPE_ERROR    // 类型错误（传播错误用）
} DataType;

// ========== 符号定义 ==========

/**
 * 符号类型
 */
typedef enum {
  SYMBOL_VARIABLE, // 变量
  SYMBOL_FUNCTION, // 函数
  SYMBOL_PARAMETER // 函数参数
} SymbolKind;

/**
 * 函数参数信息
 */
typedef struct {
  char *name;    // 参数名
  DataType type; // 参数类型
} ParamInfo;

/**
 * 符号结构体
 *
 * 代表符号表中的一个条目（变量、函数等）
 */
typedef struct Symbol {
  char *name;         // 符号名称
  SymbolKind kind;    // 符号类型（变量/函数/参数）
  DataType data_type; // 数据类型

  // 函数特有信息
  ParamInfo *params;    // 参数列表（函数用）
  int param_count;      // 参数数量
  DataType return_type; // 返回类型（函数用）

  // 位置信息（用于错误报告）
  int line;
  int column;

  struct Symbol *next; // 链表下一个（用于哈希冲突）
} Symbol;

// ========== 作用域定义 ==========

/**
 * 符号表大小（哈希表桶数）
 */
#define SYMBOL_TABLE_SIZE 64

/**
 * 作用域结构体
 *
 * 每个作用域包含一个符号表（哈希表）。
 * 作用域之间通过 parent 指针链接形成栈。
 */
typedef struct Scope {
  Symbol *symbols[SYMBOL_TABLE_SIZE]; // 符号哈希表
  struct Scope *parent;               // 父作用域
  int level;                          // 作用域层级（0=全局）
} Scope;

// ========== 语义分析器 ==========

/**
 * 语义错误类型
 */
typedef enum {
  SEM_ERROR_NONE,
  SEM_ERROR_UNDECLARED_VAR,  // 未声明的变量
  SEM_ERROR_UNDECLARED_FUNC, // 未声明的函数
  SEM_ERROR_REDECLARED,      // 重复声明
  SEM_ERROR_TYPE_MISMATCH,   // 类型不匹配
  SEM_ERROR_WRONG_ARG_COUNT, // 参数数量错误
  SEM_ERROR_WRONG_ARG_TYPE,  // 参数类型错误
  SEM_ERROR_RETURN_TYPE,     // 返回类型错误
  SEM_ERROR_VOID_VARIABLE,   // void 类型的变量
  SEM_ERROR_NOT_CALLABLE     // 调用了非函数
} SemanticErrorType;

/**
 * 语义错误信息
 */
typedef struct SemanticError {
  SemanticErrorType type;
  char message[256];
  int line;
  int column;
  struct SemanticError *next;
} SemanticError;

/**
 * 语义分析器结构体
 */
typedef struct {
  Scope *current_scope;  // 当前作用域
  Scope *global_scope;   // 全局作用域
  SemanticError *errors; // 错误链表
  int error_count;       // 错误数量

  // 当前函数信息（用于检查 return 语句）
  Symbol *current_function;
} SemanticAnalyzer;

// ========== 函数声明 ==========

// 初始化和清理
SemanticAnalyzer *semantic_init(void);
void semantic_free(SemanticAnalyzer *analyzer);

// 作用域管理
void semantic_enter_scope(SemanticAnalyzer *analyzer);
void semantic_exit_scope(SemanticAnalyzer *analyzer);

// 符号表操作
Symbol *semantic_declare(SemanticAnalyzer *analyzer, const char *name,
                         SymbolKind kind, DataType type);
Symbol *semantic_lookup(SemanticAnalyzer *analyzer, const char *name);
Symbol *semantic_lookup_current_scope(SemanticAnalyzer *analyzer,
                                      const char *name);

// 类型转换
DataType string_to_datatype(const char *type_str);
const char *datatype_to_string(DataType type);

// 类型检查
DataType semantic_check_binary_expr(SemanticAnalyzer *analyzer, BinaryOp op,
                                    DataType left, DataType right);
int semantic_types_compatible(DataType expected, DataType actual);

// 分析 AST
void semantic_analyze(SemanticAnalyzer *analyzer, ASTNode *ast);

// 错误处理
void semantic_error(SemanticAnalyzer *analyzer, SemanticErrorType type,
                    int line, const char *format, ...);
int semantic_has_errors(SemanticAnalyzer *analyzer);
void semantic_print_errors(SemanticAnalyzer *analyzer);

#endif // SEMANTIC_H
