/**
 * ast.c - AST 节点操作函数实现
 *
 * 这个文件包含：
 * 1. 创建各种 AST 节点的函数
 * 2. 打印 AST 的函数（用于调试）
 * 3. 释放 AST 内存的函数
 */

#include "../include/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========== 辅助函数 ==========

/**
 * 复制字符串（带空指针检查）
 */
static char *str_dup(const char *s) {
  if (s == NULL)
    return NULL;
  size_t len = strlen(s) + 1;
  char *copy = (char *)malloc(len);
  if (copy) {
    memcpy(copy, s, len);
  }
  return copy;
}

/**
 * 创建新节点（通用）
 */
static ASTNode *create_node(ASTNodeType type) {
  ASTNode *node = (ASTNode *)calloc(1, sizeof(ASTNode));
  if (node) {
    node->type = type;
    node->line = 0;
    node->column = 0;
  }
  return node;
}

// ========== 节点创建函数 ==========

ASTNode *ast_create_program(void) {
  ASTNode *node = create_node(AST_PROGRAM);
  if (node) {
    node->data.program.declarations = NULL;
    node->data.program.count = 0;
    node->data.program.capacity = 0;
  }
  return node;
}

ASTNode *ast_create_var_decl(const char *type, const char *name,
                             ASTNode *init) {
  ASTNode *node = create_node(AST_VAR_DECL);
  if (node) {
    node->data.var_decl.type = str_dup(type);
    node->data.var_decl.name = str_dup(name);
    node->data.var_decl.initializer = init;
  }
  return node;
}

ASTNode *ast_create_func_decl(const char *ret_type, const char *name,
                              ASTNode **params, int param_count,
                              ASTNode *body) {
  ASTNode *node = create_node(AST_FUNC_DECL);
  if (node) {
    node->data.func_decl.return_type = str_dup(ret_type);
    node->data.func_decl.name = str_dup(name);
    node->data.func_decl.params = params;
    node->data.func_decl.param_count = param_count;
    node->data.func_decl.body = body;
  }
  return node;
}

ASTNode *ast_create_param(const char *type, const char *name) {
  ASTNode *node = create_node(AST_PARAM);
  if (node) {
    node->data.param.type = str_dup(type);
    node->data.param.name = str_dup(name);
  }
  return node;
}

ASTNode *ast_create_block(void) {
  ASTNode *node = create_node(AST_BLOCK);
  if (node) {
    node->data.block.statements = NULL;
    node->data.block.count = 0;
    node->data.block.capacity = 0;
  }
  return node;
}

ASTNode *ast_create_if_stmt(ASTNode *cond, ASTNode *then_br, ASTNode *else_br) {
  ASTNode *node = create_node(AST_IF_STMT);
  if (node) {
    node->data.if_stmt.condition = cond;
    node->data.if_stmt.then_branch = then_br;
    node->data.if_stmt.else_branch = else_br;
  }
  return node;
}

ASTNode *ast_create_while_stmt(ASTNode *cond, ASTNode *body) {
  ASTNode *node = create_node(AST_WHILE_STMT);
  if (node) {
    node->data.while_stmt.condition = cond;
    node->data.while_stmt.body = body;
  }
  return node;
}

ASTNode *ast_create_for_stmt(ASTNode *init, ASTNode *cond, ASTNode *update,
                             ASTNode *body) {
  ASTNode *node = create_node(AST_FOR_STMT);
  if (node) {
    node->data.for_stmt.init = init;
    node->data.for_stmt.condition = cond;
    node->data.for_stmt.update = update;
    node->data.for_stmt.body = body;
  }
  return node;
}

ASTNode *ast_create_return_stmt(ASTNode *value) {
  ASTNode *node = create_node(AST_RETURN_STMT);
  if (node) {
    node->data.return_stmt.value = value;
  }
  return node;
}

ASTNode *ast_create_expr_stmt(ASTNode *expr) {
  ASTNode *node = create_node(AST_EXPR_STMT);
  if (node) {
    node->data.expr_stmt.expression = expr;
  }
  return node;
}

ASTNode *ast_create_binary_expr(BinaryOp op, ASTNode *left, ASTNode *right) {
  ASTNode *node = create_node(AST_BINARY_EXPR);
  if (node) {
    node->data.binary_expr.op = op;
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;
  }
  return node;
}

ASTNode *ast_create_unary_expr(UnaryOp op, ASTNode *operand) {
  ASTNode *node = create_node(AST_UNARY_EXPR);
  if (node) {
    node->data.unary_expr.op = op;
    node->data.unary_expr.operand = operand;
  }
  return node;
}

