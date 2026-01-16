/**
 * lexer.h - 词法分析器头文件
 * 
 * 词法分析器（Lexer）的职责：
 * 将源代码字符串分解为 Token 序列
 */

#ifndef LEXER_H
#define LEXER_H

#include "token.h"

/**
 * Lexer 结构体
 * 
 * 为什么需要这个结构体？
 * - 保存源代码的引用
 * - 记录当前读取位置
 * - 方便在多次调用 next_token 之间保持状态
 */
typedef struct {
    const char* source;  // 源代码字符串
    int pos;             // 当前读取位置（字符索引）
    int line;            // 当前行号（用于错误报告）
    int column;          // 当前列号（用于错误报告）
} Lexer;

/**
 * lexer_init - 初始化词法分析器
 * @source: 要分析的源代码字符串
 * 
 * 返回: 初始化好的 Lexer 结构体
 */
Lexer lexer_init(const char* source);

/**
 * lexer_next_token - 获取下一个 Token
 * @lexer: 词法分析器指针
 * 
 * 返回: 下一个 Token
 * 
 * 这是词法分析器的核心函数！
 * 每次调用，它会：
 * 1. 跳过空白字符和注释
 * 2. 识别并返回下一个 Token
 * 3. 更新 lexer 的位置
 */
Token lexer_next_token(Lexer* lexer);

/**
 * lexer_peek_token - 预览下一个 Token（不消耗）
 * @lexer: 词法分析器指针
 * 
 * 返回: 下一个 Token，但不改变 lexer 的状态
 * 
 * 这在语法分析时很有用，有时候需要"偷看"下一个 Token
 * 来决定走哪个分支。
 */
Token lexer_peek_token(Lexer* lexer);

#endif // LEXER_H
