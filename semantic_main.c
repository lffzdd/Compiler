#include "include/semantic.h"
#include "include/parser.h"
#include "include/lexer.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== 语义分析器演示 ===\n\n");
    
    // 测试代码示例
    const char* test_program = 
        "int add(int a, int b);\n"
        "\n"
        "int main() {\n"
        "    int x = 5;\n"
        "    int y = 10;\n"
        "    int result = add(x, y);\n"
        "    if (result > 10) {\n"
        "        return result;\n"
        "    } else {\n"
        "        return 0;\n"
        "    }\n"
        "}\n"
        "\n"
        "int add(int a, int b) {\n"
        "    return a + b;\n"
        "}\n";
    
    printf("待分析的程序:\n");
    printf("%s\n", test_program);
    
    // 进行语法分析生成AST
    printf("=== 开始语法分析 ===\n");
    ASTNode* ast = parse_and_build_ast(test_program);
    
    if (!ast) {
        printf("语法分析失败！\n");
        return 1;
    }
    
    printf("语法分析完成。\n\n");
    
    // 进行语义分析
    printf("=== 开始语义分析 ===\n");
    bool semantic_result = performSemanticAnalysis(ast);
    
    if (semantic_result) {
        printf("\n✅ 语义分析成功！程序语义正确。\n");
    } else {
        printf("\n❌ 语义分析失败！程序存在语义错误。\n");
    }
    
    // 测试一个有错误的程序
    printf("\n\n=== 测试错误检测 ===\n");
    const char* error_program = 
        "int main() {\n"
        "    int x = 5;\n"
        "    string y = x;\n"  // 类型不匹配错误
        "    z = 10;\n"        // 未定义变量错误
        "    return y;\n"      // 返回类型不匹配错误
        "}\n";
    
    printf("包含错误的程序:\n");
    printf("%s\n", error_program);
    
    ASTNode* error_ast = parse_and_build_ast(error_program);
    if (error_ast) {
        printf("=== 语义分析错误程序 ===\n");
        bool error_result = performSemanticAnalysis(error_ast);
        
        if (!error_result) {
            printf("\n✅ 错误检测成功！成功识别出程序中的语义错误。\n");
        } else {
            printf("\n❌ 错误检测失败！未能识别程序中的语义错误。\n");
        }
    }
    
    return 0;
}