ASTNode *ast_create_call_expr(const char *callee, ASTNode **args,
                              int arg_count) {
  ASTNode *node = create_node(AST_CALL_EXPR);
  if (node) {
    node->data.call_expr.callee = str_dup(callee);
    node->data.call_expr.arguments = args;
    node->data.call_expr.arg_count = arg_count;
  }
  return node;
}

ASTNode *ast_create_assign_expr(const char *name, ASTNode *value) {
  ASTNode *node = create_node(AST_ASSIGN_EXPR);
  if (node) {
    node->data.assign_expr.name = str_dup(name);
    node->data.assign_expr.value = value;
  }
  return node;
}

ASTNode *ast_create_identifier(const char *name) {
  ASTNode *node = create_node(AST_IDENTIFIER);
  if (node) {
    node->data.identifier.name = str_dup(name);
  }
  return node;
}

ASTNode *ast_create_int_literal(int value) {
  ASTNode *node = create_node(AST_INT_LITERAL);
  if (node) {
    node->data.int_literal.value = value;
  }
  return node;
}

ASTNode *ast_create_float_literal(double value) {
  ASTNode *node = create_node(AST_FLOAT_LITERAL);
  if (node) {
    node->data.float_literal.value = value;
  }
  return node;
}

ASTNode *ast_create_string_literal(const char *value) {
  ASTNode *node = create_node(AST_STRING_LITERAL);
  if (node) {
    node->data.string_literal.value = str_dup(value);
  }
  return node;
}

ASTNode *ast_create_char_literal(char value) {
  ASTNode *node = create_node(AST_CHAR_LITERAL);
  if (node) {
    node->data.char_literal.value = value;
  }
  return node;
}

// ========== 添加子节点函数 ==========

void ast_program_add(ASTNode *program, ASTNode *decl) {
  if (!program || program->type != AST_PROGRAM || !decl)
    return;

  ProgramData *data = &program->data.program;

  // 扩容检查
  if (data->count >= data->capacity) {
    int new_capacity = data->capacity == 0 ? 8 : data->capacity * 2;
    ASTNode **new_array = (ASTNode **)realloc(data->declarations,
                                              sizeof(ASTNode *) * new_capacity);
    if (!new_array)
      return;
    data->declarations = new_array;
    data->capacity = new_capacity;
  }

  data->declarations[data->count++] = decl;
}

void ast_block_add(ASTNode *block, ASTNode *stmt) {
  if (!block || block->type != AST_BLOCK || !stmt)
    return;

  BlockData *data = &block->data.block;

  // 扩容检查
  if (data->count >= data->capacity) {
    int new_capacity = data->capacity == 0 ? 8 : data->capacity * 2;
    ASTNode **new_array =
        (ASTNode **)realloc(data->statements, sizeof(ASTNode *) * new_capacity);
    if (!new_array)
      return;
    data->statements = new_array;
    data->capacity = new_capacity;
  }

  data->statements[data->count++] = stmt;
}

// ========== 辅助转换函数 ==========

const char *ast_node_type_to_string(ASTNodeType type) {
  switch (type) {
  case AST_PROGRAM:
    return "Program";
  case AST_VAR_DECL:
    return "VarDecl";
  case AST_FUNC_DECL:
    return "FuncDecl";
  case AST_PARAM:
    return "Param";
  case AST_BLOCK:
    return "Block";
  case AST_IF_STMT:
    return "IfStmt";
  case AST_WHILE_STMT:
    return "WhileStmt";
  case AST_FOR_STMT:
    return "ForStmt";
  case AST_RETURN_STMT:
    return "ReturnStmt";
  case AST_EXPR_STMT:
    return "ExprStmt";
  case AST_BINARY_EXPR:
    return "BinaryExpr";
  case AST_UNARY_EXPR:
    return "UnaryExpr";
  case AST_CALL_EXPR:
    return "CallExpr";
  case AST_ASSIGN_EXPR:
    return "AssignExpr";
  case AST_IDENTIFIER:
    return "Identifier";
  case AST_INT_LITERAL:
    return "IntLiteral";
  case AST_FLOAT_LITERAL:
    return "FloatLiteral";
  case AST_STRING_LITERAL:
    return "StringLiteral";
  case AST_CHAR_LITERAL:
    return "CharLiteral";
  default:
    return "Unknown";
  }
}

