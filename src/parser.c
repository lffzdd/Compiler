/**
 * parser.c - 递归下降语法分析器实现
 *
 * 这是编译器的第二个阶段！
 *
 * 语法分析的核心思想：
 * 1. 为每个语法规则写一个 parse_xxx() 函数
 * 2. 函数读取 Token，构建对应的 AST 节点
 * 3. 函数之间相互调用形成递归结构
 *
 * 我们支持的语法（简化版 C 语言）：
 *
 * program     → declaration*
 * declaration → var_decl | func_decl
 * var_decl    → type IDENTIFIER ("=" expression)? ";"
 * func_decl   → type IDENTIFIER "(" params? ")" block
 * params      → param ("," param)*
 * param       → type IDENTIFIER
 * block       → "{" statement* "}"
 * statement   → var_decl | if_stmt | while_stmt | return_stmt | expr_stmt
 * if_stmt     → "if" "(" expression ")" statement ("else" statement)?
 * while_stmt  → "while" "(" expression ")" statement
 * return_stmt → "return" expression? ";"
 * expr_stmt   → expression ";"
 * expression  → assignment
 * assignment  → IDENTIFIER "=" assignment | logic_or
 * logic_or    → logic_and ("||" logic_and)*
 * logic_and   → equality ("&&" equality)*
 * equality    → comparison (("==" | "!=") comparison)*
 * comparison  → term (("<" | ">" | "<=" | ">=") term)*
 * term        → factor (("+" | "-") factor)*
 * factor      → unary (("*" | "/" | "%") unary)*
 * unary       → ("-" | "!") unary | call
 * call        → primary ("(" arguments? ")")*
 * arguments   → expression ("," expression)*
 * primary     → INTEGER | FLOAT | STRING | CHAR | IDENTIFIER | "(" expression
 * ")"
 */

#include "../include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========== 错误处理 ==========

/**
 * 报告错误
 */
static void error_at(Parser *parser, Token *token, const char *message) {
  if (parser->panic_mode)
    return; // 已在错误恢复模式，不重复报告
  parser->panic_mode = 1;
  parser->had_error = 1;

  fprintf(stderr, "[Line %d] Error", parser->lexer->line);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, " at end");
  } else if (token->type != TOKEN_UNKNOWN) {
    fprintf(stderr, " at '%s'", token->value);
  }

  fprintf(stderr, ": %s\n", message);
}

/**
 * 报告当前 Token 的错误
 */
static void error(Parser *parser, const char *message) {
  error_at(parser, &parser->current, message);
}

// ========== Token 操作 ==========

/**
 * 前进到下一个 Token
 */
static void advance(Parser *parser) {
  parser->previous = parser->current;

  // 跳过错误的 Token
  while (1) {
    parser->current = lexer_next_token(parser->lexer);
    if (parser->current.type != TOKEN_UNKNOWN)
      break;
    error(parser, "Unexpected character.");
  }
}

/**
 * 检查当前 Token 类型
 */
static int check(Parser *parser, TokenType type) {
  return parser->current.type == type;
}

/**
 * 检查当前 Token 是否是指定的关键字
 */
static int check_keyword(Parser *parser, const char *keyword) {
  return parser->current.type == TOKEN_KEYWORD &&
         strcmp(parser->current.value, keyword) == 0;
}

/**
 * 如果当前 Token 类型匹配，则前进
 */
static int match(Parser *parser, TokenType type) {
  if (!check(parser, type))
    return 0;
  advance(parser);
  return 1;
}

/**
 * 如果当前是指定关键字，则前进
 */
static int match_keyword(Parser *parser, const char *keyword) {
  if (!check_keyword(parser, keyword))
    return 0;
  advance(parser);
  return 1;
}

/**
 * 期望当前 Token 是指定类型，否则报错
 */
static void consume(Parser *parser, TokenType type, const char *message) {
  if (parser->current.type == type) {
    advance(parser);
    return;
  }
  error(parser, message);
}

