/**
 * semantic.c - 语义分析器实现
 *
 * 这是编译器的第三个阶段！
 *
 * 语义分析的核心工作：
 * 1. 遍历 AST，收集声明信息到符号表
 * 2. 检查每个标识符是否已声明
 * 3. 检查类型是否兼容
 * 4. 报告语义错误
 */

#include "../include/semantic.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


// ========== 辅助函数 ==========

/**
 * 字符串复制
 */
static char *str_dup(const char *s) {
  if (!s)
    return NULL;
  size_t len = strlen(s) + 1;
  char *copy = (char *)malloc(len);
  if (copy)
    memcpy(copy, s, len);
  return copy;
}

/**
 * 简单的字符串哈希函数
 */
static unsigned int hash(const char *str) {
  unsigned int h = 0;
  while (*str) {
    h = h * 31 + (unsigned char)*str++;
  }
  return h % SYMBOL_TABLE_SIZE;
}

// ========== 类型转换 ==========

/**
 * 字符串转数据类型
 */
DataType string_to_datatype(const char *type_str) {
  if (!type_str)
    return TYPE_UNKNOWN;
  if (strcmp(type_str, "int") == 0)
    return TYPE_INT;
  if (strcmp(type_str, "float") == 0)
    return TYPE_FLOAT;
  if (strcmp(type_str, "char") == 0)
    return TYPE_CHAR;
  if (strcmp(type_str, "void") == 0)
    return TYPE_VOID;
  if (strcmp(type_str, "bool") == 0)
    return TYPE_BOOL;
  if (strcmp(type_str, "string") == 0)
    return TYPE_STRING;
  return TYPE_UNKNOWN;
}

/**
 * 数据类型转字符串
 */
const char *datatype_to_string(DataType type) {
  switch (type) {
  case TYPE_VOID:
    return "void";
  case TYPE_INT:
    return "int";
  case TYPE_FLOAT:
    return "float";
  case TYPE_CHAR:
    return "char";
  case TYPE_STRING:
    return "string";
  case TYPE_BOOL:
    return "bool";
  case TYPE_UNKNOWN:
    return "unknown";
  case TYPE_ERROR:
    return "error";
  default:
    return "???";
  }
}

// ========== 符号操作 ==========

/**
 * 创建符号
 */
static Symbol *create_symbol(const char *name, SymbolKind kind, DataType type) {
  Symbol *sym = (Symbol *)calloc(1, sizeof(Symbol));
  if (sym) {
    sym->name = str_dup(name);
    sym->kind = kind;
    sym->data_type = type;
    sym->return_type = TYPE_VOID;
    sym->params = NULL;
    sym->param_count = 0;
    sym->next = NULL;
  }
  return sym;
}

/**
 * 释放符号
 */
static void free_symbol(Symbol *sym) {
  if (!sym)
    return;
  free(sym->name);
  for (int i = 0; i < sym->param_count; i++) {
    free(sym->params[i].name);
  }
  free(sym->params);
  free(sym);
}

// ========== 作用域操作 ==========

/**
 * 创建作用域
 */
static Scope *create_scope(Scope *parent) {
  Scope *scope = (Scope *)calloc(1, sizeof(Scope));
  if (scope) {
    scope->parent = parent;
    scope->level = parent ? parent->level + 1 : 0;
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
      scope->symbols[i] = NULL;
    }
  }
  return scope;
}

/**
 * 释放作用域（不递归释放父作用域）
 */
static void free_scope(Scope *scope) {
  if (!scope)
    return;
  for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
    Symbol *sym = scope->symbols[i];
    while (sym) {
      Symbol *next = sym->next;
      free_symbol(sym);
      sym = next;
    }
  }
  free(scope);
}

/**
 * 在作用域中添加符号
 */
static int scope_add_symbol(Scope *scope, Symbol *sym) {
  unsigned int h = hash(sym->name);

  // 检查是否已存在
  Symbol *existing = scope->symbols[h];
  while (existing) {
    if (strcmp(existing->name, sym->name) == 0) {
      return 0; // 已存在，添加失败
    }
    existing = existing->next;
  }

  // 添加到链表头部
  sym->next = scope->symbols[h];
  scope->symbols[h] = sym;
  return 1; // 成功
}

