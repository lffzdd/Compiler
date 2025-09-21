#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "token.h"
#include <stdbool.h>

// ===================== 数据类型定义 =====================

// 基本数据类型枚举
typedef enum {
    SEMANTIC_TYPE_VOID,     // void类型
    SEMANTIC_TYPE_INT,      // 整数类型
    SEMANTIC_TYPE_FLOAT,    // 浮点数类型
    SEMANTIC_TYPE_CHAR,     // 字符类型
    SEMANTIC_TYPE_STRING,   // 字符串类型
    SEMANTIC_TYPE_BOOL,     // 布尔类型
    SEMANTIC_TYPE_FUNCTION, // 函数类型
    SEMANTIC_TYPE_ERROR     // 错误类型（用于错误处理）
} SemanticType;

// 函数参数信息
typedef struct FunctionParameter {
    char* name;             // 参数名
    SemanticType type;      // 参数类型
    struct FunctionParameter* next; // 下一个参数
} FunctionParameter;

// 函数信息
typedef struct FunctionInfo {
    char* name;             // 函数名
    SemanticType return_type; // 返回类型
    FunctionParameter* parameters; // 参数列表
    int parameter_count;    // 参数个数
    bool is_defined;        // 是否已定义（区分声明和定义）
    int line_number;        // 定义/声明行号
} FunctionInfo;

// 变量信息
typedef struct VariableInfo {
    char* name;             // 变量名
    SemanticType type;      // 变量类型
    bool is_initialized;    // 是否已初始化
    bool is_used;           // 是否被使用过
    int line_number;        // 定义行号
} VariableInfo;

// 符号表项类型
typedef enum {
    SYMBOL_VARIABLE,        // 变量符号
    SYMBOL_FUNCTION         // 函数符号
} SymbolType;

// 符号表项
typedef struct SymbolTableEntry {
    char* name;             // 符号名
    SymbolType symbol_type; // 符号类型
    union {
        VariableInfo* var_info;  // 变量信息
        FunctionInfo* func_info; // 函数信息
    };
    struct SymbolTableEntry* next; // 链表下一项
} SymbolTableEntry;

// 作用域（符号表）
typedef struct Scope {
    SymbolTableEntry* symbols; // 符号表项链表
    struct Scope* parent;       // 父作用域
    struct Scope* children;     // 子作用域链表
    struct Scope* next_sibling; // 兄弟作用域
    int scope_level;            // 作用域层级（0为全局作用域）
} Scope;

// 语义分析器上下文
typedef struct SemanticAnalyzer {
    Scope* global_scope;        // 全局作用域
    Scope* current_scope;       // 当前作用域
    FunctionInfo* current_function; // 当前正在分析的函数
    int error_count;            // 错误计数
    bool has_main_function;     // 是否有main函数
} SemanticAnalyzer;

// 语义错误类型
typedef enum {
    SEMANTIC_ERROR_UNDEFINED_VARIABLE,      // 未定义的变量
    SEMANTIC_ERROR_UNDEFINED_FUNCTION,      // 未定义的函数
    SEMANTIC_ERROR_REDECLARED_VARIABLE,     // 重复声明变量
    SEMANTIC_ERROR_REDECLARED_FUNCTION,     // 重复声明函数
    SEMANTIC_ERROR_TYPE_MISMATCH,           // 类型不匹配
    SEMANTIC_ERROR_INVALID_ASSIGNMENT,      // 无效赋值
    SEMANTIC_ERROR_INVALID_OPERATION,       // 无效操作
    SEMANTIC_ERROR_FUNCTION_CALL_MISMATCH,  // 函数调用参数不匹配
    SEMANTIC_ERROR_RETURN_TYPE_MISMATCH,    // 返回类型不匹配
    SEMANTIC_ERROR_UNINITIALIZED_VARIABLE,  // 使用未初始化变量
    SEMANTIC_ERROR_NO_MAIN_FUNCTION,        // 缺少main函数
    SEMANTIC_ERROR_VOID_ASSIGNMENT          // void类型赋值错误
} SemanticErrorType;

// ===================== 符号表管理函数 =====================

// 创建和销毁
SemanticAnalyzer* createSemanticAnalyzer(void);
void destroySemanticAnalyzer(SemanticAnalyzer* analyzer);

// 作用域管理
Scope* createScope(Scope* parent, int level);
void destroyScope(Scope* scope);
void enterScope(SemanticAnalyzer* analyzer);
void exitScope(SemanticAnalyzer* analyzer);

