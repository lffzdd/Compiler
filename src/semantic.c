#include "../include/semantic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// ===================== 创建和销毁函数 =====================

SemanticAnalyzer* createSemanticAnalyzer(void) {
    SemanticAnalyzer* analyzer = (SemanticAnalyzer*)malloc(sizeof(SemanticAnalyzer));
    if (!analyzer) {
        printf("Memory allocation failed for SemanticAnalyzer\n");
        return NULL;
    }
    
    analyzer->global_scope = createScope(NULL, 0);
    analyzer->current_scope = analyzer->global_scope;
    analyzer->current_function = NULL;
    analyzer->error_count = 0;
    analyzer->has_main_function = false;
    
    return analyzer;
}

void destroySemanticAnalyzer(SemanticAnalyzer* analyzer) {
    if (!analyzer) return;
    
    destroyScope(analyzer->global_scope);
    free(analyzer);
}

Scope* createScope(Scope* parent, int level) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    if (!scope) {
        printf("Memory allocation failed for Scope\n");
        return NULL;
    }
    
    scope->symbols = NULL;
    scope->parent = parent;
    scope->children = NULL;
    scope->next_sibling = NULL;
    scope->scope_level = level;
    
    return scope;
}

void destroyScope(Scope* scope) {
    if (!scope) return;
    
    // 销毁符号表项
    SymbolTableEntry* current = scope->symbols;
    while (current) {
        SymbolTableEntry* next = current->next;
        
        if (current->name) free(current->name);
        
        if (current->symbol_type == SYMBOL_VARIABLE && current->var_info) {
            if (current->var_info->name) free(current->var_info->name);
            free(current->var_info);
        } else if (current->symbol_type == SYMBOL_FUNCTION && current->func_info) {
            if (current->func_info->name) free(current->func_info->name);
            destroyFunctionParameters(current->func_info->parameters);
            free(current->func_info);
        }
        
        free(current);
        current = next;
    }
    
    // 销毁子作用域
    Scope* child = scope->children;
    while (child) {
        Scope* next_child = child->next_sibling;
        destroyScope(child);
        child = next_child;
    }
    
    free(scope);
}

// ===================== 作用域管理函数 =====================

void enterScope(SemanticAnalyzer* analyzer) {
    if (!analyzer) return;
    
    Scope* new_scope = createScope(analyzer->current_scope, analyzer->current_scope->scope_level + 1);
    
    // 将新作用域添加到当前作用域的子作用域链表中
    if (analyzer->current_scope->children == NULL) {
        analyzer->current_scope->children = new_scope;
    } else {
        Scope* child = analyzer->current_scope->children;
        while (child->next_sibling) {
            child = child->next_sibling;
        }
        child->next_sibling = new_scope;
    }
    
    analyzer->current_scope = new_scope;
}

void exitScope(SemanticAnalyzer* analyzer) {
    if (!analyzer || !analyzer->current_scope->parent) return;
    
    analyzer->current_scope = analyzer->current_scope->parent;
}

// ===================== 符号表操作函数 =====================

SymbolTableEntry* createVariableSymbol(const char* name, SemanticType type, int line_number) {
    SymbolTableEntry* symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
    if (!symbol) return NULL;
    
    symbol->name = strdup(name);
    symbol->symbol_type = SYMBOL_VARIABLE;
    symbol->next = NULL;
    
    symbol->var_info = (VariableInfo*)malloc(sizeof(VariableInfo));
    symbol->var_info->name = strdup(name);
    symbol->var_info->type = type;
    symbol->var_info->is_initialized = false;
    symbol->var_info->is_used = false;
    symbol->var_info->line_number = line_number;
    
    return symbol;
}

SymbolTableEntry* createFunctionSymbol(const char* name, SemanticType return_type, 
                                      FunctionParameter* params, int param_count, int line_number) {
    SymbolTableEntry* symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
    if (!symbol) return NULL;
    
    symbol->name = strdup(name);
    symbol->symbol_type = SYMBOL_FUNCTION;
    symbol->next = NULL;
    
    symbol->func_info = (FunctionInfo*)malloc(sizeof(FunctionInfo));
    symbol->func_info->name = strdup(name);
    symbol->func_info->return_type = return_type;
    symbol->func_info->parameters = params;
    symbol->func_info->parameter_count = param_count;
    symbol->func_info->is_defined = false;
    symbol->func_info->line_number = line_number;
    
    return symbol;
}

bool addSymbol(Scope* scope, SymbolTableEntry* symbol) {
    if (!scope || !symbol) return false;
    
    // 检查是否已经存在同名符号（仅在当前作用域中）
    SymbolTableEntry* existing = lookupSymbolInCurrentScope(scope, symbol->name);
    if (existing) {
        return false; // 符号已存在
    }
    
    // 添加到符号表链表头部
    symbol->next = scope->symbols;
    scope->symbols = symbol;
    
    return true;
}

SymbolTableEntry* lookupSymbol(Scope* scope, const char* name) {
    if (!scope || !name) return NULL;
    
    // 从当前作用域开始向上查找
    Scope* current_scope = scope;
    while (current_scope) {
        SymbolTableEntry* symbol = current_scope->symbols;
        while (symbol) {
            if (strcmp(symbol->name, name) == 0) {
                return symbol;
            }
            symbol = symbol->next;
        }
        current_scope = current_scope->parent;
    }
    
    return NULL; // 未找到
}

