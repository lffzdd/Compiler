/**
 * main.c - 编译器主程序
 *
 * 演示编译器前端的四个阶段：
 * 1. 词法分析 (Lexer)
 * 2. 语法分析 (Parser)
 * 3. 语义分析 (Semantic)
 * 4. 中间代码生成 (IR)
 */

#include "include/ast.h"
#include "include/ir.h"
#include "include/lexer.h"
#include "include/parser.h"
#include "include/semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**
 * 读取文件内容
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
 * 编译流程（所有阶段）
 */
void compile(const char *source, int show_tokens, int show_ast, int show_ir) {
  printf("\n========== Source Code ==========\n");
  printf("%s", source);
  printf("=================================\n\n");

  // 阶段1: 词法分析
  if (show_tokens) {
    printf("========== Phase 1: Lexical Analysis ==========\n");
    Lexer temp_lexer = lexer_init(source);
    Token token;
    do {
      token = lexer_next_token(&temp_lexer);
      print_token(token);
    } while (token.type != TOKEN_EOF);
    printf("================================================\n\n");
  }

  // 阶段2: 语法分析
  printf("========== Phase 2: Syntax Analysis ==========\n");
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
  printf("==============================================\n\n");

  // 阶段3: 语义分析
  printf("========== Phase 3: Semantic Analysis ==========\n");
  SemanticAnalyzer *analyzer = semantic_init();
  semantic_analyze(analyzer, ast);

  if (semantic_has_errors(analyzer)) {
    printf("Semantic analysis FAILED.\n\n");
    semantic_print_errors(analyzer);
    semantic_free(analyzer);
    ast_free(ast);
    return;
  }
  printf("Semantic analysis successful!\n");
  printf("================================================\n\n");

  // 阶段4: 中间代码生成
  printf("========== Phase 4: IR Generation ==========\n");
  IRProgram *ir = ir_generate(ast);
  printf("IR generation successful! (%d instructions)\n", ir->count);

  if (show_ir) {
    printf("\n");
    ir_print(ir);
  }
  printf("============================================\n");

  // 清理
  ir_program_free(ir);
  semantic_free(analyzer);
  ast_free(ast);
}

/**
 * 演示程序
 */
void demo(void) {
  printf("================================================\n");
  printf("    Compiler Demo - All 4 Frontend Phases\n");
  printf("================================================\n");

  const char *program = "int add(int a, int b) {\n"
                        "    return a + b;\n"
                        "}\n"
                        "\n"
                        "int main() {\n"
                        "    int x = 5;\n"
                        "    int y = 10;\n"
                        "    int sum = add(x, y);\n"
                        "    \n"
                        "    if (sum > 10) {\n"
                        "        return 1;\n"
                        "    }\n"
                        "    return 0;\n"
                        "}\n";

  compile(program, 0, 0, 1); // 显示 IR
}

/**
 * 测试不同的 IR 生成场景
 */
void test_ir(void) {
  printf("================================================\n");
  printf("    IR Generation Test Cases\n");
  printf("================================================\n");

  struct {
    const char *name;
    const char *code;
  } tests[] = {{"Simple Arithmetic", "int main() {\n"
                                     "    int x = 1 + 2 * 3;\n"
                                     "    return x;\n"
                                     "}\n"},
               {"If Statement", "int main() {\n"
                                "    int x = 5;\n"
                                "    if (x > 0) {\n"
                                "        x = 1;\n"
                                "    } else {\n"
                                "        x = 0;\n"
                                "    }\n"
                                "    return x;\n"
                                "}\n"},
               {"While Loop", "int main() {\n"
                              "    int i = 0;\n"
                              "    int sum = 0;\n"
                              "    while (i < 10) {\n"
                              "        sum = sum + i;\n"
                              "        i = i + 1;\n"
                              "    }\n"
                              "    return sum;\n"
                              "}\n"},
               {"Function Call", "int square(int n) {\n"
                                 "    return n * n;\n"
                                 "}\n"
                                 "int main() {\n"
                                 "    int x = square(5);\n"
                                 "    return x;\n"
                                 "}\n"},
               {NULL, NULL}};

  for (int i = 0; tests[i].name != NULL; i++) {
    printf("\n--- Test: %s ---\n", tests[i].name);
    compile(tests[i].code, 0, 0, 1);
  }
}

void print_usage(const char *prog) {
  printf("Usage: %s [options] [file]\n", prog);
  printf("\nOptions:\n");
  printf("  -t, --tokens    Show token stream\n");
  printf("  -a, --ast       Show AST\n");
  printf("  -i, --ir        Show IR code\n");
  printf("  --test          Run IR test cases\n");
  printf("  -h, --help      Show this help\n");
}

int main(int argc, char *argv[]) {
  int show_tokens = 0;
  int show_ast = 0;
  int show_ir = 1; // 默认显示 IR
  const char *filename = NULL;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tokens") == 0) {
      show_tokens = 1;
    } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--ast") == 0) {
      show_ast = 1;
    } else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--ir") == 0) {
      show_ir = 1;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "--test") == 0) {
      test_ir();
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
    compile(source, show_tokens, show_ast, show_ir);

    free(source);
  } else {
    demo();
  }

  return 0;
}