/**
 * 在作用域中查找符号（不查父作用域）
 */
static Symbol *scope_lookup(Scope *scope, const char *name) {
  unsigned int h = hash(name);
  Symbol *sym = scope->symbols[h];
  while (sym) {
    if (strcmp(sym->name, name) == 0) {
      return sym;
    }
    sym = sym->next;
  }
  return NULL;
}

// ========== 语义分析器操作 ==========

/**
 * 初始化语义分析器
 */
SemanticAnalyzer *semantic_init(void) {
  SemanticAnalyzer *analyzer =
      (SemanticAnalyzer *)calloc(1, sizeof(SemanticAnalyzer));
  if (analyzer) {
    analyzer->global_scope = create_scope(NULL);
    analyzer->current_scope = analyzer->global_scope;
    analyzer->errors = NULL;
    analyzer->error_count = 0;
    analyzer->current_function = NULL;
  }
  return analyzer;
}

/**
 * 释放语义分析器
 */
void semantic_free(SemanticAnalyzer *analyzer) {
  if (!analyzer)
    return;

  // 释放所有作用域
  Scope *scope = analyzer->current_scope;
  while (scope) {
    Scope *parent = scope->parent;
    free_scope(scope);
    scope = parent;
  }

  // 释放错误链表
  SemanticError *err = analyzer->errors;
  while (err) {
    SemanticError *next = err->next;
    free(err);
    err = next;
  }

  free(analyzer);
}

/**
 * 进入新作用域
 */
void semantic_enter_scope(SemanticAnalyzer *analyzer) {
  Scope *new_scope = create_scope(analyzer->current_scope);
  analyzer->current_scope = new_scope;
}

/**
 * 退出当前作用域
 */
void semantic_exit_scope(SemanticAnalyzer *analyzer) {
  if (analyzer->current_scope == analyzer->global_scope) {
    return; // 不能退出全局作用域
  }

  Scope *old_scope = analyzer->current_scope;
  analyzer->current_scope = old_scope->parent;
  free_scope(old_scope);
}

/**
 * 声明符号
 */
Symbol *semantic_declare(SemanticAnalyzer *analyzer, const char *name,
                         SymbolKind kind, DataType type) {
  // 检查当前作用域是否已存在
  if (scope_lookup(analyzer->current_scope, name)) {
    return NULL; // 重复声明
  }

  Symbol *sym = create_symbol(name, kind, type);
  if (sym) {
    sym->data_type = type;
    scope_add_symbol(analyzer->current_scope, sym);
  }
  return sym;
}

/**
 * 查找符号（从当前作用域向上查找）
 */
Symbol *semantic_lookup(SemanticAnalyzer *analyzer, const char *name) {
  Scope *scope = analyzer->current_scope;
  while (scope) {
    Symbol *sym = scope_lookup(scope, name);
    if (sym)
      return sym;
    scope = scope->parent;
  }
  return NULL; // 未找到
}

/**
 * 只在当前作用域查找
 */
Symbol *semantic_lookup_current_scope(SemanticAnalyzer *analyzer,
                                      const char *name) {
  return scope_lookup(analyzer->current_scope, name);
}

// ========== 类型检查 ==========

/**
 * 检查类型是否兼容
 */
int semantic_types_compatible(DataType expected, DataType actual) {
  if (expected == actual)
    return 1;
  if (expected == TYPE_ERROR || actual == TYPE_ERROR)
    return 1; // 不重复报错

  // 数值类型之间可以兼容
  if ((expected == TYPE_INT || expected == TYPE_FLOAT) &&
      (actual == TYPE_INT || actual == TYPE_FLOAT)) {
    return 1;
  }

  // char 可以转换为 int
  if (expected == TYPE_INT && actual == TYPE_CHAR)
    return 1;
  if (expected == TYPE_CHAR && actual == TYPE_INT)
    return 1;

  return 0;
}

/**
 * 检查二元表达式的类型
 */