SymbolTableEntry* lookupSymbolInCurrentScope(Scope* scope, const char* name) {
    if (!scope || !name) return NULL;
    
    SymbolTableEntry* symbol = scope->symbols;
    while (symbol) {
        if (strcmp(symbol->name, name) == 0) {
            return symbol;
        }
        symbol = symbol->next;
    }
    
    return NULL;
}

// ===================== 函数参数管理 =====================

FunctionParameter* createFunctionParameter(const char* name, SemanticType type) {
    FunctionParameter* param = (FunctionParameter*)malloc(sizeof(FunctionParameter));
    if (!param) return NULL;
    
    param->name = strdup(name);
    param->type = type;
    param->next = NULL;
    
    return param;
}

void addFunctionParameter(FunctionInfo* func_info, FunctionParameter* param) {
    if (!func_info || !param) return;
    
    if (func_info->parameters == NULL) {
        func_info->parameters = param;
    } else {
        FunctionParameter* current = func_info->parameters;
        while (current->next) {
            current = current->next;
        }
        current->next = param;
    }
    func_info->parameter_count++;
}

void destroyFunctionParameters(FunctionParameter* params) {
    while (params) {
        FunctionParameter* next = params->next;
        if (params->name) free(params->name);
        free(params);
        params = next;
    }
}

// ===================== 类型系统函数 =====================

SemanticType stringToSemanticType(const char* type_str) {
    if (!type_str) return SEMANTIC_TYPE_ERROR;
    
    if (strcmp(type_str, "void") == 0) return SEMANTIC_TYPE_VOID;
    if (strcmp(type_str, "int") == 0) return SEMANTIC_TYPE_INT;
    if (strcmp(type_str, "float") == 0) return SEMANTIC_TYPE_FLOAT;
    if (strcmp(type_str, "char") == 0) return SEMANTIC_TYPE_CHAR;
    if (strcmp(type_str, "string") == 0) return SEMANTIC_TYPE_STRING;
    if (strcmp(type_str, "bool") == 0) return SEMANTIC_TYPE_BOOL;
    
    return SEMANTIC_TYPE_ERROR;
}

const char* semanticTypeToString(SemanticType type) {
    switch (type) {
        case SEMANTIC_TYPE_VOID: return "void";
        case SEMANTIC_TYPE_INT: return "int";
        case SEMANTIC_TYPE_FLOAT: return "float";
        case SEMANTIC_TYPE_CHAR: return "char";
        case SEMANTIC_TYPE_STRING: return "string";
        case SEMANTIC_TYPE_BOOL: return "bool";
        case SEMANTIC_TYPE_FUNCTION: return "function";
        case SEMANTIC_TYPE_ERROR: return "error";
        default: return "unknown";
    }
}

bool isNumericType(SemanticType type) {
    return type == SEMANTIC_TYPE_INT || type == SEMANTIC_TYPE_FLOAT;
}

bool isCompatibleTypes(SemanticType type1, SemanticType type2) {
    if (type1 == type2) return true;
    
    // 数值类型之间可以兼容
    if (isNumericType(type1) && isNumericType(type2)) return true;
    
    // char可以与int兼容
    if ((type1 == SEMANTIC_TYPE_CHAR && type2 == SEMANTIC_TYPE_INT) ||
        (type1 == SEMANTIC_TYPE_INT && type2 == SEMANTIC_TYPE_CHAR)) return true;
    
    return false;
}

SemanticType getPromotedType(SemanticType type1, SemanticType type2) {
    if (type1 == SEMANTIC_TYPE_ERROR || type2 == SEMANTIC_TYPE_ERROR) {
        return SEMANTIC_TYPE_ERROR;
    }
    
    // float优先于int
    if (type1 == SEMANTIC_TYPE_FLOAT || type2 == SEMANTIC_TYPE_FLOAT) {
        return SEMANTIC_TYPE_FLOAT;
    }
    
    // int优先于char
    if (type1 == SEMANTIC_TYPE_INT || type2 == SEMANTIC_TYPE_INT) {
        return SEMANTIC_TYPE_INT;
    }
    
    // 其他情况保持原类型
    if (type1 == type2) return type1;
    
    return SEMANTIC_TYPE_ERROR;
}

bool canImplicitlyCast(SemanticType from, SemanticType to) {
    if (from == to) return true;
    
    // 数值类型的隐式转换
    if (from == SEMANTIC_TYPE_INT && to == SEMANTIC_TYPE_FLOAT) return true;
    if (from == SEMANTIC_TYPE_CHAR && to == SEMANTIC_TYPE_INT) return true;
    if (from == SEMANTIC_TYPE_CHAR && to == SEMANTIC_TYPE_FLOAT) return true;
    
    return false;
}

// ===================== 表达式类型推导 =====================

