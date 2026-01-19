/**
 * main.c - 编译器主程序
 *
 * 演示词法分析、语法分析和语义分析
 */

#include "include/ast.h"
#include "include/lexer.h"
#include "include/parser.h"
#include "include/semantic.h"
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

  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

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
 * 完整的编译流程
 */
void compile(const char *source, int show_tokens, int show_ast) {
  printf("\n");
  printf("========== Source Code ==========\n");
  printf("%s", source);
  printf("=================================\n\n");

  // 阶段1: 词法分析
  if (show_tokens) {
    printf("========== Lexical Analysis ==========\n");
    Lexer temp_lexer = lexer_init(source);
    Token token;
    do {
      token = lexer_next_token(&temp_lexer);
      print_token(token);
    } while (token.type != TOKEN_EOF);
    printf("======================================\n\n");
  }

  // 阶段2: 语法分析
  printf("========== Syntax Analysis ==========\n");
  Lexer lexer = lexer_init(source);
  Parser parser = parser_init(&lexer);
  ASTNode *ast = parser_parse(&parser);

  if (parser_had_error(&parser)) {
    printf("Parsing FAILED.\n");
    ast_free(ast);
    return;
  }
  printf("Parsing successful!\n");

  if (show_ast) {
    printf("\nAbstract Syntax Tree:\n");
    ast_print(ast, 0);
  }
  printf("=====================================\n\n");

  // 阶段3: 语义分析
  printf("========== Semantic Analysis ==========\n");
  SemanticAnalyzer *analyzer = semantic_init();
  semantic_analyze(analyzer, ast);

  if (semantic_has_errors(analyzer)) {
    printf("Semantic analysis FAILED.\n\n");
    semantic_print_errors(analyzer);
  } else {
    printf("Semantic analysis successful!\n");
    printf("No semantic errors found.\n");
  }
  printf("=======================================\n");

  // 清理
  semantic_free(analyzer);
  ast_free(ast);
}

/**
 * 演示程序
 */
void demo(void) {
  printf("============================================\n");
  printf("    Compiler Demo - Phases 1, 2, 3\n");
  printf("============================================\n");

  // 正确的程序
  const char *correct_program = "int add(int a, int b) {\n"
                                "    return a + b;\n"
                                "}\n"
                                "\n"
                                "int main() {\n"
                                "    int x = 5;\n"
                                "    int y = 10;\n"
                                "    int result = add(x, y);\n"
                                "    return result;\n"
                                "}\n";

  printf("\n>>> Test 1: Correct Program\n");
  compile(correct_program, 0, 0);

  // 有语义错误的程序
  const char *error_program =
      "int main() {\n"
      "    int x = 5;\n"
      "    y = 10;           // Error: y not declared\n"
      "    int x = 20;       // Error: x already declared\n"
      "    return x;\n"
      "}\n";

  printf("\n>>> Test 2: Program with Semantic Errors\n");
  compile(error_program, 0, 0);

  // 类型错误
  const char *type_error_program =
      "int foo(int a) {\n"
      "    return a;\n"
      "}\n"
      "\n"
      "int main() {\n"
      "    int x = foo(1, 2, 3);  // Error: wrong argument count\n"
      "    return 0;\n"
      "}\n";

  printf("\n>>> Test 3: Function Call Errors\n");
  compile(type_error_program, 0, 0);
}

/**
 * 测试语义分析的各种情况
 */
void test_semantic(void) {
  printf("============================================\n");
  printf("    Semantic Analysis Test Cases\n");
  printf("============================================\n");

  struct {
    const char *name;
    const char *code;
  } tests[] = {
      {"Undeclared Variable", "int main() { x = 5; return 0; }\n"},
      {"Duplicate Declaration",
       "int main() { int x = 1; int x = 2; return 0; }\n"},
      {"Undeclared Function", "int main() { int x = foo(); return 0; }\n"},
      {"Wrong Argument Count", "int add(int a, int b) { return a + b; }\n"
                               "int main() { int x = add(1); return 0; }\n"},
      {"Correct Nested Scopes",
       "int main() {\n"
       "    int x = 1;\n"
       "    if (x > 0) {\n"
       "        int y = 2;\n" // y only visible in if block
       "        x = y;\n"     // x is visible from outer scope
       "    }\n"
       "    return x;\n"
       "}\n"},
      {"Shadow Variable (Legal)",
       "int x = 1;\n" // global x
       "int main() {\n"
       "    int x = 2;\n" // local x shadows global
       "    return x;\n"
       "}\n"},
      {NULL, NULL}};

  for (int i = 0; tests[i].name != NULL; i++) {
    printf("\n--- Test: %s ---\n", tests[i].name);
    compile(tests[i].code, 0, 0);
  }
}

void print_usage(const char *prog) {
  printf("Usage: %s [options] [file]\n", prog);
  printf("\nOptions:\n");
  printf("  -t, --tokens    Show token stream\n");
  printf("  -a, --ast       Show AST\n");
  printf("  --test          Run semantic test cases\n");
  printf("  -h, --help      Show this help\n");
}

int main(int argc, char *argv[]) {
  int show_tokens = 0;
  int show_ast = 0;
  const char *filename = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tokens") == 0) {
      show_tokens = 1;
    } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--ast") == 0) {
      show_ast = 1;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "--test") == 0) {
      test_semantic();
      return 0;
    } else {
      filename = argv[i];
    }
  }

  if (filename) {
    char *source = read_file(filename);
    if (!source)
      return 1;

    printf("Compiling: %s\n", filename);
    compile(source, show_tokens, show_ast);

    free(source);
  } else {
    demo();
  }

  return 0;
}
