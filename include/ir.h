/**
 * ir.h - 中间代码 (IR) 定义
 *
 * 中间代码采用"三地址码"格式：
 *   result = arg1 op arg2
 *
 * 每条指令最多涉及三个地址（操作数）。
 */

#ifndef IR_H
#define IR_H

#include "ast.h"

/**
 * IR 操作码
 */
typedef enum {
  // 赋值和算术
  IR_ASSIGN, // result = arg1
  IR_ADD,    // result = arg1 + arg2
  IR_SUB,    // result = arg1 - arg2
  IR_MUL,    // result = arg1 * arg2
  IR_DIV,    // result = arg1 / arg2
  IR_MOD,    // result = arg1 % arg2
  IR_NEG,    // result = -arg1

  // 比较运算
  IR_EQ, // result = (arg1 == arg2)
  IR_NE, // result = (arg1 != arg2)
  IR_LT, // result = (arg1 < arg2)
  IR_GT, // result = (arg1 > arg2)
  IR_LE, // result = (arg1 <= arg2)
  IR_GE, // result = (arg1 >= arg2)

  // 逻辑运算
  IR_AND, // result = arg1 && arg2
  IR_OR,  // result = arg1 || arg2
  IR_NOT, // result = !arg1

  // 控制流
  IR_LABEL,   // label:
  IR_GOTO,    // goto label
  IR_IF,      // if arg1 goto label
  IR_IFFALSE, // iffalse arg1 goto label

  // 函数相关
  IR_FUNC_BEGIN, // 函数开始
  IR_FUNC_END,   // 函数结束
  IR_PARAM,      // param arg1 (传递参数)
  IR_CALL,       // result = call func, n (调用函数)
  IR_RETURN,     // return arg1

  // 特殊
  IR_NOP // 空操作
} IROpcode;

/**
 * 操作数类型
 */
typedef enum {
  OPERAND_NONE,   // 无操作数
  OPERAND_TEMP,   // 临时变量: t1, t2, ...
  OPERAND_VAR,    // 程序变量: x, y, ...
  OPERAND_INT,    // 整数常量: 42
  OPERAND_FLOAT,  // 浮点常量: 3.14
  OPERAND_STRING, // 字符串常量: "hello"
  OPERAND_LABEL,  // 标签: L1, L2, ...
  OPERAND_FUNC    // 函数名: main, add, ...
} OperandType;

/**
 * 操作数结构
 */
typedef struct {
  OperandType type;
  union {
    int temp_id;      // 临时变量编号
    char *name;       // 变量名/函数名
    int int_val;      // 整数值
    double float_val; // 浮点值
    int label_id;     // 标签编号
  } value;
} IROperand;

/**
 * IR 指令结构
 */
typedef struct {
  IROpcode opcode;  // 操作码
  IROperand result; // 结果操作数
  IROperand arg1;   // 第一个参数
  IROperand arg2;   // 第二个参数（可选）
  int arg_count;    // 函数调用时的参数数量
} IRInstruction;

/**
 * IR 程序（指令列表）
 */
typedef struct {
  IRInstruction *instructions; // 指令数组
  int count;                   // 指令数量
  int capacity;                // 数组容量
  int temp_counter;            // 临时变量计数器
  int label_counter;           // 标签计数器
} IRProgram;

// ========== 函数声明 ==========

// 创建和销毁
IRProgram *ir_program_create(void);
void ir_program_free(IRProgram *program);

// 生成 IR
IRProgram *ir_generate(ASTNode *ast);

// 辅助函数
IROperand ir_new_temp(IRProgram *program);
int ir_new_label(IRProgram *program);
void ir_emit(IRProgram *program, IROpcode op, IROperand result, IROperand arg1,
             IROperand arg2);

// 构造操作数
IROperand ir_operand_none(void);
IROperand ir_operand_temp(int id);
IROperand ir_operand_var(const char *name);
IROperand ir_operand_int(int val);
IROperand ir_operand_float(double val);
IROperand ir_operand_label(int id);
IROperand ir_operand_func(const char *name);

// 打印 IR（调试用）
void ir_print(IRProgram *program);
const char *ir_opcode_to_string(IROpcode op);

#endif // IR_H