SemanticType inferBinaryOperationType(BinaryOperatorType op, SemanticType left, SemanticType right) {
    switch (op) {
        case BINARY_ADD:
        case BINARY_SUB:
        case BINARY_MUL:
        case BINARY_DIV:
            // 算术运算：要求操作数为数值类型
            if (isNumericType(left) && isNumericType(right)) {
                return getPromotedType(left, right);
            }
            // 字符串连接（仅加法）
            if (op == BINARY_ADD && left == SEMANTIC_TYPE_STRING && right == SEMANTIC_TYPE_STRING) {
                return SEMANTIC_TYPE_STRING;
            }
            return SEMANTIC_TYPE_ERROR;
            
        case BINARY_GT:
        case BINARY_LT:
        case BINARY_GE:
        case BINARY_LE:
            // 比较运算：要求操作数类型兼容，结果为bool
            if (isCompatibleTypes(left, right)) {
                return SEMANTIC_TYPE_BOOL;
            }
            return SEMANTIC_TYPE_ERROR;
            
        case BINARY_EQ:
        case BINARY_NE:
            // 等值比较：要求操作数类型兼容，结果为bool
            if (isCompatibleTypes(left, right)) {
                return SEMANTIC_TYPE_BOOL;
            }
            return SEMANTIC_TYPE_ERROR;
            
        case BINARY_ASSIGN:
            // 赋值：要求右值可以转换为左值类型
            if (canImplicitlyCast(right, left)) {
                return left;
            }
            return SEMANTIC_TYPE_ERROR;
            
        default:
            return SEMANTIC_TYPE_ERROR;
    }
}

SemanticType inferExpressionType(SemanticAnalyzer* analyzer, ASTNode* expr) {
    if (!analyzer || !expr) return SEMANTIC_TYPE_ERROR;
    
    switch (expr->type) {
        case AST_LITERAL: {
            // 根据字面值类型推导
            if (expr->data.literal.token_type == TOKEN_INTEGER) {
                return SEMANTIC_TYPE_INT;
            } else if (expr->data.literal.token_type == TOKEN_FLOAT) {
                return SEMANTIC_TYPE_FLOAT;
            } else if (expr->data.literal.token_type == TOKEN_CHAR) {
                return SEMANTIC_TYPE_CHAR;
            } else if (expr->data.literal.token_type == TOKEN_STRING) {
                return SEMANTIC_TYPE_STRING;
            } else if (expr->data.literal.token_type == TOKEN_BOOL) {
                return SEMANTIC_TYPE_BOOL;
            }
            return SEMANTIC_TYPE_ERROR;
        }
        
        case AST_IDENTIFIER: {
            // 查找变量类型
            SymbolTableEntry* symbol = lookupSymbol(analyzer->current_scope, expr->data.identifier.name);
            if (symbol && symbol->symbol_type == SYMBOL_VARIABLE) {
                // 标记变量为已使用
                symbol->var_info->is_used = true;
                return symbol->var_info->type;
            }
            return SEMANTIC_TYPE_ERROR;
        }
        
        case AST_BINARY_EXPRESSION: {
            // 推导二元表达式类型
            SemanticType left_type = inferExpressionType(analyzer, expr->data.binary_expr.left);
            SemanticType right_type = inferExpressionType(analyzer, expr->data.binary_expr.right);
            return inferBinaryOperationType(expr->data.binary_expr.operator, left_type, right_type);
        }
        
        case AST_FUNCTION_CALL: {
            // 查找函数返回类型
            SymbolTableEntry* symbol = lookupSymbol(analyzer->current_scope, expr->data.func_call.name);
            if (symbol && symbol->symbol_type == SYMBOL_FUNCTION) {
                return symbol->func_info->return_type;
            }
            return SEMANTIC_TYPE_ERROR;
        }
        
        default:
            return SEMANTIC_TYPE_ERROR;
    }
}

// ===================== 错误处理函数 =====================

void reportSemanticError(SemanticAnalyzer* analyzer, SemanticErrorType error_type, 
                        const char* message, int line_number) {
    if (!analyzer) return;
    
    printf("Semantic Error (line %d): %s\n", line_number, message);
    analyzer->error_count++;
}

void reportUndefinedVariable(SemanticAnalyzer* analyzer, const char* var_name, int line_number) {
    char message[256];
    snprintf(message, sizeof(message), "Undefined variable '%s'", var_name);
    reportSemanticError(analyzer, SEMANTIC_ERROR_UNDEFINED_VARIABLE, message, line_number);
}

void reportUndefinedFunction(SemanticAnalyzer* analyzer, const char* func_name, int line_number) {
    char message[256];
    snprintf(message, sizeof(message), "Undefined function '%s'", func_name);
    reportSemanticError(analyzer, SEMANTIC_ERROR_UNDEFINED_FUNCTION, message, line_number);
}

void reportRedeclaredSymbol(SemanticAnalyzer* analyzer, const char* symbol_name, int line_number) {
    char message[256];
    snprintf(message, sizeof(message), "Symbol '%s' already declared in this scope", symbol_name);
    reportSemanticError(analyzer, SEMANTIC_ERROR_REDECLARED_VARIABLE, message, line_number);
}

void reportTypeMismatch(SemanticAnalyzer* analyzer, SemanticType expected, 
                       SemanticType actual, int line_number) {
    char message[256];
    snprintf(message, sizeof(message), "Type mismatch: expected '%s', got '%s'", 
             semanticTypeToString(expected), semanticTypeToString(actual));
    reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, message, line_number);
}

