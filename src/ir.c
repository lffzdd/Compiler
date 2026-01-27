/**
 * ir.c - 中间代码生成器实现
 *
 * 这是编译器的第四个阶段！
 *
 * 核心工作：
 * 1. 遍历 AST
 * 2. 为每个节点生成对应的三地址码
 * 3. 使用临时变量保存中间结果
 * 4. 使用标签处理控制流
 */

#include "../include/ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ========== 辅助函数 ==========

static char *str_dup(const char *s) {
  if (!s)
    return NULL;
  size_t len = strlen(s) + 1;
  char *copy = (char *)malloc(len);
  if (copy)
    memcpy(copy, s, len);
  return copy;
}

// ========== 操作数构造 ==========

IROperand ir_operand_none(void) {
  IROperand op = {0};
  op.type = OPERAND_NONE;
  return op;
}

IROperand ir_operand_temp(int id) {
  IROperand op = {0};
  op.type = OPERAND_TEMP;
  op.value.temp_id = id;
  return op;
}

IROperand ir_operand_var(const char *name) {
  IROperand op = {0};
  op.type = OPERAND_VAR;
  op.value.name = str_dup(name);
  return op;
}

IROperand ir_operand_int(int val) {
  IROperand op = {0};
  op.type = OPERAND_INT;
  op.value.int_val = val;
  return op;
}

IROperand ir_operand_float(double val) {
  IROperand op = {0};
  op.type = OPERAND_FLOAT;
  op.value.float_val = val;
  return op;
}

IROperand ir_operand_label(int id) {
  IROperand op = {0};
  op.type = OPERAND_LABEL;
  op.value.label_id = id;
  return op;
}

IROperand ir_operand_func(const char *name) {
  IROperand op = {0};
  op.type = OPERAND_FUNC;
  op.value.name = str_dup(name);
  return op;
}

// ========== IR 程序操作 ==========

IRProgram *ir_program_create(void) {
  IRProgram *program = (IRProgram *)calloc(1, sizeof(IRProgram));
  if (program) {
    program->instructions = NULL;
    program->count = 0;
    program->capacity = 0;
    program->temp_counter = 0;
    program->label_counter = 0;
  }
  return program;
}

void ir_program_free(IRProgram *program) {
  if (!program)
    return;

  for (int i = 0; i < program->count; i++) {
    IRInstruction *instr = &program->instructions[i];
    if (instr->result.type == OPERAND_VAR || instr->result.type == OPERAND_FUNC)
      free(instr->result.value.name);
    if (instr->arg1.type == OPERAND_VAR || instr->arg1.type == OPERAND_FUNC)
      free(instr->arg1.value.name);
    if (instr->arg2.type == OPERAND_VAR || instr->arg2.type == OPERAND_FUNC)
      free(instr->arg2.value.name);
  }

  free(program->instructions);
  free(program);
}

IROperand ir_new_temp(IRProgram *program) {
  return ir_operand_temp(program->temp_counter++);
}

int ir_new_label(IRProgram *program) { return program->label_counter++; }

void ir_emit(IRProgram *program, IROpcode op, IROperand result, IROperand arg1,
             IROperand arg2) {
  // 扩容
  if (program->count >= program->capacity) {
    int new_cap = program->capacity == 0 ? 64 : program->capacity * 2;
    program->instructions = (IRInstruction *)realloc(
        program->instructions, sizeof(IRInstruction) * new_cap);
    program->capacity = new_cap;
  }

  IRInstruction *instr = &program->instructions[program->count++];
  instr->opcode = op;
  instr->result = result;
  instr->arg1 = arg1;
  instr->arg2 = arg2;
  instr->arg_count = 0;
}

// 发射带参数数量的指令（用于函数调用）
static void ir_emit_call(IRProgram *program, IROperand result, IROperand func,
                         int arg_count) {
  if (program->count >= program->capacity) {
    int new_cap = program->capacity == 0 ? 64 : program->capacity * 2;
    program->instructions = (IRInstruction *)realloc(
        program->instructions, sizeof(IRInstruction) * new_cap);
    program->capacity = new_cap;
  }

  IRInstruction *instr = &program->instructions[program->count++];
  instr->opcode = IR_CALL;
  instr->result = result;
  instr->arg1 = func;
  instr->arg2 = ir_operand_none();
  instr->arg_count = arg_count;
}

