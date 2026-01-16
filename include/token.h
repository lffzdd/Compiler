/**
 * token.h - Token 类型定义
 * 
 * Token（词法单元）是词法分析的输出。
 * 每个 Token 包含：
 *   - type: 表示这是什么类型的词（关键字？数字？运算符？）
 *   - value: 这个词的具体内容
 */

#ifndef TOKEN_H
#define TOKEN_H

/**
 * TokenType 枚举
 * 定义了我们的语言支持的所有 Token 类型
 */
typedef enum {
    // ========== 关键字和标识符 ==========
    TOKEN_KEYWORD,      // 关键字: int, if, while, return 等
    TOKEN_IDENTIFIER,   // 标识符: 变量名、函数名等

    // ========== 字面量 (Literals) ==========
    TOKEN_INTEGER,      // 整数: 42, 0, 123
    TOKEN_FLOAT,        // 浮点数: 3.14, 0.5
    TOKEN_CHAR,         // 字符: 'a', 'b'
    TOKEN_STRING,       // 字符串: "hello"

    // ========== 运算符 ==========
    TOKEN_OPERATOR,     // 运算符: +, -, *, /, =, <, >, ==, !=, <=, >=

    // ========== 分隔符 ==========
    TOKEN_LPAREN,       // 左圆括号: (
    TOKEN_RPAREN,       // 右圆括号: )
    TOKEN_LBRACE,       // 左花括号: {
    TOKEN_RBRACE,       // 右花括号: }
    TOKEN_LBRACKET,     // 左方括号: [
    TOKEN_RBRACKET,     // 右方括号: ]
    TOKEN_SEMICOLON,    // 分号: ;
    TOKEN_COMMA,        // 逗号: ,

    // ========== 特殊 ==========
    TOKEN_EOF,          // 文件结束
    TOKEN_UNKNOWN       // 未知字符（错误处理用）
} TokenType;

/**
 * Token 结构体
 * 
 * 为什么 value 是固定大小的数组？
 * - 简单：不需要动态内存管理
 * - 对于学习来说足够了
 * - 实际编译器可能会用指针+长度的方式
 */
#define MAX_TOKEN_LENGTH 256

typedef struct {
    TokenType type;                 // Token 的类型
    char value[MAX_TOKEN_LENGTH];   // Token 的值（字符串形式）
} Token;

/**
 * 辅助函数声明
 */
// 将 TokenType 转换为可读的字符串（用于调试）
const char* token_type_to_string(TokenType type);

// 打印一个 Token（用于调试）
void print_token(Token token);

#endif // TOKEN_H