void reportFunctionCallMismatch(SemanticAnalyzer* analyzer, const char* func_name, 
                               int expected_params, int actual_params, int line_number) {
    char message[256];
    snprintf(message, sizeof(message), "Function '%s' expects %d parameters, got %d", 
             func_name, expected_params, actual_params);
    reportSemanticError(analyzer, SEMANTIC_ERROR_FUNCTION_CALL_MISMATCH, message, line_number);
}

// ===================== 调试和实用函数 =====================

void printSymbolTable(Scope* scope, int indent_level) {
    if (!scope) return;
    
    for (int i = 0; i < indent_level; i++) {
        printf("  ");
    }
    printf("Scope Level %d:\n", scope->scope_level);
    
    SymbolTableEntry* symbol = scope->symbols;
    while (symbol) {
        for (int i = 0; i < indent_level + 1; i++) {
            printf("  ");
        }
        
        if (symbol->symbol_type == SYMBOL_VARIABLE) {
            printf("Variable: %s, Type: %s, Initialized: %s, Used: %s (line %d)\n",
                   symbol->var_info->name,
                   semanticTypeToString(symbol->var_info->type),
                   symbol->var_info->is_initialized ? "yes" : "no",
                   symbol->var_info->is_used ? "yes" : "no",
                   symbol->var_info->line_number);
        } else if (symbol->symbol_type == SYMBOL_FUNCTION) {
            printf("Function: %s, Return Type: %s, Parameters: %d, Defined: %s (line %d)\n",
                   symbol->func_info->name,
                   semanticTypeToString(symbol->func_info->return_type),
                   symbol->func_info->parameter_count,
                   symbol->func_info->is_defined ? "yes" : "no",
                   symbol->func_info->line_number);
        }
        
        symbol = symbol->next;
    }
    
    // 打印子作用域
    Scope* child = scope->children;
    while (child) {
        printSymbolTable(child, indent_level + 1);
        child = child->next_sibling;
    }
}

void printSemanticAnalysisResults(SemanticAnalyzer* analyzer) {
    if (!analyzer) return;
    
    printf("\n=== Semantic Analysis Results ===\n");
    printf("Total errors: %d\n", analyzer->error_count);
    printf("Has main function: %s\n", analyzer->has_main_function ? "yes" : "no");
    
    if (analyzer->error_count == 0) {
        printf("Semantic analysis completed successfully!\n");
    } else {
        printf("Semantic analysis failed with %d errors.\n", analyzer->error_count);
    }
}

void dumpSymbolTables(SemanticAnalyzer* analyzer) {
    if (!analyzer) return;
    
    printf("\n=== Symbol Tables ===\n");
    printSymbolTable(analyzer->global_scope, 0);
}

bool isMainFunction(const char* func_name) {
    return func_name && strcmp(func_name, "main") == 0;
}

int getLineNumberFromNode(ASTNode* node) {
    // 简化实现，实际应该从AST节点中获取行号信息
    // 这里返回默认值，实际实现需要在AST构建时保存行号信息
    return 1;
}

void markVariableAsUsed(SemanticAnalyzer* analyzer, const char* var_name) {
    if (!analyzer || !var_name) return;
    
    SymbolTableEntry* symbol = lookupSymbol(analyzer->current_scope, var_name);
    if (symbol && symbol->symbol_type == SYMBOL_VARIABLE) {
        symbol->var_info->is_used = true;
    }
}

void checkUnusedVariables(SemanticAnalyzer* analyzer) {
    if (!analyzer) return;
    
    // 递归检查所有作用域中的未使用变量
    // 这里是简化实现，仅检查全局作用域
    SymbolTableEntry* symbol = analyzer->global_scope->symbols;
    while (symbol) {
        if (symbol->symbol_type == SYMBOL_VARIABLE && !symbol->var_info->is_used) {
            char message[256];
            snprintf(message, sizeof(message), "Variable '%s' declared but never used", 
                     symbol->var_info->name);
            printf("Warning (line %d): %s\n", symbol->var_info->line_number, message);
        }
        symbol = symbol->next;
    }
}

// ===================== 语义分析主要函数 =====================

bool performSemanticAnalysis(ASTNode* ast) {
    if (!ast) {
        printf("Error: No AST provided for semantic analysis\n");
        return false;
    }
    
    SemanticAnalyzer* analyzer = createSemanticAnalyzer();
    if (!analyzer) {
        printf("Error: Failed to create semantic analyzer\n");
        return false;
    }
    
    printf("=== Semantic Analysis ===\n");
    
    bool success = analyzeNode(analyzer, ast);
    
    // 检查是否有main函数
    if (success && !analyzer->has_main_function) {
        reportSemanticError(analyzer, SEMANTIC_ERROR_NO_MAIN_FUNCTION, 
                           "Program must have a main function", 1);
        success = false;
    }
    
    // 检查未使用的变量
    checkUnusedVariables(analyzer);
    
    // 打印结果
    printSemanticAnalysisResults(analyzer);
    
    if (analyzer->error_count == 0) {
        printf("Symbol tables:\n");
        dumpSymbolTables(analyzer);
    }
    
    bool result = (analyzer->error_count == 0);
    destroySemanticAnalyzer(analyzer);
    
    return result;
}

