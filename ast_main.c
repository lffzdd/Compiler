#include "include/parser.h"
#include "include/ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <source_file> [--ast]\n", argv[0]);
        fprintf(stderr, "  --ast: Show AST structure\n");
        return 1;
    }

    // 检查是否显示AST
    int show_ast = 0;
    if (argc == 3 && strcmp(argv[2], "--ast") == 0) {
        show_ast = 1;
    }

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

    if (show_ast) {
        printf("=== Building AST ===\n");
        ASTNode* ast = parse_and_build_ast(src);
        if (ast) {
            printf("=== AST Structure ===\n");
            printAST(ast, 0);
            printf("=== AST Built Successfully! ===\n");
            freeAST(ast);
        }
        else {
            printf("Failed to build AST!\n");
        }
    }
    else {
        printf("=== Parsing (Basic) ===\n");
        parse(src);
    }

    fclose(file);
    return 0;
}