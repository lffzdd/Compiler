#include "keyword.c"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef enum {
  TOKEN_KEYWORD,
  TOKEN_OPERATOR,
  TOKEN_IDENTIFIER,

  TOKEN_INTEGER,
  TOKEN_FLOAT,
  TOKEN_CHAR,
  TOKEN_STRING,
  TOKEN_BOOL,

  TOKEN_LEFT_PAREN,
  TOKEN_RIGHT_PAREN,
  TOKEN_LEFT_BRACE,
  TOKEN_RIGHT_BRACE,
  TOKEN_LEFT_BRACKET,
  TOKEN_RIGHT_BRACKET,

  TOKEN_SEMICOLON,
  TOKEN_EOF,
  TOKEN_UNKNOWN
  } TokenType;

typedef struct {
  TokenType type;
  char value[1024];
  } Token;

Token getNextToken(const char* src, int* pos) {
  Token token;
  memset(token.value, 0, sizeof(token.value));

  // 1.跳过空格和制表符
  while (isspace(src[*pos])) {
    (*pos)++;
    }

  // 2.检查是否结束
  if (src[*pos] == '\0') {
    token.type = TOKEN_EOF;
    return token;
    }

  // 3.识别标识符、关键字和布尔值
  if (isalpha(src[*pos]) || src[*pos] == '_') {
    int i = 0;
    while (isalnum(src[*pos]) || src[*pos] == '_') {
      token.value[i++] = src[(*pos)++];
      }

    // 检查是否为布尔值
    if (strcmp(token.value, "true") == 0 || strcmp(token.value, "false") == 0) {
      token.type = TOKEN_BOOL;
      } else if (isKeyword(token.value)) {
      token.type = TOKEN_KEYWORD;
      } else {
      token.type = TOKEN_IDENTIFIER;
      }

    return token;
    }

  // 4.识别操作符,先检查多字符，再检查单字符
  if (strncmp(&src[*pos], "==", 2) == 0 || strncmp(&src[*pos], "!=", 2) == 0 ||
    strncmp(&src[*pos], "<=", 2) == 0 || strncmp(&src[*pos], ">=", 2) == 0) {
    token.type = TOKEN_OPERATOR;
    token.value[0] = src[*pos];
    token.value[1] = src[*pos + 1];
    token.value[2] = '\0';
    *pos += 2;
    return token;
    } else if (strchr("+-*/<>%=", src[*pos])) {
      token.type = TOKEN_OPERATOR;
      token.value[0] = src[(*pos)++];
      return token;
      }

    // 5.识别数字(整数和浮点数)
    if (isdigit(src[*pos])) {
      int i = 0;
      token.type = TOKEN_INTEGER; // 默认为整数
      
      // 读取数字部分
      while (isdigit(src[*pos]) && i < sizeof(token.value) - 1) {
        token.value[i++] = src[(*pos)++];
        }
      
      // 检查是否有小数点
      if (src[*pos] == '.' && isdigit(src[*pos + 1])) {
        token.type = TOKEN_FLOAT;
        token.value[i++] = src[(*pos)++]; // 添加小数点
        
        // 读取小数部分
        while (isdigit(src[*pos]) && i < sizeof(token.value) - 1) {
          token.value[i++] = src[(*pos)++];
          }
        }
      
      return token;
      }

    // 6.识别字符字面量 'a'
    if (src[*pos] == '\'') {
      token.type = TOKEN_CHAR;
      int i = 0;
      token.value[i++] = src[(*pos)++]; // 添加开始的单引号
      
      // 处理转义字符
      if (src[*pos] == '\\' && src[*pos + 1] != '\0') {
        token.value[i++] = src[(*pos)++]; // 反斜杠
        token.value[i++] = src[(*pos)++]; // 转义字符
        } else if (src[*pos] != '\'' && src[*pos] != '\0') {
        token.value[i++] = src[(*pos)++]; // 普通字符
        }
      
      // 添加结束的单引号
      if (src[*pos] == '\'') {
        token.value[i++] = src[(*pos)++];
        }
      
      return token;
      }

    // 7.识别字符串字面量 "hello"
    if (src[*pos] == '"') {
      token.type = TOKEN_STRING;
      int i = 0;
      token.value[i++] = src[(*pos)++]; // 添加开始的双引号
      
      // 读取字符串内容直到结束引号或文件结束
      while (src[*pos] != '"' && src[*pos] != '\0' && i < sizeof(token.value) - 2) {
        if (src[*pos] == '\\' && src[*pos + 1] != '\0') {
          token.value[i++] = src[(*pos)++]; // 反斜杠
          token.value[i++] = src[(*pos)++]; // 转义字符
          } else {
          token.value[i++] = src[(*pos)++];
          }
        }
      
      // 添加结束的双引号
      if (src[*pos] == '"') {
        token.value[i++] = src[(*pos)++];
        }
      
      return token;
      }

    // 8.识别括号
    if (strchr("(){}[]", src[*pos])) {
      char c = src[*pos];
      token.value[0] = c;
      switch (c) {
          case '(': token.type = TOKEN_LEFT_PAREN; break;
          case ')': token.type = TOKEN_RIGHT_PAREN; break;
          case '{': token.type = TOKEN_LEFT_BRACE; break;
          case '}': token.type = TOKEN_RIGHT_BRACE; break;
          case '[': token.type = TOKEN_LEFT_BRACKET; break;
          case ']': token.type = TOKEN_RIGHT_BRACKET; break;
          default:  token.type = TOKEN_UNKNOWN; break;
        }
      (*pos)++;
      return token;
      }

    // 9.识别分号
    if (src[*pos] == ';') {
      token.type = TOKEN_SEMICOLON;
      token.value[0] = ';';
      (*pos)++;
      return token;
      }

    // 10.识别未知字符
    token.type = TOKEN_UNKNOWN;
    token.value[0] = src[*pos];
    (*pos)++;
    return token;
  }