bool analyzeNode(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (!analyzer || !node) return false;
    
    switch (node->type) {
        case AST_PROGRAM:
            return analyzeProgram(analyzer, node);
        case AST_VARIABLE_DECLARATION:
            return analyzeVariableDeclaration(analyzer, node);
        case AST_FUNCTION_DECLARATION:
            return analyzeFunctionDeclaration(analyzer, node);
        case AST_FUNCTION_DEFINITION:
            return analyzeFunctionDefinition(analyzer, node);
        case AST_ASSIGNMENT_STATEMENT:
            return analyzeAssignment(analyzer, node);
        case AST_IF_STATEMENT:
            return analyzeIfStatement(analyzer, node);
        case AST_WHILE_STATEMENT:
            return analyzeWhileStatement(analyzer, node);
        case AST_RETURN_STATEMENT:
            return analyzeReturnStatement(analyzer, node);
        case AST_BLOCK_STATEMENT:
            return analyzeBlock(analyzer, node);
        case AST_EXPRESSION_STATEMENT:
            return analyzeExpression(analyzer, node->data.expr_stmt.expression);
        case AST_FUNCTION_CALL:
            return analyzeFunctionCall(analyzer, node);
        default:
            return analyzeExpression(analyzer, node);
    }
}

bool analyzeProgram(SemanticAnalyzer* analyzer, ASTNode* program) {
    if (!analyzer || !program || program->type != AST_PROGRAM) return false;
    
    bool success = true;
    
    // 分析所有顶层声明
    ASTNode* declaration = program->data.program.declarations;
    while (declaration) {
        if (!analyzeNode(analyzer, declaration)) {
            success = false;
        }
        declaration = declaration->next;
    }
    
    return success;
}

bool analyzeVariableDeclaration(SemanticAnalyzer* analyzer, ASTNode* var_decl) {
    if (!analyzer || !var_decl || var_decl->type != AST_VARIABLE_DECLARATION) return false;
    
    const char* var_name = var_decl->data.var_decl.name;
    const char* type_str = var_decl->data.var_decl.type;
    SemanticType var_type = stringToSemanticType(type_str);
    
    if (var_type == SEMANTIC_TYPE_ERROR) {
        reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, 
                           "Invalid variable type", getLineNumberFromNode(var_decl));
        return false;
    }
    
    // 检查是否在当前作用域中重复声明
    if (lookupSymbolInCurrentScope(analyzer->current_scope, var_name)) {
        reportRedeclaredSymbol(analyzer, var_name, getLineNumberFromNode(var_decl));
        return false;
    }
    
    // 创建变量符号
    SymbolTableEntry* var_symbol = createVariableSymbol(var_name, var_type, 
                                                        getLineNumberFromNode(var_decl));
    if (!var_symbol) {
        reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, 
                           "Failed to create variable symbol", getLineNumberFromNode(var_decl));
        return false;
    }
    
    // 检查初始化表达式
    if (var_decl->data.var_decl.initializer) {
        SemanticType init_type = inferExpressionType(analyzer, var_decl->data.var_decl.initializer);
        if (init_type == SEMANTIC_TYPE_ERROR) {
            reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, 
                               "Invalid initializer expression", getLineNumberFromNode(var_decl));
            free(var_symbol);
            return false;
        }
        
        if (!canImplicitlyCast(init_type, var_type)) {
            reportTypeMismatch(analyzer, var_type, init_type, getLineNumberFromNode(var_decl));
            free(var_symbol);
            return false;
        }
        
        var_symbol->var_info->is_initialized = true;
    }
    
    // 添加到符号表
    if (!addSymbol(analyzer->current_scope, var_symbol)) {
        reportRedeclaredSymbol(analyzer, var_name, getLineNumberFromNode(var_decl));
        free(var_symbol);
        return false;
    }
    
    return true;
}