// ========== AST 到 IR 翻译 ==========

// 前向声明
static IROperand translate_expression(IRProgram *program, ASTNode *node);
static void translate_statement(IRProgram *program, ASTNode *node);

/**
 * 将二元操作符转换为 IR 操作码
 */
static IROpcode binary_op_to_ir(BinaryOp op) {
  switch (op) {
  case OP_ADD:
    return IR_ADD;
  case OP_SUB:
    return IR_SUB;
  case OP_MUL:
    return IR_MUL;
  case OP_DIV:
    return IR_DIV;
  case OP_MOD:
    return IR_MOD;
  case OP_EQ:
    return IR_EQ;
  case OP_NE:
    return IR_NE;
  case OP_LT:
    return IR_LT;
  case OP_GT:
    return IR_GT;
  case OP_LE:
    return IR_LE;
  case OP_GE:
    return IR_GE;
  case OP_AND:
    return IR_AND;
  case OP_OR:
    return IR_OR;
  default:
    return IR_NOP;
  }
}

/**
 * 翻译表达式，返回保存结果的操作数
 */
static IROperand translate_expression(IRProgram *program, ASTNode *node) {
  if (!node)
    return ir_operand_none();

  switch (node->type) {
  case AST_INT_LITERAL: {
    // 整数字面量：直接返回常量
    return ir_operand_int(node->data.int_literal.value);
  }

  case AST_FLOAT_LITERAL: {
    return ir_operand_float(node->data.float_literal.value);
  }

  case AST_IDENTIFIER: {
    // 变量：直接返回变量名
    return ir_operand_var(node->data.identifier.name);
  }

  case AST_BINARY_EXPR: {
    // 二元表达式：先翻译两个操作数，再生成运算指令
    IROperand left = translate_expression(program, node->data.binary_expr.left);
    IROperand right =
        translate_expression(program, node->data.binary_expr.right);
    IROperand result = ir_new_temp(program);

    IROpcode op = binary_op_to_ir(node->data.binary_expr.op);
    ir_emit(program, op, result, left, right);

    return result;
  }

  case AST_UNARY_EXPR: {
    IROperand operand =
        translate_expression(program, node->data.unary_expr.operand);
    IROperand result = ir_new_temp(program);

    if (node->data.unary_expr.op == OP_NEG) {
      ir_emit(program, IR_NEG, result, operand, ir_operand_none());
    } else {
      ir_emit(program, IR_NOT, result, operand, ir_operand_none());
    }

    return result;
  }

  case AST_ASSIGN_EXPR: {
    // 赋值表达式
    IROperand value =
        translate_expression(program, node->data.assign_expr.value);
    IROperand var = ir_operand_var(node->data.assign_expr.name);
    ir_emit(program, IR_ASSIGN, var, value, ir_operand_none());
    return var;
  }

  case AST_CALL_EXPR: {
    // 函数调用：先传递所有参数，再调用
    int arg_count = node->data.call_expr.arg_count;

    // 翻译并传递每个参数
    for (int i = 0; i < arg_count; i++) {
      IROperand arg =
          translate_expression(program, node->data.call_expr.arguments[i]);
      ir_emit(program, IR_PARAM, ir_operand_none(), arg, ir_operand_none());
    }

    // 调用函数
    IROperand result = ir_new_temp(program);
    IROperand func = ir_operand_func(node->data.call_expr.callee);
    ir_emit_call(program, result, func, arg_count);

    return result;
  }

  default:
    return ir_operand_none();
  }
}

/**
 * 翻译语句
 */
