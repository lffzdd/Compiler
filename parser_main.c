#include "include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <source_file> [--debug]\n", argv[0]);
        fprintf(stderr, "  --debug: Enable debug mode to show parsing process\n");
        return 1;
    }

    // 检查是否启用调试模式
    int debug = 0;
    if (argc == 3 && strcmp(argv[2], "--debug") == 0) {
        debug = 1;
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

    // 根据参数选择调用parse还是parse_debug
    if (debug) {
        parse_debug(src);
    }
    else {
        parse(src);
    }

    fclose(file);
    return 0;
}