bool analyzeFunctionDeclaration(SemanticAnalyzer* analyzer, ASTNode* func_decl) {
    if (!analyzer || !func_decl || func_decl->type != AST_FUNCTION_DECLARATION) return false;
    
    const char* func_name = func_decl->data.func_decl.name;
    const char* return_type_str = func_decl->data.func_decl.return_type;
    SemanticType return_type = stringToSemanticType(return_type_str);
    
    if (return_type == SEMANTIC_TYPE_ERROR) {
        reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, 
                           "Invalid return type", getLineNumberFromNode(func_decl));
        return false;
    }
    
    // 检查是否为main函数
    if (isMainFunction(func_name)) {
        analyzer->has_main_function = true;
        if (return_type != SEMANTIC_TYPE_INT && return_type != SEMANTIC_TYPE_VOID) {
            reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, 
                               "main function must return int or void", getLineNumberFromNode(func_decl));
            return false;
        }
    }
    
    // 解析参数列表
    FunctionParameter* params = NULL;
    int param_count = 0;
    
    if (func_decl->data.func_decl.parameters) {
        // 遍历参数列表
        ASTNode* param_node = func_decl->data.func_decl.parameters->data.list.items;
        while (param_node) {
            if (param_node->type == AST_VARIABLE_DECLARATION) {
                const char* param_name = param_node->data.var_decl.name;
                const char* param_type_str = param_node->data.var_decl.type;
                SemanticType param_type = stringToSemanticType(param_type_str);
                
                if (param_type == SEMANTIC_TYPE_ERROR) {
                    reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, 
                                       "Invalid parameter type", getLineNumberFromNode(param_node));
                    destroyFunctionParameters(params);
                    return false;
                }
                
                FunctionParameter* param = createFunctionParameter(param_name, param_type);
                if (!param) {
                    destroyFunctionParameters(params);
                    return false;
                }
                
                // 添加到参数列表
                if (!params) {
                    params = param;
                } else {
                    FunctionParameter* current = params;
                    while (current->next) {
                        current = current->next;
                    }
                    current->next = param;
                }
                param_count++;
            }
            param_node = param_node->next;
        }
    }
    
    // 检查函数是否已经声明
    SymbolTableEntry* existing = lookupSymbolInCurrentScope(analyzer->current_scope, func_name);
    if (existing) {
        if (existing->symbol_type != SYMBOL_FUNCTION) {
            reportRedeclaredSymbol(analyzer, func_name, getLineNumberFromNode(func_decl));
            destroyFunctionParameters(params);
            return false;
        }
        
        // 检查函数签名是否匹配
        FunctionInfo* existing_func = existing->func_info;
        if (existing_func->return_type != return_type || 
            existing_func->parameter_count != param_count) {
            reportSemanticError(analyzer, SEMANTIC_ERROR_REDECLARED_FUNCTION, 
                               "Function signature mismatch", getLineNumberFromNode(func_decl));
            destroyFunctionParameters(params);
            return false;
        }
        
        // 检查参数类型
        FunctionParameter* existing_param = existing_func->parameters;
        FunctionParameter* new_param = params;
        while (existing_param && new_param) {
            if (existing_param->type != new_param->type) {
                reportSemanticError(analyzer, SEMANTIC_ERROR_REDECLARED_FUNCTION, 
                                   "Parameter type mismatch", getLineNumberFromNode(func_decl));
                destroyFunctionParameters(params);
                return false;
            }
            existing_param = existing_param->next;
            new_param = new_param->next;
        }
        
        destroyFunctionParameters(params);
        return true; // 重复声明但签名相同，允许
    }
    
    // 创建函数符号
    SymbolTableEntry* func_symbol = createFunctionSymbol(func_name, return_type, params, param_count, 
                                                         getLineNumberFromNode(func_decl));
    if (!func_symbol) {
        destroyFunctionParameters(params);
        return false;
    }
    
    // 添加到符号表
    if (!addSymbol(analyzer->current_scope, func_symbol)) {
        reportRedeclaredSymbol(analyzer, func_name, getLineNumberFromNode(func_decl));
        return false;
    }
    
    return true;
}

bool analyzeFunctionDefinition(SemanticAnalyzer* analyzer, ASTNode* func_def) {
    if (!analyzer || !func_def || func_def->type != AST_FUNCTION_DEFINITION) return false;
    
    // 先分析函数声明部分
    const char* func_name = func_def->data.func_def.name;
    const char* return_type_str = func_def->data.func_def.return_type;
    SemanticType return_type = stringToSemanticType(return_type_str);
    
    if (return_type == SEMANTIC_TYPE_ERROR) {
        reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, 
                           "Invalid return type", getLineNumberFromNode(func_def));
        return false;
    }
    
    // 检查是否为main函数
    if (isMainFunction(func_name)) {
        analyzer->has_main_function = true;
        if (return_type != SEMANTIC_TYPE_INT && return_type != SEMANTIC_TYPE_VOID) {
            reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, 
                               "main function must return int or void", getLineNumberFromNode(func_def));
            return false;
        }
    }
    
    // 查找函数声明
    SymbolTableEntry* func_symbol = lookupSymbol(analyzer->current_scope, func_name);
    FunctionInfo* func_info = NULL;
    
    if (func_symbol && func_symbol->symbol_type == SYMBOL_FUNCTION) {
        func_info = func_symbol->func_info;
        if (func_info->is_defined) {
            reportSemanticError(analyzer, SEMANTIC_ERROR_REDECLARED_FUNCTION, 
                               "Function already defined", getLineNumberFromNode(func_def));
            return false;
        }
        func_info->is_defined = true;
    } else {
        // 创建新的函数符号（如果没有预先声明）
        FunctionParameter* params = NULL;
        int param_count = 0;
        
        // 解析参数列表（简化处理）
        func_symbol = createFunctionSymbol(func_name, return_type, params, param_count, 
                                          getLineNumberFromNode(func_def));
        if (!addSymbol(analyzer->current_scope, func_symbol)) {
            return false;
        }
        func_info = func_symbol->func_info;
        func_info->is_defined = true;
    }
    
    // 设置当前分析的函数
    FunctionInfo* prev_function = analyzer->current_function;
    analyzer->current_function = func_info;
    
    // 进入函数作用域
    enterScope(analyzer);
    
    // 将参数添加到函数作用域
    if (func_def->data.func_def.parameters) {
        ASTNode* param_node = func_def->data.func_def.parameters->data.list.items;
        while (param_node) {
            if (param_node->type == AST_VARIABLE_DECLARATION) {
                if (!analyzeVariableDeclaration(analyzer, param_node)) {
                    exitScope(analyzer);
                    analyzer->current_function = prev_function;
                    return false;
                }
            }
            param_node = param_node->next;
        }
    }
    
    // 分析函数体
    bool success = analyzeBlock(analyzer, func_def->data.func_def.body);
    
    // 退出函数作用域
    exitScope(analyzer);
    analyzer->current_function = prev_function;
    
    return success;
}