static void translate_statement(IRProgram *program, ASTNode *node) {
  if (!node)
    return;

  switch (node->type) {
  case AST_BLOCK: {
    for (int i = 0; i < node->data.block.count; i++) {
      translate_statement(program, node->data.block.statements[i]);
    }
    break;
  }

  case AST_VAR_DECL: {
    // 变量声明：如果有初始化，生成赋值
    if (node->data.var_decl.initializer) {
      IROperand value =
          translate_expression(program, node->data.var_decl.initializer);
      IROperand var = ir_operand_var(node->data.var_decl.name);
      ir_emit(program, IR_ASSIGN, var, value, ir_operand_none());
    }
    break;
  }

  case AST_EXPR_STMT: {
    translate_expression(program, node->data.expr_stmt.expression);
    break;
  }

  case AST_IF_STMT: {
    // if (cond) then_branch else else_branch
    //
    // 生成代码：
    //   <翻译 cond>
    //   iffalse cond goto L_else
    //   <翻译 then_branch>
    //   goto L_end
    // L_else:
    //   <翻译 else_branch>
    // L_end:

    int label_else = ir_new_label(program);
    int label_end = ir_new_label(program);

    // 翻译条件
    IROperand cond =
        translate_expression(program, node->data.if_stmt.condition);

    // 条件为假跳到 else
    ir_emit(program, IR_IFFALSE, ir_operand_label(label_else), cond,
            ir_operand_none());

    // then 分支
    translate_statement(program, node->data.if_stmt.then_branch);

    if (node->data.if_stmt.else_branch) {
      // 有 else 分支：跳过它
      ir_emit(program, IR_GOTO, ir_operand_label(label_end), ir_operand_none(),
              ir_operand_none());

      // else 标签
      ir_emit(program, IR_LABEL, ir_operand_label(label_else),
              ir_operand_none(), ir_operand_none());

      // else 分支
      translate_statement(program, node->data.if_stmt.else_branch);

      // 结束标签
      ir_emit(program, IR_LABEL, ir_operand_label(label_end), ir_operand_none(),
              ir_operand_none());
    } else {
      // 无 else 分支
      ir_emit(program, IR_LABEL, ir_operand_label(label_else),
              ir_operand_none(), ir_operand_none());
    }
    break;
  }

  case AST_WHILE_STMT: {
    // while (cond) body
    //
    // 生成代码：
    // L_start:
    //   <翻译 cond>
    //   iffalse cond goto L_end
    //   <翻译 body>
    //   goto L_start
    // L_end:

    int label_start = ir_new_label(program);
    int label_end = ir_new_label(program);

    // 循环开始标签
    ir_emit(program, IR_LABEL, ir_operand_label(label_start), ir_operand_none(),
            ir_operand_none());

    // 翻译条件
    IROperand cond =
        translate_expression(program, node->data.while_stmt.condition);

    // 条件为假跳出循环
    ir_emit(program, IR_IFFALSE, ir_operand_label(label_end), cond,
            ir_operand_none());

    // 循环体
    translate_statement(program, node->data.while_stmt.body);

    // 跳回循环开始
    ir_emit(program, IR_GOTO, ir_operand_label(label_start), ir_operand_none(),
            ir_operand_none());

    // 循环结束标签
    ir_emit(program, IR_LABEL, ir_operand_label(label_end), ir_operand_none(),
            ir_operand_none());
    break;
  }

  case AST_RETURN_STMT: {
    if (node->data.return_stmt.value) {
      IROperand value =
          translate_expression(program, node->data.return_stmt.value);
      ir_emit(program, IR_RETURN, ir_operand_none(), value, ir_operand_none());
    } else {
      ir_emit(program, IR_RETURN, ir_operand_none(), ir_operand_none(),
              ir_operand_none());
    }
    break;
  }

  default:
    break;
  }
}

/**
 * 翻译函数定义
 */
static void translate_function(IRProgram *program, ASTNode *node) {
  if (!node || node->type != AST_FUNC_DECL)
    return;

  // 函数开始
  ir_emit(program, IR_FUNC_BEGIN, ir_operand_func(node->data.func_decl.name),
          ir_operand_none(), ir_operand_none());

  // 翻译函数体
  if (node->data.func_decl.body) {
    translate_statement(program, node->data.func_decl.body);
  }

  // 函数结束
  ir_emit(program, IR_FUNC_END, ir_operand_func(node->data.func_decl.name),
          ir_operand_none(), ir_operand_none());
}

/**
 * 翻译整个程序
 */
IRProgram *ir_generate(ASTNode *ast) {
  if (!ast || ast->type != AST_PROGRAM)
    return NULL;

  IRProgram *program = ir_program_create();

  // 翻译每个顶层声明
  for (int i = 0; i < ast->data.program.count; i++) {
    ASTNode *decl = ast->data.program.declarations[i];
    if (decl->type == AST_FUNC_DECL) {
      translate_function(program, decl);
    } else if (decl->type == AST_VAR_DECL) {
      translate_statement(program, decl);
    }
  }

  return program;
}