DataType semantic_check_binary_expr(SemanticAnalyzer *analyzer, BinaryOp op,
                                    DataType left, DataType right) {
  (void)analyzer; // 暂时不用

  // 如果任一操作数类型错误，传播错误
  if (left == TYPE_ERROR || right == TYPE_ERROR) {
    return TYPE_ERROR;
  }

  switch (op) {
  // 算术运算符：返回更"大"的类型
  case OP_ADD:
  case OP_SUB:
  case OP_MUL:
  case OP_DIV:
  case OP_MOD:
    if (left == TYPE_FLOAT || right == TYPE_FLOAT) {
      return TYPE_FLOAT;
    }
    if (left == TYPE_INT || right == TYPE_INT) {
      return TYPE_INT;
    }
    return TYPE_ERROR;

  // 比较运算符：返回 bool/int
  case OP_EQ:
  case OP_NE:
  case OP_LT:
  case OP_GT:
  case OP_LE:
  case OP_GE:
    return TYPE_INT; // 我们用 int 表示 bool

  // 逻辑运算符
  case OP_AND:
  case OP_OR:
    return TYPE_INT;

  default:
    return TYPE_ERROR;
  }
}

// ========== 错误处理 ==========

/**
 * 报告语义错误
 */
void semantic_error(SemanticAnalyzer *analyzer, SemanticErrorType type,
                    int line, const char *format, ...) {
  SemanticError *err = (SemanticError *)calloc(1, sizeof(SemanticError));
  if (!err)
    return;

  err->type = type;
  err->line = line;

  va_list args;
  va_start(args, format);
  vsnprintf(err->message, sizeof(err->message), format, args);
  va_end(args);

  // 添加到错误链表末尾
  err->next = NULL;
  if (!analyzer->errors) {
    analyzer->errors = err;
  } else {
    SemanticError *last = analyzer->errors;
    while (last->next)
      last = last->next;
    last->next = err;
  }

  analyzer->error_count++;
}

/**
 * 检查是否有错误
 */
int semantic_has_errors(SemanticAnalyzer *analyzer) {
  return analyzer->error_count > 0;
}

/**
 * 打印所有错误
 */
void semantic_print_errors(SemanticAnalyzer *analyzer) {
  SemanticError *err = analyzer->errors;
  while (err) {
    fprintf(stderr, "[Line %d] Semantic Error: %s\n", err->line, err->message);
    err = err->next;
  }
  if (analyzer->error_count > 0) {
    fprintf(stderr, "Total: %d semantic error(s)\n", analyzer->error_count);
  }
}

// ========== AST 遍历和分析 ==========

// 前向声明
static DataType analyze_expression(SemanticAnalyzer *analyzer, ASTNode *node);
static void analyze_statement(SemanticAnalyzer *analyzer, ASTNode *node);
static void analyze_declaration(SemanticAnalyzer *analyzer, ASTNode *node);

/**
 * 分析表达式，返回表达式的类型
 */