/**
 * 期望当前 Token 是指定关键字，否则报错
 */
static void consume_keyword(Parser *parser, const char *keyword,
                            const char *message) {
  if (check_keyword(parser, keyword)) {
    advance(parser);
    return;
  }
  error(parser, message);
}

/**
 * 检查是否是类型关键字
 */
static int is_type_keyword(Parser *parser) {
  if (parser->current.type != TOKEN_KEYWORD)
    return 0;
  return strcmp(parser->current.value, "int") == 0 ||
         strcmp(parser->current.value, "float") == 0 ||
         strcmp(parser->current.value, "char") == 0 ||
         strcmp(parser->current.value, "void") == 0;
}

/**
 * 错误恢复：同步到下一个语句
 */
static void synchronize(Parser *parser) {
  parser->panic_mode = 0;

  while (parser->current.type != TOKEN_EOF) {
    // 分号后面通常是新语句的开始
    if (parser->previous.type == TOKEN_SEMICOLON)
      return;

    // 这些关键字通常标志着新语句的开始
    if (parser->current.type == TOKEN_KEYWORD) {
      if (strcmp(parser->current.value, "if") == 0 ||
          strcmp(parser->current.value, "while") == 0 ||
          strcmp(parser->current.value, "for") == 0 ||
          strcmp(parser->current.value, "return") == 0 ||
          strcmp(parser->current.value, "int") == 0 ||
          strcmp(parser->current.value, "float") == 0 ||
          strcmp(parser->current.value, "char") == 0 ||
          strcmp(parser->current.value, "void") == 0) {
        return;
      }
    }

    advance(parser);
  }
}

// ========== 运算符转换 ==========

/**
 * 将运算符字符串转换为 BinaryOp 枚举
 */
static BinaryOp string_to_binary_op(const char *op) {
  if (strcmp(op, "+") == 0)
    return OP_ADD;
  if (strcmp(op, "-") == 0)
    return OP_SUB;
  if (strcmp(op, "*") == 0)
    return OP_MUL;
  if (strcmp(op, "/") == 0)
    return OP_DIV;
  if (strcmp(op, "%") == 0)
    return OP_MOD;
  if (strcmp(op, "==") == 0)
    return OP_EQ;
  if (strcmp(op, "!=") == 0)
    return OP_NE;
  if (strcmp(op, "<") == 0)
    return OP_LT;
  if (strcmp(op, ">") == 0)
    return OP_GT;
  if (strcmp(op, "<=") == 0)
    return OP_LE;
  if (strcmp(op, ">=") == 0)
    return OP_GE;
  if (strcmp(op, "&&") == 0)
    return OP_AND;
  if (strcmp(op, "||") == 0)
    return OP_OR;
  return OP_ADD; // 默认
}

// ========== 前向声明 ==========

static ASTNode *parse_expression(Parser *parser);
static ASTNode *parse_statement(Parser *parser);
static ASTNode *parse_block(Parser *parser);

// ========== 表达式解析 ==========

/**
 * primary → INTEGER | FLOAT | STRING | CHAR | IDENTIFIER | "(" expression ")"
 *
 * 这是优先级最高的解析函数，处理"原子"元素
 */
static ASTNode *parse_primary(Parser *parser) {
  // 整数
  if (match(parser, TOKEN_INTEGER)) {
    int value = atoi(parser->previous.value);
    return ast_create_int_literal(value);
  }

  // 浮点数
  if (match(parser, TOKEN_FLOAT)) {
    double value = atof(parser->previous.value);
    return ast_create_float_literal(value);
  }

  // 字符串
  if (match(parser, TOKEN_STRING)) {
    return ast_create_string_literal(parser->previous.value);
  }

  // 字符
  if (match(parser, TOKEN_CHAR)) {
    char value = parser->previous.value[0];
    return ast_create_char_literal(value);
  }

  // true/false
  if (check_keyword(parser, "true")) {
    advance(parser);
    return ast_create_int_literal(1);
  }
  if (check_keyword(parser, "false")) {
    advance(parser);
    return ast_create_int_literal(0);
  }

  // 标识符
  if (match(parser, TOKEN_IDENTIFIER)) {
    return ast_create_identifier(parser->previous.value);
  }

  // 括号表达式
  if (match(parser, TOKEN_LPAREN)) {
    ASTNode *expr = parse_expression(parser);
    consume(parser, TOKEN_RPAREN, "Expect ')' after expression.");
    return expr;
  }

  error(parser, "Expect expression.");
  return NULL;
}