// 符号表操作
SymbolTableEntry* createVariableSymbol(const char* name, SemanticType type, int line_number);
SymbolTableEntry* createFunctionSymbol(const char* name, SemanticType return_type, 
                                      FunctionParameter* params, int param_count, int line_number);
bool addSymbol(Scope* scope, SymbolTableEntry* symbol);
SymbolTableEntry* lookupSymbol(Scope* scope, const char* name);
SymbolTableEntry* lookupSymbolInCurrentScope(Scope* scope, const char* name);

// 函数参数管理
FunctionParameter* createFunctionParameter(const char* name, SemanticType type);
void addFunctionParameter(FunctionInfo* func_info, FunctionParameter* param);
void destroyFunctionParameters(FunctionParameter* params);

// ===================== 类型系统函数 =====================

// 类型转换和检查
SemanticType stringToSemanticType(const char* type_str);
const char* semanticTypeToString(SemanticType type);
bool isNumericType(SemanticType type);
bool isCompatibleTypes(SemanticType type1, SemanticType type2);
SemanticType getPromotedType(SemanticType type1, SemanticType type2);
bool canImplicitlyCast(SemanticType from, SemanticType to);

// 表达式类型推导
SemanticType inferBinaryOperationType(BinaryOperatorType op, SemanticType left, SemanticType right);
SemanticType inferExpressionType(SemanticAnalyzer* analyzer, ASTNode* expr);

// ===================== 语义分析主要函数 =====================

// 主分析函数
bool performSemanticAnalysis(ASTNode* ast);
bool analyzeNode(SemanticAnalyzer* analyzer, ASTNode* node);

// 具体节点分析函数
bool analyzeProgram(SemanticAnalyzer* analyzer, ASTNode* program);
bool analyzeDeclaration(SemanticAnalyzer* analyzer, ASTNode* declaration);
bool analyzeVariableDeclaration(SemanticAnalyzer* analyzer, ASTNode* var_decl);
bool analyzeFunctionDeclaration(SemanticAnalyzer* analyzer, ASTNode* func_decl);
bool analyzeFunctionDefinition(SemanticAnalyzer* analyzer, ASTNode* func_def);
bool analyzeStatement(SemanticAnalyzer* analyzer, ASTNode* statement);
bool analyzeExpression(SemanticAnalyzer* analyzer, ASTNode* expression);
bool analyzeAssignment(SemanticAnalyzer* analyzer, ASTNode* assignment);
bool analyzeFunctionCall(SemanticAnalyzer* analyzer, ASTNode* func_call);
bool analyzeIfStatement(SemanticAnalyzer* analyzer, ASTNode* if_stmt);
bool analyzeWhileStatement(SemanticAnalyzer* analyzer, ASTNode* while_stmt);
bool analyzeReturnStatement(SemanticAnalyzer* analyzer, ASTNode* return_stmt);
bool analyzeBlock(SemanticAnalyzer* analyzer, ASTNode* block);

// ===================== 错误处理函数 =====================

// 错误报告
void reportSemanticError(SemanticAnalyzer* analyzer, SemanticErrorType error_type, 
                        const char* message, int line_number);
void reportUndefinedVariable(SemanticAnalyzer* analyzer, const char* var_name, int line_number);
void reportUndefinedFunction(SemanticAnalyzer* analyzer, const char* func_name, int line_number);
void reportRedeclaredSymbol(SemanticAnalyzer* analyzer, const char* symbol_name, int line_number);
void reportTypeMismatch(SemanticAnalyzer* analyzer, SemanticType expected, 
                       SemanticType actual, int line_number);
void reportFunctionCallMismatch(SemanticAnalyzer* analyzer, const char* func_name, 
                               int expected_params, int actual_params, int line_number);

// ===================== 调试和实用函数 =====================

// 调试输出
void printSymbolTable(Scope* scope, int indent_level);
void printSemanticAnalysisResults(SemanticAnalyzer* analyzer);
void dumpSymbolTables(SemanticAnalyzer* analyzer);

// 实用函数
bool isMainFunction(const char* func_name);
int getLineNumberFromNode(ASTNode* node);
void markVariableAsUsed(SemanticAnalyzer* analyzer, const char* var_name);
void checkUnusedVariables(SemanticAnalyzer* analyzer);

#endif // SEMANTIC_H