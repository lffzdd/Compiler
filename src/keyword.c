#include "../include/keyword.h"
#include <string.h>

// 关键字数组
const char* keywords[] = {
    "int", "char", "float", "double", "void", "if", "else", "while",
    "for", "return", "break", "continue", "switch", "case", "default",
    "string", "bool",  // 添加自定义类型
    NULL // 结束标记
};

int isKeyword(const char* str) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcmp(str, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}
