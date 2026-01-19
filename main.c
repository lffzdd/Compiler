/**
 * main.c - 编译器主程序
 *
 * 演示词法分析器和语法分析器的使用
 */

#include "include/ast.h"
#include "include/lexer.h"
#include "include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
 * 读取文件内容到字符串
 */
char *read_file(const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
    return NULL;
  }

  // 获取文件大小
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  // 分配内存并读取
  char *content = (char *)malloc(size + 1);
  if (!content) {
    fclose(file);
    return NULL;
  }

  fread(content, 1, size, file);
  content[size] = '\0';

  fclose(file);
  return content;
}

/**
 * 演示词法分析
 */
void demo_lexer(const char *source) {
  printf("========== Lexical Analysis ==========\n\n");

  Lexer lexer = lexer_init(source);
  Token token;
  int count = 0;

  printf("Token Stream:\n");
  printf("%-4s %-14s %s\n", "#", "Type", "Value");
  printf("----------------------------------------\n");

  do {
    token = lexer_next_token(&lexer);
    printf("%-4d %-14s \"%s\"\n", ++count, token_type_to_string(token.type),
           token.value);
  } while (token.type != TOKEN_EOF);

  printf("----------------------------------------\n");
  printf("Total: %d tokens\n\n", count);
}

/**
 * 演示语法分析
 */
void demo_parser(const char *source) {
  printf("========== Syntax Analysis ==========\n\n");

  // 初始化词法分析器
  Lexer lexer = lexer_init(source);

  // 初始化语法分析器
  Parser parser = parser_init(&lexer);

  // 解析程序
  ASTNode *program = parser_parse(&parser);

  if (parser_had_error(&parser)) {
    printf("Parsing failed with errors.\n");
  } else {
    printf("Parsing successful!\n\n");
    printf("Abstract Syntax Tree:\n");
    printf("----------------------------------------\n");
    ast_print(program, 0);
    printf("----------------------------------------\n");
  }

  // 释放内存
  ast_free(program);
}

/**
 * 完整分析流程
 */
void analyze(const char *source, int show_tokens) {
  printf("\n");
  printf("Source Code:\n");
  printf("========================================\n");
  printf("%s", source);
  printf("========================================\n\n");

  if (show_tokens) {
    demo_lexer(source);
  }

  demo_parser(source);
}

/**
 * 演示程序
 */
void demo(void) {
  printf("============================================\n");
  printf("    Compiler Demo - Lexer & Parser\n");
  printf("============================================\n");

  const char *source = "// Example program\n"
                       "int factorial(int n) {\n"
                       "    if (n <= 1) {\n"
                       "        return 1;\n"
                       "    }\n"
                       "    return n * factorial(n - 1);\n"
                       "}\n"
                       "\n"
                       "int main() {\n"
                       "    int x = 5;\n"
                       "    int result = factorial(x);\n"
                       "    return result;\n"
                       "}\n";

  analyze(source, 1);
}

/**
 * 表达式测试
 */
void test_expressions(void) {
  printf("\n============================================\n");
  printf("    Expression Parsing Test\n");
  printf("============================================\n");

  const char *tests[] = {// 基本算术
                         "int main() { int x = 1 + 2 * 3; return x; }\n",

                         // 优先级测试
                         "int main() { int x = (1 + 2) * 3; return x; }\n",

                         // 比较和逻辑
                         "int main() { int x = 1; if (x > 0 && x < 10) { "
                         "return 1; } return 0; }\n",

                         // 函数调用
                         "int foo(int a, int b) { return a + b; }\n"
                         "int main() { return foo(1, 2); }\n",

                         NULL};

  for (int i = 0; tests[i] != NULL; i++) {
    printf("\n--- Test %d ---\n", i + 1);
    analyze(tests[i], 0);
  }
}

/**
 * 打印使用帮助
 */
void print_usage(const char *prog) {
  printf("Usage: %s [options] [file]\n", prog);
  printf("\nOptions:\n");
  printf("  -t, --tokens    Show token stream\n");
  printf("  -h, --help      Show this help\n");
  printf("\nExamples:\n");
  printf("  %s                    Run demo\n", prog);
  printf("  %s test.c             Parse file\n", prog);
  printf("  %s -t test.c          Parse file with token output\n", prog);
}

int main(int argc, char *argv[]) {
  int show_tokens = 0;
  const char *filename = NULL;

  // 解析命令行参数
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tokens") == 0) {
      show_tokens = 1;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "--test") == 0) {
      test_expressions();
      return 0;
    } else {
      filename = argv[i];
    }
  }

  if (filename) {
    // 分析文件
    char *source = read_file(filename);
    if (!source) {
      return 1;
    }

    printf("Analyzing file: %s\n", filename);
    analyze(source, show_tokens);

    free(source);
  } else {
    // 运行演示
    demo();
  }

  return 0;
}