bool analyzeAssignment(SemanticAnalyzer* analyzer, ASTNode* assignment) {
    if (!analyzer || !assignment || assignment->type != AST_ASSIGNMENT_STATEMENT) return false;
    
    const char* var_name = assignment->data.assignment.variable;
    
    // 查找变量
    SymbolTableEntry* var_symbol = lookupSymbol(analyzer->current_scope, var_name);
    if (!var_symbol || var_symbol->symbol_type != SYMBOL_VARIABLE) {
        reportUndefinedVariable(analyzer, var_name, getLineNumberFromNode(assignment));
        return false;
    }
    
    // 分析赋值表达式
    SemanticType value_type = inferExpressionType(analyzer, assignment->data.assignment.value);
    if (value_type == SEMANTIC_TYPE_ERROR) {
        reportSemanticError(analyzer, SEMANTIC_ERROR_INVALID_ASSIGNMENT, 
                           "Invalid assignment expression", getLineNumberFromNode(assignment));
        return false;
    }
    
    // 检查类型兼容性
    SemanticType var_type = var_symbol->var_info->type;
    if (!canImplicitlyCast(value_type, var_type)) {
        reportTypeMismatch(analyzer, var_type, value_type, getLineNumberFromNode(assignment));
        return false;
    }
    
    // 标记变量为已初始化和已使用
    var_symbol->var_info->is_initialized = true;
    var_symbol->var_info->is_used = true;
    
    return true;
}

bool analyzeFunctionCall(SemanticAnalyzer* analyzer, ASTNode* func_call) {
    if (!analyzer || !func_call || func_call->type != AST_FUNCTION_CALL) return false;
    
    const char* func_name = func_call->data.func_call.name;
    
    // 查找函数
    SymbolTableEntry* func_symbol = lookupSymbol(analyzer->current_scope, func_name);
    if (!func_symbol || func_symbol->symbol_type != SYMBOL_FUNCTION) {
        reportUndefinedFunction(analyzer, func_name, getLineNumberFromNode(func_call));
        return false;
    }
    
    FunctionInfo* func_info = func_symbol->func_info;
    
    // 检查参数数量
    int actual_param_count = 0;
    if (func_call->data.func_call.arguments) {
        ASTNode* arg = func_call->data.func_call.arguments->data.list.items;
        while (arg) {
            actual_param_count++;
            arg = arg->next;
        }
    }
    
    if (actual_param_count != func_info->parameter_count) {
        reportFunctionCallMismatch(analyzer, func_name, func_info->parameter_count, 
                                  actual_param_count, getLineNumberFromNode(func_call));
        return false;
    }
    
    // 检查参数类型
    if (func_call->data.func_call.arguments) {
        ASTNode* arg = func_call->data.func_call.arguments->data.list.items;
        FunctionParameter* param = func_info->parameters;
        
        while (arg && param) {
            SemanticType arg_type = inferExpressionType(analyzer, arg);
            if (arg_type == SEMANTIC_TYPE_ERROR) {
                reportSemanticError(analyzer, SEMANTIC_ERROR_FUNCTION_CALL_MISMATCH, 
                                   "Invalid argument expression", getLineNumberFromNode(func_call));
                return false;
            }
            
            if (!canImplicitlyCast(arg_type, param->type)) {
                char message[256];
                snprintf(message, sizeof(message), 
                         "Argument type mismatch for parameter '%s': expected '%s', got '%s'",
                         param->name, semanticTypeToString(param->type), semanticTypeToString(arg_type));
                reportSemanticError(analyzer, SEMANTIC_ERROR_FUNCTION_CALL_MISMATCH, 
                                   message, getLineNumberFromNode(func_call));
                return false;
            }
            
            arg = arg->next;
            param = param->next;
        }
    }
    
    return true;
}

bool analyzeIfStatement(SemanticAnalyzer* analyzer, ASTNode* if_stmt) {
    if (!analyzer || !if_stmt || if_stmt->type != AST_IF_STATEMENT) return false;
    
    // 分析条件表达式
    SemanticType condition_type = inferExpressionType(analyzer, if_stmt->data.if_stmt.condition);
    if (condition_type == SEMANTIC_TYPE_ERROR) {
        reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, 
                           "Invalid condition expression", getLineNumberFromNode(if_stmt));
        return false;
    }
    
    // 条件表达式应该是bool类型或可以转换为bool的类型
    if (condition_type != SEMANTIC_TYPE_BOOL && !isNumericType(condition_type)) {
        reportTypeMismatch(analyzer, SEMANTIC_TYPE_BOOL, condition_type, getLineNumberFromNode(if_stmt));
        return false;
    }
    
    // 分析then语句块
    bool success = analyzeStatement(analyzer, if_stmt->data.if_stmt.then_stmt);
    
    // 分析else语句块（如果存在）
    if (if_stmt->data.if_stmt.else_stmt) {
        success &= analyzeStatement(analyzer, if_stmt->data.if_stmt.else_stmt);
    }
    
    return success;
}

