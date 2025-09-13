#include "include/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern Token currentToken;
extern int pos;
extern const char* src;

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
    static char source[8192];
    memset(source, 0, sizeof(source));

    size_t bytes_read = fread(source, sizeof(char), sizeof(source), file);
    source[bytes_read] = '\0';

    // 设置全局源码指针
    src = source;
    pos = 0;

    printf("=== Parsing ===\n");

    // 获取第一个token
    nextToken();

    // 开始解析
    parseProgram();

    printf("Parse completed successfully!\n");

    fclose(file);
    return 0;
}