/**
 * call → primary ("(" arguments? ")")*
 *
 * 处理函数调用
 */
static ASTNode *parse_call(Parser *parser) {
  ASTNode *expr = parse_primary(parser);

  while (1) {
    if (match(parser, TOKEN_LPAREN)) {
      // 这是一个函数调用
      if (expr->type != AST_IDENTIFIER) {
        error(parser, "Can only call functions.");
        return expr;
      }

      char *callee = expr->data.identifier.name;

      // 解析参数列表
      ASTNode **args = NULL;
      int arg_count = 0;
      int arg_capacity = 0;

      if (!check(parser, TOKEN_RPAREN)) {
        do {
          ASTNode *arg = parse_expression(parser);

          // 扩容
          if (arg_count >= arg_capacity) {
            arg_capacity = arg_capacity == 0 ? 4 : arg_capacity * 2;
            args = (ASTNode **)realloc(args, sizeof(ASTNode *) * arg_capacity);
          }
          args[arg_count++] = arg;
        } while (match(parser, TOKEN_COMMA));
      }

      consume(parser, TOKEN_RPAREN, "Expect ')' after arguments.");

      // 创建新的函数调用节点，复制函数名
      char *callee_copy = (char *)malloc(strlen(callee) + 1);
      strcpy(callee_copy, callee);

      // 释放原来的标识符节点
      ast_free(expr);

      expr = ast_create_call_expr(callee_copy, args, arg_count);
      free(callee_copy);
    } else {
      break;
    }
  }

  return expr;
}

/**
 * unary → ("-" | "!") unary | call
 *
 * 处理一元运算符
 */
static ASTNode *parse_unary(Parser *parser) {
  // 检查一元运算符
  if (check(parser, TOKEN_OPERATOR) &&
      (strcmp(parser->current.value, "-") == 0 ||
       strcmp(parser->current.value, "!") == 0)) {
    advance(parser);
    UnaryOp op = parser->previous.value[0] == '-' ? OP_NEG : OP_NOT;
    ASTNode *operand = parse_unary(parser); // 递归处理右边
    return ast_create_unary_expr(op, operand);
  }

  return parse_call(parser);
}

/**
 * factor → unary (("*" | "/" | "%") unary)*
 *
 * 处理乘法、除法、取模（高优先级）
 */
static ASTNode *parse_factor(Parser *parser) {
  ASTNode *left = parse_unary(parser);

  while (check(parser, TOKEN_OPERATOR) &&
         (strcmp(parser->current.value, "*") == 0 ||
          strcmp(parser->current.value, "/") == 0 ||
          strcmp(parser->current.value, "%") == 0)) {
    advance(parser);
    BinaryOp op = string_to_binary_op(parser->previous.value);
    ASTNode *right = parse_unary(parser);
    left = ast_create_binary_expr(op, left, right);
  }

  return left;
}

/**
 * term → factor (("+" | "-") factor)*
 *
 * 处理加法、减法（低于乘除的优先级）
 */
static ASTNode *parse_term(Parser *parser) {
  ASTNode *left = parse_factor(parser);

  while (check(parser, TOKEN_OPERATOR) &&
         (strcmp(parser->current.value, "+") == 0 ||
          strcmp(parser->current.value, "-") == 0)) {
    advance(parser);
    BinaryOp op = string_to_binary_op(parser->previous.value);
    ASTNode *right = parse_factor(parser);
    left = ast_create_binary_expr(op, left, right);
  }

  return left;
}

