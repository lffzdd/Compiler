#include "keyword.c"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef enum {
  TOKEN_KEYWORD,
  TOKEN_OPERATOR,
  TOKEN_IDENTIFIER,
  TOKEN_INTEGER,

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

Token getNextToken(const char *src, int *pos) {
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

  // 3.识别标识符和关键字
  if (isalpha(src[*pos]) || src[*pos] == '_') {
    int i = 0;
    while (isalnum(src[*pos]) || src[*pos] == '_') {
      token.value[i++] = src[(*pos)++];
    }

    if (isKeyword(token.value)) {
      token.type = TOKEN_KEYWORD;
    } else {
      token.type = TOKEN_IDENTIFIER;
    }

    return token;
  }

  // 4.识别操作符
  if (strchr("+-*/%=", src[*pos])) {
    token.type = TOKEN_OPERATOR;
    token.value[0] = src[(*pos)++];
    return token;
  }

  // 5.识别整数
  if (isdigit(src[*pos])) {
    token.type = TOKEN_INTEGER;
    int i = 0;
    while (isdigit(src[*pos])) {
      token.value[i++] = src[(*pos)++];
    }
    return token;
  }

  // 6.识别分号
  if (src[*pos] == ';') {
    token.type = TOKEN_SEMICOLON;
    token.value[0] = ';';
    (*pos)++;
    return token;
  }

  // 7.识别括号
  if (src[*pos] == '(') {
  
  }
  // 8.识别未知字符
  token.type = TOKEN_UNKNOWN;
  token.value[0] = src[*pos];
  (*pos)++;
  return token;
}

const char *tokenTypeToString(TokenType type) {
  switch (type) {
  case TOKEN_KEYWORD:
    return "KEYWORD";
  case TOKEN_OPERATOR:
    return "OPERATOR";
  case TOKEN_IDENTIFIER:
    return "IDENTIFIER";
  case TOKEN_INTEGER:
    return "INTEGER";
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

int lexer(const char *src) {
  int pos = 0;
  Token token = getNextToken(src, &pos);

  // 打印Token类型和值
  while (token.type != TOKEN_EOF) {
    printf("Token Type: %s,\tValue: %s\n", tokenTypeToString(token.type), token.value);

    token = getNextToken(src, &pos);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  // 接收命令行参数文件路径
  const char *file_path = argv[1];
  // 打开文件并读取内容
  FILE *file;
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