static DataType analyze_expression(SemanticAnalyzer *analyzer, ASTNode *node) {
  if (!node)
    return TYPE_ERROR;

  switch (node->type) {
  case AST_INT_LITERAL:
    return TYPE_INT;

  case AST_FLOAT_LITERAL:
    return TYPE_FLOAT;

  case AST_CHAR_LITERAL:
    return TYPE_CHAR;

  case AST_STRING_LITERAL:
    return TYPE_STRING;

  case AST_IDENTIFIER: {
    // 查找变量
    Symbol *sym = semantic_lookup(analyzer, node->data.identifier.name);
    if (!sym) {
      semantic_error(analyzer, SEM_ERROR_UNDECLARED_VAR, node->line,
                     "Undeclared variable '%s'", node->data.identifier.name);
      return TYPE_ERROR;
    }
    return sym->data_type;
  }

  case AST_BINARY_EXPR: {
    DataType left = analyze_expression(analyzer, node->data.binary_expr.left);
    DataType right = analyze_expression(analyzer, node->data.binary_expr.right);
    return semantic_check_binary_expr(analyzer, node->data.binary_expr.op, left,
                                      right);
  }

  case AST_UNARY_EXPR: {
    DataType operand =
        analyze_expression(analyzer, node->data.unary_expr.operand);
    if (node->data.unary_expr.op == OP_NOT) {
      return TYPE_INT; // 逻辑非返回 int
    }
    return operand; // 负号保持类型
  }

  case AST_CALL_EXPR: {
    // 查找函数
    Symbol *func = semantic_lookup(analyzer, node->data.call_expr.callee);
    if (!func) {
      semantic_error(analyzer, SEM_ERROR_UNDECLARED_FUNC, node->line,
                     "Undeclared function '%s'", node->data.call_expr.callee);
      return TYPE_ERROR;
    }
    if (func->kind != SYMBOL_FUNCTION) {
      semantic_error(analyzer, SEM_ERROR_NOT_CALLABLE, node->line,
                     "'%s' is not a function", node->data.call_expr.callee);
      return TYPE_ERROR;
    }

    // 检查参数数量
    if (node->data.call_expr.arg_count != func->param_count) {
      semantic_error(analyzer, SEM_ERROR_WRONG_ARG_COUNT, node->line,
                     "Function '%s' expects %d arguments, got %d", func->name,
                     func->param_count, node->data.call_expr.arg_count);
    }

    // 分析每个参数
    for (int i = 0; i < node->data.call_expr.arg_count; i++) {
      DataType arg_type =
          analyze_expression(analyzer, node->data.call_expr.arguments[i]);

      // 检查参数类型（如果参数数量匹配）
      if (i < func->param_count) {
        if (!semantic_types_compatible(func->params[i].type, arg_type)) {
          semantic_error(analyzer, SEM_ERROR_WRONG_ARG_TYPE, node->line,
                         "Argument %d of '%s': expected %s, got %s", i + 1,
                         func->name, datatype_to_string(func->params[i].type),
                         datatype_to_string(arg_type));
        }
      }
    }

    return func->return_type;
  }

  case AST_ASSIGN_EXPR: {
    // 查找变量
    Symbol *sym = semantic_lookup(analyzer, node->data.assign_expr.name);
    if (!sym) {
      semantic_error(analyzer, SEM_ERROR_UNDECLARED_VAR, node->line,
                     "Undeclared variable '%s'", node->data.assign_expr.name);
      return TYPE_ERROR;
    }

    DataType value_type =
        analyze_expression(analyzer, node->data.assign_expr.value);

    // 类型检查
    if (!semantic_types_compatible(sym->data_type, value_type)) {
      semantic_error(analyzer, SEM_ERROR_TYPE_MISMATCH, node->line,
                     "Cannot assign %s to %s variable '%s'",
                     datatype_to_string(value_type),
                     datatype_to_string(sym->data_type), sym->name);
    }

    return sym->data_type;
  }

  default:
    return TYPE_ERROR;
  }
}

/**
 * 分析语句
 */
static void analyze_statement(SemanticAnalyzer *analyzer, ASTNode *node) {
  if (!node)
    return;

  switch (node->type) {
  case AST_BLOCK:
    semantic_enter_scope(analyzer);
    for (int i = 0; i < node->data.block.count; i++) {
      analyze_statement(analyzer, node->data.block.statements[i]);
    }
    semantic_exit_scope(analyzer);
    break;

  case AST_VAR_DECL: {
    DataType type = string_to_datatype(node->data.var_decl.type);

    // void 类型变量不允许
    if (type == TYPE_VOID) {
      semantic_error(analyzer, SEM_ERROR_VOID_VARIABLE, node->line,
                     "Cannot declare variable '%s' with void type",
                     node->data.var_decl.name);
    }

    // 检查重复声明
    if (semantic_lookup_current_scope(analyzer, node->data.var_decl.name)) {
      semantic_error(analyzer, SEM_ERROR_REDECLARED, node->line,
                     "Variable '%s' already declared in this scope",
                     node->data.var_decl.name);
    } else {
      semantic_declare(analyzer, node->data.var_decl.name, SYMBOL_VARIABLE,
                       type);
    }

    // 分析初始化表达式
    if (node->data.var_decl.initializer) {
      DataType init_type =
          analyze_expression(analyzer, node->data.var_decl.initializer);
      if (!semantic_types_compatible(type, init_type)) {
        semantic_error(analyzer, SEM_ERROR_TYPE_MISMATCH, node->line,
                       "Cannot initialize %s variable with %s value",
                       datatype_to_string(type), datatype_to_string(init_type));
      }
    }
    break;
  }

  case AST_IF_STMT:
    analyze_expression(analyzer, node->data.if_stmt.condition);
    analyze_statement(analyzer, node->data.if_stmt.then_branch);
    if (node->data.if_stmt.else_branch) {
      analyze_statement(analyzer, node->data.if_stmt.else_branch);
    }
    break;

  case AST_WHILE_STMT:
    analyze_expression(analyzer, node->data.while_stmt.condition);
    analyze_statement(analyzer, node->data.while_stmt.body);
    break;

  case AST_RETURN_STMT: {
    DataType return_type = TYPE_VOID;
    if (node->data.return_stmt.value) {
      return_type = analyze_expression(analyzer, node->data.return_stmt.value);
    }

    // 检查返回类型是否匹配当前函数
    if (analyzer->current_function) {
      if (!semantic_types_compatible(analyzer->current_function->return_type,
                                     return_type)) {
        semantic_error(
            analyzer, SEM_ERROR_RETURN_TYPE, node->line,
            "Function '%s' should return %s, got %s",
            analyzer->current_function->name,
            datatype_to_string(analyzer->current_function->return_type),
            datatype_to_string(return_type));
      }
    }
    break;
  }

  case AST_EXPR_STMT:
    analyze_expression(analyzer, node->data.expr_stmt.expression);
    break;

  default:
    break;
  }
}

