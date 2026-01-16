/**
 * main.c - 词法分析器测试程序
 * 
 * 这个程序演示了词法分析器的使用方法
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "include/lexer.h"

/**
 * 读取文件内容到字符串
 */
char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        return NULL;
    }
    
    // 获取文件大小
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // 分配内存并读取
    char* content = (char*)malloc(size + 1);
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
 * 演示词法分析器的基本使用
 */
void demo_basic_lexer(void) {
    printf("========== 词法分析器演示 ==========\n\n");
    
    // 测试代码
    const char* source = 
        "// 这是一个简单的程序\n"
        "int main() {\n"
        "    int x = 42;\n"
        "    float pi = 3.14;\n"
        "    if (x > 10) {\n"
        "        return x + 1;\n"
        "    }\n"
        "    return 0;\n"
        "}\n";
    
    printf("源代码:\n");
    printf("----------------------------------------\n");
    printf("%s", source);
    printf("----------------------------------------\n\n");
    
    printf("Token 序列:\n");
    printf("----------------------------------------\n");
    
    // 初始化词法分析器
    Lexer lexer = lexer_init(source);
    
    // 循环获取所有 Token
    Token token;
    do {
        token = lexer_next_token(&lexer);
        print_token(token);
    } while (token.type != TOKEN_EOF);
    
    printf("----------------------------------------\n");
}

/**
 * 交互式测试模式
 */
void interactive_mode(void) {
    printf("========== 交互式词法分析器 ==========\n");
    printf("输入代码，按回车分析。输入 'quit' 退出。\n\n");
    
    char input[1024];
    
    while (1) {
        printf("> ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        // 去掉末尾换行符
        input[strcspn(input, "\n")] = '\0';
        
        if (strcmp(input, "quit") == 0) {
            printf("Bye!\n");
            break;
        }
        
        // 分析输入
        Lexer lexer = lexer_init(input);
        Token token;
        do {
            token = lexer_next_token(&lexer);
            print_token(token);
        } while (token.type != TOKEN_EOF);
        
        printf("\n");
    }
}

/**
 * 分析文件
 */
int analyze_file(const char* filename) {
    printf("分析文件: %s\n", filename);
    printf("========================================\n\n");
    
    char* source = read_file(filename);
    if (!source) {
        return 1;
    }
    
    printf("源代码:\n");
    printf("----------------------------------------\n");
    printf("%s", source);
    printf("----------------------------------------\n\n");
    
    printf("Token 序列:\n");
    printf("----------------------------------------\n");
    
    Lexer lexer = lexer_init(source);
    Token token;
    int token_count = 0;
    
    do {
        token = lexer_next_token(&lexer);
        print_token(token);
        token_count++;
    } while (token.type != TOKEN_EOF);
    
    printf("----------------------------------------\n");
    printf("共 %d 个 Token\n", token_count);
    
    free(source);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        // 如果提供了文件名，分析该文件
        return analyze_file(argv[1]);
    } else {
        // 否则运行演示
        demo_basic_lexer();
        printf("\n");
        interactive_mode();
    }
    
    return 0;
}