const char* tokenTypeToString(TokenType type) {
  switch (type) {
      case TOKEN_KEYWORD:
        return "KEYWORD";
      case TOKEN_OPERATOR:
        return "OPERATOR";
      case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
      case TOKEN_INTEGER:
        return "INTEGER";
      case TOKEN_FLOAT:
        return "FLOAT";
      case TOKEN_CHAR:
        return "CHAR";
      case TOKEN_STRING:
        return "STRING";
      case TOKEN_BOOL:
        return "BOOL";
      case TOKEN_LEFT_PAREN:
        return "LEFT_PAREN";
      case TOKEN_RIGHT_PAREN:
        return "RIGHT_PAREN";
      case TOKEN_LEFT_BRACE:
        return "LEFT_BRACE";
      case TOKEN_RIGHT_BRACE:
        return "RIGHT_BRACE";
      case TOKEN_LEFT_BRACKET:
        return "LEFT_BRACKET";
      case TOKEN_RIGHT_BRACKET:
        return "RIGHT_BRACKET";
      case TOKEN_SEMICOLON:
        return "SEMICOLON";
      case TOKEN_EOF:
        return "EOF";
      case TOKEN_UNKNOWN:
        return "UNKNOWN";
      default:
        return "UNKNOWN";
    }
  }

int lexer(const char* src) {
  int pos = 0;
  Token token = getNextToken(src, &pos);

  // 打印Token类型和值
  while (token.type != TOKEN_EOF) {
    printf("Token Type: %s,\tValue: %s\n", tokenTypeToString(token.type), token.value);

    token = getNextToken(src, &pos);
    }
  return 0;
  }

int main(int argc, char* argv[]) {
  // 接收命令行参数文件路径
  const char* file_path = argv[1];
  // 打开文件并读取内容
  FILE* file;
  fopen_s(&file, file_path, "r");
  if (!file) {
    fprintf(stderr, "Failed to open file: %s\n", file_path);
    return 1;
    }

  // 读取文件内容
  char src[8192];
  memset(src, 0, sizeof(src));

  size_t bytes_read = fread(src, sizeof(char), sizeof(src), file);
  src[bytes_read] = '\0';

  lexer(src);
  fclose(file);
  return 0;
  }