/**
 * comparison → term (("<" | ">" | "<=" | ">=") term)*
 *
 * 处理比较运算符
 */
static ASTNode *parse_comparison(Parser *parser) {
  ASTNode *left = parse_term(parser);

  while (check(parser, TOKEN_OPERATOR) &&
         (strcmp(parser->current.value, "<") == 0 ||
          strcmp(parser->current.value, ">") == 0 ||
          strcmp(parser->current.value, "<=") == 0 ||
          strcmp(parser->current.value, ">=") == 0)) {
    advance(parser);
    BinaryOp op = string_to_binary_op(parser->previous.value);
    ASTNode *right = parse_term(parser);
    left = ast_create_binary_expr(op, left, right);
  }

  return left;
}

/**
 * equality → comparison (("==" | "!=") comparison)*
 *
 * 处理相等性比较
 */
static ASTNode *parse_equality(Parser *parser) {
  ASTNode *left = parse_comparison(parser);

  while (check(parser, TOKEN_OPERATOR) &&
         (strcmp(parser->current.value, "==") == 0 ||
          strcmp(parser->current.value, "!=") == 0)) {
    advance(parser);
    BinaryOp op = string_to_binary_op(parser->previous.value);
    ASTNode *right = parse_comparison(parser);
    left = ast_create_binary_expr(op, left, right);
  }

  return left;
}

/**
 * logic_and → equality ("&&" equality)*
 */
static ASTNode *parse_logic_and(Parser *parser) {
  ASTNode *left = parse_equality(parser);

  while (check(parser, TOKEN_OPERATOR) &&
         strcmp(parser->current.value, "&&") == 0) {
    advance(parser);
    ASTNode *right = parse_equality(parser);
    left = ast_create_binary_expr(OP_AND, left, right);
  }

  return left;
}

/**
 * logic_or → logic_and ("||" logic_and)*
 */
static ASTNode *parse_logic_or(Parser *parser) {
  ASTNode *left = parse_logic_and(parser);

  while (check(parser, TOKEN_OPERATOR) &&
         strcmp(parser->current.value, "||") == 0) {
    advance(parser);
    ASTNode *right = parse_logic_and(parser);
    left = ast_create_binary_expr(OP_OR, left, right);
  }

  return left;
}

/**
 * assignment → IDENTIFIER "=" assignment | logic_or
 *
 * 赋值是右结合的，所以递归调用自身
 */
static ASTNode *parse_assignment(Parser *parser) {
  ASTNode *expr = parse_logic_or(parser);

  if (check(parser, TOKEN_OPERATOR) &&
      strcmp(parser->current.value, "=") == 0) {
    advance(parser);

    // 检查左边是否是标识符
    if (expr->type != AST_IDENTIFIER) {
      error(parser, "Invalid assignment target.");
      return expr;
    }

    char *name = expr->data.identifier.name;
    ASTNode *value = parse_assignment(parser); // 右结合，递归

    // 创建赋值节点
    ASTNode *assign = ast_create_assign_expr(name, value);
    ast_free(expr); // 释放原来的标识符节点
    return assign;
  }

  return expr;
}

/**
 * expression → assignment
 */
static ASTNode *parse_expression(Parser *parser) {
  return parse_assignment(parser);
}

// ========== 语句解析 ==========

/**
 * expr_stmt → expression ";"
 */
static ASTNode *parse_expr_statement(Parser *parser) {
  ASTNode *expr = parse_expression(parser);
  consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
  return ast_create_expr_stmt(expr);
}

/**
 * return_stmt → "return" expression? ";"
 */