const char *ast_binary_op_to_string(BinaryOp op) {
  switch (op) {
  case OP_ADD:
    return "+";
  case OP_SUB:
    return "-";
  case OP_MUL:
    return "*";
  case OP_DIV:
    return "/";
  case OP_MOD:
    return "%";
  case OP_EQ:
    return "==";
  case OP_NE:
    return "!=";
  case OP_LT:
    return "<";
  case OP_GT:
    return ">";
  case OP_LE:
    return "<=";
  case OP_GE:
    return ">=";
  case OP_AND:
    return "&&";
  case OP_OR:
    return "||";
  default:
    return "?";
  }
}

const char *ast_unary_op_to_string(UnaryOp op) {
  switch (op) {
  case OP_NEG:
    return "-";
  case OP_NOT:
    return "!";
  default:
    return "?";
  }
}

// ========== 打印 AST ==========

/**
 * 打印缩进
 */
static void print_indent(int indent) {
  for (int i = 0; i < indent; i++) {
    printf("  ");
  }
}

/**
 * 递归打印 AST 节点
 */
void ast_print(ASTNode *node, int indent) {
  if (!node) {
    print_indent(indent);
    printf("(null)\n");
    return;
  }

  print_indent(indent);

  switch (node->type) {
  case AST_PROGRAM:
    printf("Program (%d declarations)\n", node->data.program.count);
    for (int i = 0; i < node->data.program.count; i++) {
      ast_print(node->data.program.declarations[i], indent + 1);
    }
    break;

  case AST_VAR_DECL:
    printf("VarDecl: %s %s\n", node->data.var_decl.type,
           node->data.var_decl.name);
    if (node->data.var_decl.initializer) {
      print_indent(indent + 1);
      printf("initializer:\n");
      ast_print(node->data.var_decl.initializer, indent + 2);
    }
    break;

  case AST_FUNC_DECL:
    printf("FuncDecl: %s %s(%d params)\n", node->data.func_decl.return_type,
           node->data.func_decl.name, node->data.func_decl.param_count);
    for (int i = 0; i < node->data.func_decl.param_count; i++) {
      ast_print(node->data.func_decl.params[i], indent + 1);
    }
    if (node->data.func_decl.body) {
      print_indent(indent + 1);
      printf("body:\n");
      ast_print(node->data.func_decl.body, indent + 2);
    }
    break;

  case AST_PARAM:
    printf("Param: %s %s\n", node->data.param.type, node->data.param.name);
    break;

  case AST_BLOCK:
    printf("Block (%d statements)\n", node->data.block.count);
    for (int i = 0; i < node->data.block.count; i++) {
      ast_print(node->data.block.statements[i], indent + 1);
    }
    break;

  case AST_IF_STMT:
    printf("IfStmt\n");
    print_indent(indent + 1);
    printf("condition:\n");
    ast_print(node->data.if_stmt.condition, indent + 2);
    print_indent(indent + 1);
    printf("then:\n");
    ast_print(node->data.if_stmt.then_branch, indent + 2);
    if (node->data.if_stmt.else_branch) {
      print_indent(indent + 1);
      printf("else:\n");
      ast_print(node->data.if_stmt.else_branch, indent + 2);
    }
    break;

  case AST_WHILE_STMT:
    printf("WhileStmt\n");
    print_indent(indent + 1);
    printf("condition:\n");
    ast_print(node->data.while_stmt.condition, indent + 2);
    print_indent(indent + 1);
    printf("body:\n");
    ast_print(node->data.while_stmt.body, indent + 2);
    break;

  case AST_FOR_STMT:
    printf("ForStmt\n");
    if (node->data.for_stmt.init) {
      print_indent(indent + 1);
      printf("init:\n");
      ast_print(node->data.for_stmt.init, indent + 2);
    }
    if (node->data.for_stmt.condition) {
      print_indent(indent + 1);
      printf("condition:\n");
      ast_print(node->data.for_stmt.condition, indent + 2);
    }
    if (node->data.for_stmt.update) {
      print_indent(indent + 1);
      printf("update:\n");
      ast_print(node->data.for_stmt.update, indent + 2);
    }
    print_indent(indent + 1);
    printf("body:\n");
    ast_print(node->data.for_stmt.body, indent + 2);
    break;

  case AST_RETURN_STMT:
    printf("ReturnStmt\n");
    if (node->data.return_stmt.value) {
      ast_print(node->data.return_stmt.value, indent + 1);
    }
    break;

  case AST_EXPR_STMT:
    printf("ExprStmt\n");
    ast_print(node->data.expr_stmt.expression, indent + 1);
    break;

  case AST_BINARY_EXPR:
    printf("BinaryExpr: %s\n",
           ast_binary_op_to_string(node->data.binary_expr.op));
    ast_print(node->data.binary_expr.left, indent + 1);
    ast_print(node->data.binary_expr.right, indent + 1);
    break;

  case AST_UNARY_EXPR:
    printf("UnaryExpr: %s\n", ast_unary_op_to_string(node->data.unary_expr.op));
    ast_print(node->data.unary_expr.operand, indent + 1);
    break;

  case AST_CALL_EXPR:
    printf("CallExpr: %s(%d args)\n", node->data.call_expr.callee,
           node->data.call_expr.arg_count);
    for (int i = 0; i < node->data.call_expr.arg_count; i++) {
      ast_print(node->data.call_expr.arguments[i], indent + 1);
    }
    break;

  case AST_ASSIGN_EXPR:
    printf("AssignExpr: %s =\n", node->data.assign_expr.name);
    ast_print(node->data.assign_expr.value, indent + 1);
    break;

  case AST_IDENTIFIER:
    printf("Identifier: %s\n", node->data.identifier.name);
    break;

  case AST_INT_LITERAL:
    printf("IntLiteral: %d\n", node->data.int_literal.value);
    break;

  case AST_FLOAT_LITERAL:
    printf("FloatLiteral: %f\n", node->data.float_literal.value);
    break;

  case AST_STRING_LITERAL:
    printf("StringLiteral: \"%s\"\n", node->data.string_literal.value);
    break;

  case AST_CHAR_LITERAL:
    printf("CharLiteral: '%c'\n", node->data.char_literal.value);
    break;

  default:
    printf("Unknown node type: %d\n", node->type);
    break;
  }
}