// ========== 打印 IR ==========

const char *ir_opcode_to_string(IROpcode op) {
  switch (op) {
  case IR_ASSIGN:
    return "ASSIGN";
  case IR_ADD:
    return "ADD";
  case IR_SUB:
    return "SUB";
  case IR_MUL:
    return "MUL";
  case IR_DIV:
    return "DIV";
  case IR_MOD:
    return "MOD";
  case IR_NEG:
    return "NEG";
  case IR_EQ:
    return "EQ";
  case IR_NE:
    return "NE";
  case IR_LT:
    return "LT";
  case IR_GT:
    return "GT";
  case IR_LE:
    return "LE";
  case IR_GE:
    return "GE";
  case IR_AND:
    return "AND";
  case IR_OR:
    return "OR";
  case IR_NOT:
    return "NOT";
  case IR_LABEL:
    return "LABEL";
  case IR_GOTO:
    return "GOTO";
  case IR_IF:
    return "IF";
  case IR_IFFALSE:
    return "IFFALSE";
  case IR_FUNC_BEGIN:
    return "FUNC_BEGIN";
  case IR_FUNC_END:
    return "FUNC_END";
  case IR_PARAM:
    return "PARAM";
  case IR_CALL:
    return "CALL";
  case IR_RETURN:
    return "RETURN";
  case IR_NOP:
    return "NOP";
  default:
    return "???";
  }
}

static void print_operand(IROperand op) {
  switch (op.type) {
  case OPERAND_NONE:
    break;
  case OPERAND_TEMP:
    printf("t%d", op.value.temp_id);
    break;
  case OPERAND_VAR:
  case OPERAND_FUNC:
    printf("%s", op.value.name);
    break;
  case OPERAND_INT:
    printf("%d", op.value.int_val);
    break;
  case OPERAND_FLOAT:
    printf("%.2f", op.value.float_val);
    break;
  case OPERAND_LABEL:
    printf("L%d", op.value.label_id);
    break;
  }
}

void ir_print(IRProgram *program) {
  if (!program)
    return;

  printf("IR Instructions (%d total):\n", program->count);
  printf("========================================\n");

  for (int i = 0; i < program->count; i++) {
    IRInstruction *instr = &program->instructions[i];

    printf("%4d: ", i);

    switch (instr->opcode) {
    case IR_LABEL:
      print_operand(instr->result);
      printf(":\n");
      break;

    case IR_GOTO:
      printf("goto ");
      print_operand(instr->result);
      printf("\n");
      break;

    case IR_IF:
      printf("if ");
      print_operand(instr->arg1);
      printf(" goto ");
      print_operand(instr->result);
      printf("\n");
      break;

    case IR_IFFALSE:
      printf("iffalse ");
      print_operand(instr->arg1);
      printf(" goto ");
      print_operand(instr->result);
      printf("\n");
      break;

    case IR_FUNC_BEGIN:
      printf("function ");
      print_operand(instr->result);
      printf(":\n");
      break;

    case IR_FUNC_END:
      printf("end function ");
      print_operand(instr->result);
      printf("\n\n");
      break;

    case IR_PARAM:
      printf("param ");
      print_operand(instr->arg1);
      printf("\n");
      break;

    case IR_CALL:
      print_operand(instr->result);
      printf(" = call ");
      print_operand(instr->arg1);
      printf(", %d\n", instr->arg_count);
      break;

    case IR_RETURN:
      printf("return");
      if (instr->arg1.type != OPERAND_NONE) {
        printf(" ");
        print_operand(instr->arg1);
      }
      printf("\n");
      break;

    case IR_ASSIGN:
      print_operand(instr->result);
      printf(" = ");
      print_operand(instr->arg1);
      printf("\n");
      break;

    case IR_NEG:
    case IR_NOT:
      print_operand(instr->result);
      printf(" = %s ", instr->opcode == IR_NEG ? "-" : "!");
      print_operand(instr->arg1);
      printf("\n");
      break;

    default:
      // 二元运算
      print_operand(instr->result);
      printf(" = ");
      print_operand(instr->arg1);
      printf(" %s ", ir_opcode_to_string(instr->opcode));
      print_operand(instr->arg2);
      printf("\n");
      break;
    }
  }

  printf("========================================\n");
}