static ASTNode *parse_return_statement(Parser *parser) {
  consume_keyword(parser, "return", "Expect 'return'.");

  ASTNode *value = NULL;
  if (!check(parser, TOKEN_SEMICOLON)) {
    value = parse_expression(parser);
  }

  consume(parser, TOKEN_SEMICOLON, "Expect ';' after return value.");
  return ast_create_return_stmt(value);
}

/**
 * while_stmt → "while" "(" expression ")" statement
 */
static ASTNode *parse_while_statement(Parser *parser) {
  consume_keyword(parser, "while", "Expect 'while'.");
  consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'.");

  ASTNode *condition = parse_expression(parser);

  consume(parser, TOKEN_RPAREN, "Expect ')' after condition.");

  ASTNode *body = parse_statement(parser);

  return ast_create_while_stmt(condition, body);
}

/**
 * if_stmt → "if" "(" expression ")" statement ("else" statement)?
 */
static ASTNode *parse_if_statement(Parser *parser) {
  consume_keyword(parser, "if", "Expect 'if'.");
  consume(parser, TOKEN_LPAREN, "Expect '(' after 'if'.");

  ASTNode *condition = parse_expression(parser);

  consume(parser, TOKEN_RPAREN, "Expect ')' after condition.");

  ASTNode *then_branch = parse_statement(parser);
  ASTNode *else_branch = NULL;

  if (match_keyword(parser, "else")) {
    else_branch = parse_statement(parser);
  }

  return ast_create_if_stmt(condition, then_branch, else_branch);
}

/**
 * block → "{" statement* "}"
 */
static ASTNode *parse_block(Parser *parser) {
  consume(parser, TOKEN_LBRACE, "Expect '{'.");

  ASTNode *block = ast_create_block();

  while (!check(parser, TOKEN_RBRACE) && !check(parser, TOKEN_EOF)) {
    ASTNode *stmt = parse_statement(parser);
    if (stmt) {
      ast_block_add(block, stmt);
    }

    if (parser->panic_mode) {
      synchronize(parser);
    }
  }

  consume(parser, TOKEN_RBRACE, "Expect '}' after block.");

  return block;
}

/**
 * var_decl → type IDENTIFIER ("=" expression)? ";"
 */
static ASTNode *parse_var_declaration(Parser *parser) {
  // 保存类型
  char type[64];
  strcpy(type, parser->current.value);
  advance(parser); // 跳过类型

  // 期望标识符
  consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
  char name[256];
  strcpy(name, parser->previous.value);

  // 可选的初始化
  ASTNode *initializer = NULL;
  if (check(parser, TOKEN_OPERATOR) &&
      strcmp(parser->current.value, "=") == 0) {
    advance(parser);
    initializer = parse_expression(parser);
  }

  consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

  return ast_create_var_decl(type, name, initializer);
}

/**
 * statement → block | if_stmt | while_stmt | return_stmt | var_decl | expr_stmt
 */
static ASTNode *parse_statement(Parser *parser) {
  // 代码块
  if (check(parser, TOKEN_LBRACE)) {
    return parse_block(parser);
  }

  // if 语句
  if (check_keyword(parser, "if")) {
    return parse_if_statement(parser);
  }

  // while 语句
  if (check_keyword(parser, "while")) {
    return parse_while_statement(parser);
  }

  // return 语句
  if (check_keyword(parser, "return")) {
    return parse_return_statement(parser);
  }

  // 变量声明
  if (is_type_keyword(parser)) {
    return parse_var_declaration(parser);
  }

  // 默认：表达式语句
  return parse_expr_statement(parser);
}

// ========== 顶层声明解析 ==========

/**
 * 解析函数参数列表
 */
