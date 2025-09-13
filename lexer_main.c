#include "include/lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        return 1;
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

    printf("=== Lexical Analysis ===\n");
    lexer(src);

    fclose(file);
    return 0;
}
