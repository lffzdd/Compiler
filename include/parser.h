/**
 * parser.h - 语法分析器头文件
 *
 * 语法分析器（Parser）的职责：
 * 将 Token 序列解析成抽象语法树（AST）
 *
 * 我们使用"递归下降"解析方法：
 * - 为每个语法规则写一个解析函数
 * - 函数之间相互调用形成递归
 */

#ifndef PARSER_H
#define PARSER_H

#include "ast.h"
#include "lexer.h"


/**
 * Parser 结构体
 *
 * 为什么需要这个结构体？
 * - 保存 Lexer 的引用
 * - 缓存当前 Token（避免重复读取）
 * - 记录错误信息
 */
typedef struct {
  Lexer *lexer;   // 词法分析器
  Token current;  // 当前 Token
  Token previous; // 上一个 Token（用于错误报告）
  int had_error;  // 是否发生错误
  int panic_mode; // 错误恢复模式
} Parser;

/**
 * parser_init - 初始化语法分析器
 * @lexer: 词法分析器指针
 *
 * 返回: 初始化好的 Parser 结构体
 */
Parser parser_init(Lexer *lexer);

/**
 * parser_parse - 解析整个程序
 * @parser: 语法分析器指针
 *
 * 返回: 程序的 AST 根节点
 *
 * 这是语法分析器的主入口！
 * 调用后会返回整个程序的抽象语法树。
 */
ASTNode *parser_parse(Parser *parser);

/**
 * parser_had_error - 检查是否有语法错误
 * @parser: 语法分析器指针
 *
 * 返回: 1 如果有错误，0 如果没有
 */
int parser_had_error(Parser *parser);

#endif // PARSER_H