// ========== 释放 AST 内存 ==========

void ast_free(ASTNode *node) {
  if (!node)
    return;

  switch (node->type) {
  case AST_PROGRAM:
    for (int i = 0; i < node->data.program.count; i++) {
      ast_free(node->data.program.declarations[i]);
    }
    free(node->data.program.declarations);
    break;

  case AST_VAR_DECL:
    free(node->data.var_decl.type);
    free(node->data.var_decl.name);
    ast_free(node->data.var_decl.initializer);
    break;

  case AST_FUNC_DECL:
    free(node->data.func_decl.return_type);
    free(node->data.func_decl.name);
    for (int i = 0; i < node->data.func_decl.param_count; i++) {
      ast_free(node->data.func_decl.params[i]);
    }
    free(node->data.func_decl.params);
    ast_free(node->data.func_decl.body);
    break;

  case AST_PARAM:
    free(node->data.param.type);
    free(node->data.param.name);
    break;

  case AST_BLOCK:
    for (int i = 0; i < node->data.block.count; i++) {
      ast_free(node->data.block.statements[i]);
    }
    free(node->data.block.statements);
    break;

  case AST_IF_STMT:
    ast_free(node->data.if_stmt.condition);
    ast_free(node->data.if_stmt.then_branch);
    ast_free(node->data.if_stmt.else_branch);
    break;

  case AST_WHILE_STMT:
    ast_free(node->data.while_stmt.condition);
    ast_free(node->data.while_stmt.body);
    break;

  case AST_FOR_STMT:
    ast_free(node->data.for_stmt.init);
    ast_free(node->data.for_stmt.condition);
    ast_free(node->data.for_stmt.update);
    ast_free(node->data.for_stmt.body);
    break;

  case AST_RETURN_STMT:
    ast_free(node->data.return_stmt.value);
    break;

  case AST_EXPR_STMT:
    ast_free(node->data.expr_stmt.expression);
    break;

  case AST_BINARY_EXPR:
    ast_free(node->data.binary_expr.left);
    ast_free(node->data.binary_expr.right);
    break;

  case AST_UNARY_EXPR:
    ast_free(node->data.unary_expr.operand);
    break;

  case AST_CALL_EXPR:
    free(node->data.call_expr.callee);
    for (int i = 0; i < node->data.call_expr.arg_count; i++) {
      ast_free(node->data.call_expr.arguments[i]);
    }
    free(node->data.call_expr.arguments);
    break;

  case AST_ASSIGN_EXPR:
    free(node->data.assign_expr.name);
    ast_free(node->data.assign_expr.value);
    break;

  case AST_IDENTIFIER:
    free(node->data.identifier.name);
    break;

  case AST_INT_LITERAL:
  case AST_FLOAT_LITERAL:
  case AST_CHAR_LITERAL:
    // 无需释放
    break;

  case AST_STRING_LITERAL:
    free(node->data.string_literal.value);
    break;

  default:
    break;
  }

  free(node);
}