static void parse_parameters(Parser *parser, ASTNode ***params,
                             int *param_count) {
  *params = NULL;
  *param_count = 0;
  int capacity = 0;

  if (!is_type_keyword(parser)) {
    return; // 无参数
  }

  do {
    // 参数类型
    char type[64];
    strcpy(type, parser->current.value);
    advance(parser);

    // 参数名
    consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
    char name[256];
    strcpy(name, parser->previous.value);

    // 创建参数节点
    ASTNode *param = ast_create_param(type, name);

    // 扩容
    if (*param_count >= capacity) {
      capacity = capacity == 0 ? 4 : capacity * 2;
      *params = (ASTNode **)realloc(*params, sizeof(ASTNode *) * capacity);
    }
    (*params)[(*param_count)++] = param;

  } while (match(parser, TOKEN_COMMA));
}

/**
 * 解析函数定义
 * func_decl → type IDENTIFIER "(" params? ")" block
 */
static ASTNode *parse_function_declaration(Parser *parser) {
  // 返回类型
  char return_type[64];
  strcpy(return_type, parser->current.value);
  advance(parser);

  // 函数名
  consume(parser, TOKEN_IDENTIFIER, "Expect function name.");
  char name[256];
  strcpy(name, parser->previous.value);

  // 参数列表
  consume(parser, TOKEN_LPAREN, "Expect '(' after function name.");

  ASTNode **params = NULL;
  int param_count = 0;
  parse_parameters(parser, &params, &param_count);

  consume(parser, TOKEN_RPAREN, "Expect ')' after parameters.");

  // 函数体
  ASTNode *body = parse_block(parser);

  return ast_create_func_decl(return_type, name, params, param_count, body);
}

/**
 * declaration → func_decl | var_decl
 *
 * 区分函数定义和变量声明的方法：
 * - 如果类型后面是 IDENTIFIER 再后面是 '('，那是函数
 * - 否则是变量
 */
static ASTNode *parse_declaration(Parser *parser) {
  if (!is_type_keyword(parser)) {
    error(parser, "Expect type.");
    return NULL;
  }

  // 前瞻：检查是函数还是变量
  // type IDENTIFIER ( → 函数
  // type IDENTIFIER ; 或 type IDENTIFIER = → 变量

  // 保存当前状态用于前瞻
  int saved_pos = parser->lexer->pos;
  int saved_line = parser->lexer->line;
  int saved_column = parser->lexer->column;
  Token saved_current = parser->current;
  Token saved_previous = parser->previous;

  // 读取类型和标识符
  advance(parser); // 跳过类型

  if (parser->current.type != TOKEN_IDENTIFIER) {
    // 恢复状态
    parser->lexer->pos = saved_pos;
    parser->lexer->line = saved_line;
    parser->lexer->column = saved_column;
    parser->current = saved_current;
    parser->previous = saved_previous;

    error(parser, "Expect identifier after type.");
    return NULL;
  }

  advance(parser); // 跳过标识符

  int is_function = (parser->current.type == TOKEN_LPAREN);

  // 恢复状态
  parser->lexer->pos = saved_pos;
  parser->lexer->line = saved_line;
  parser->lexer->column = saved_column;
  parser->current = saved_current;
  parser->previous = saved_previous;

  if (is_function) {
    return parse_function_declaration(parser);
  } else {
    return parse_var_declaration(parser);
  }
}

// ========== 主要接口 ==========

/**
 * parser_init - 初始化语法分析器
 */
Parser parser_init(Lexer *lexer) {
  Parser parser;
  parser.lexer = lexer;
  parser.had_error = 0;
  parser.panic_mode = 0;

  // 读取第一个 Token
  advance(&parser);

  return parser;
}

/**
 * parser_parse - 解析整个程序
 *
 * program → declaration*
 */
ASTNode *parser_parse(Parser *parser) {
  ASTNode *program = ast_create_program();

  while (!check(parser, TOKEN_EOF)) {
    ASTNode *decl = parse_declaration(parser);
    if (decl) {
      ast_program_add(program, decl);
    }

    // 错误恢复
    if (parser->panic_mode) {
      synchronize(parser);
    }
  }

  return program;
}

/**
 * parser_had_error - 检查是否有错误
 */
int parser_had_error(Parser *parser) { return parser->had_error; }