/**
 * 分析声明（函数/全局变量）
 */
static void analyze_declaration(SemanticAnalyzer *analyzer, ASTNode *node) {
  if (!node)
    return;

  switch (node->type) {
  case AST_VAR_DECL:
    analyze_statement(analyzer, node); // 复用语句分析
    break;

  case AST_FUNC_DECL: {
    DataType return_type = string_to_datatype(node->data.func_decl.return_type);

    // 检查函数是否已声明
    if (semantic_lookup_current_scope(analyzer, node->data.func_decl.name)) {
      semantic_error(analyzer, SEM_ERROR_REDECLARED, node->line,
                     "Function '%s' already declared",
                     node->data.func_decl.name);
      return;
    }

    // 声明函数
    Symbol *func_sym = semantic_declare(analyzer, node->data.func_decl.name,
                                        SYMBOL_FUNCTION, return_type);
    if (!func_sym)
      return;

    func_sym->return_type = return_type;
    func_sym->param_count = node->data.func_decl.param_count;

    // 复制参数信息
    if (func_sym->param_count > 0) {
      func_sym->params =
          (ParamInfo *)calloc(func_sym->param_count, sizeof(ParamInfo));
      for (int i = 0; i < func_sym->param_count; i++) {
        ASTNode *param = node->data.func_decl.params[i];
        func_sym->params[i].name = str_dup(param->data.param.name);
        func_sym->params[i].type = string_to_datatype(param->data.param.type);
      }
    }

    // 进入函数作用域
    semantic_enter_scope(analyzer);
    analyzer->current_function = func_sym;

    // 声明参数为局部变量
    for (int i = 0; i < node->data.func_decl.param_count; i++) {
      ASTNode *param = node->data.func_decl.params[i];
      DataType param_type = string_to_datatype(param->data.param.type);
      semantic_declare(analyzer, param->data.param.name, SYMBOL_PARAMETER,
                       param_type);
    }

    // 分析函数体
    if (node->data.func_decl.body) {
      // 函数体是一个 block，但我们已经在函数作用域了
      // 所以直接分析 block 内的语句，不创建新作用域
      ASTNode *body = node->data.func_decl.body;
      for (int i = 0; i < body->data.block.count; i++) {
        analyze_statement(analyzer, body->data.block.statements[i]);
      }
    }

    // 退出函数作用域
    analyzer->current_function = NULL;
    semantic_exit_scope(analyzer);
    break;
  }

  default:
    break;
  }
}

/**
 * 分析程序（入口函数）
 */
void semantic_analyze(SemanticAnalyzer *analyzer, ASTNode *ast) {
  if (!ast || ast->type != AST_PROGRAM)
    return;

  // 遍历所有顶层声明
  for (int i = 0; i < ast->data.program.count; i++) {
    analyze_declaration(analyzer, ast->data.program.declarations[i]);
  }
}