bool analyzeWhileStatement(SemanticAnalyzer* analyzer, ASTNode* while_stmt) {
    if (!analyzer || !while_stmt || while_stmt->type != AST_WHILE_STATEMENT) return false;
    
    // 分析条件表达式
    SemanticType condition_type = inferExpressionType(analyzer, while_stmt->data.while_stmt.condition);
    if (condition_type == SEMANTIC_TYPE_ERROR) {
        reportSemanticError(analyzer, SEMANTIC_ERROR_TYPE_MISMATCH, 
                           "Invalid condition expression", getLineNumberFromNode(while_stmt));
        return false;
    }
    
    // 条件表达式应该是bool类型或可以转换为bool的类型
    if (condition_type != SEMANTIC_TYPE_BOOL && !isNumericType(condition_type)) {
        reportTypeMismatch(analyzer, SEMANTIC_TYPE_BOOL, condition_type, getLineNumberFromNode(while_stmt));
        return false;
    }
    
    // 分析循环体
    return analyzeStatement(analyzer, while_stmt->data.while_stmt.body);
}

bool analyzeReturnStatement(SemanticAnalyzer* analyzer, ASTNode* return_stmt) {
    if (!analyzer || !return_stmt || return_stmt->type != AST_RETURN_STATEMENT) return false;
    
    // 检查是否在函数内
    if (!analyzer->current_function) {
        reportSemanticError(analyzer, SEMANTIC_ERROR_RETURN_TYPE_MISMATCH, 
                           "Return statement outside function", getLineNumberFromNode(return_stmt));
        return false;
    }
    
    SemanticType expected_return_type = analyzer->current_function->return_type;
    
    if (return_stmt->data.return_stmt.value) {
        // 有返回值的return语句
        SemanticType actual_return_type = inferExpressionType(analyzer, return_stmt->data.return_stmt.value);
        if (actual_return_type == SEMANTIC_TYPE_ERROR) {
            reportSemanticError(analyzer, SEMANTIC_ERROR_RETURN_TYPE_MISMATCH, 
                               "Invalid return expression", getLineNumberFromNode(return_stmt));
            return false;
        }
        
        if (expected_return_type == SEMANTIC_TYPE_VOID) {
            reportSemanticError(analyzer, SEMANTIC_ERROR_RETURN_TYPE_MISMATCH, 
                               "void function cannot return a value", getLineNumberFromNode(return_stmt));
            return false;
        }
        
        if (!canImplicitlyCast(actual_return_type, expected_return_type)) {
            reportTypeMismatch(analyzer, expected_return_type, actual_return_type, 
                              getLineNumberFromNode(return_stmt));
            return false;
        }
    } else {
        // 无返回值的return语句
        if (expected_return_type != SEMANTIC_TYPE_VOID) {
            reportSemanticError(analyzer, SEMANTIC_ERROR_RETURN_TYPE_MISMATCH, 
                               "non-void function must return a value", getLineNumberFromNode(return_stmt));
            return false;
        }
    }
    
    return true;
}

bool analyzeBlock(SemanticAnalyzer* analyzer, ASTNode* block) {
    if (!analyzer || !block || block->type != AST_BLOCK_STATEMENT) return false;
    
    // 进入新的作用域
    enterScope(analyzer);
    
    bool success = true;
    
    // 分析块中的所有语句
    ASTNode* statement = block->data.block.statements;
    while (statement) {
        if (!analyzeStatement(analyzer, statement)) {
            success = false;
        }
        statement = statement->next;
    }
    
    // 退出作用域
    exitScope(analyzer);
    
    return success;
}

bool analyzeStatement(SemanticAnalyzer* analyzer, ASTNode* statement) {
    if (!analyzer || !statement) return false;
    
    switch (statement->type) {
        case AST_VARIABLE_DECLARATION:
            return analyzeVariableDeclaration(analyzer, statement);
        case AST_ASSIGNMENT_STATEMENT:
            return analyzeAssignment(analyzer, statement);
        case AST_IF_STATEMENT:
            return analyzeIfStatement(analyzer, statement);
        case AST_WHILE_STATEMENT:
            return analyzeWhileStatement(analyzer, statement);
        case AST_RETURN_STATEMENT:
            return analyzeReturnStatement(analyzer, statement);
        case AST_BLOCK_STATEMENT:
            return analyzeBlock(analyzer, statement);
        case AST_EXPRESSION_STATEMENT:
            return analyzeExpression(analyzer, statement->data.expr_stmt.expression);
        default:
            return analyzeExpression(analyzer, statement);
    }
}

bool analyzeExpression(SemanticAnalyzer* analyzer, ASTNode* expression) {
    if (!analyzer || !expression) return false;
    
    SemanticType expr_type = inferExpressionType(analyzer, expression);
    return expr_type != SEMANTIC_TYPE_ERROR;
}