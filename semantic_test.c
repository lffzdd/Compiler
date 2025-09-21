#include "../include/semantic.h"
#include "../include/parser.h"
#include "../include/lexer.h"
#include <stdio.h>
#include <stdlib.h>

// 测试用例结构
typedef struct {
    const char* name;
    const char* code;
    bool should_pass;
    const char* description;
} SemanticTestCase;

// 测试用例数组
SemanticTestCase test_cases[] = {
    // 测试1: 基本的变量声明和使用
    {
        "Basic Variable Declaration",
        "int main() {\n"
        "    int x = 5;\n"
        "    int y;\n"
        "    y = x + 10;\n"
        "    return 0;\n"
        "}\n",
        true,
        "测试基本变量声明、初始化和赋值"
    },
    
    // 测试2: 类型不匹配错误
    {
        "Type Mismatch Error",
        "int main() {\n"
        "    int x = 5;\n"
        "    string y = x;\n"
        "    return 0;\n"
        "}\n",
        false,
        "测试类型不匹配的错误检测"
    },
    
    // 测试3: 未定义变量错误
    {
        "Undefined Variable Error",
        "int main() {\n"
        "    int x = 5;\n"
        "    y = x + 10;\n"
        "    return 0;\n"
        "}\n",
        false,
        "测试使用未定义变量的错误检测"
    },
    
    // 测试4: 函数声明和调用
    {
        "Function Declaration and Call",
        "int add(int a, int b);\n"
        "int main() {\n"
        "    int result = add(5, 10);\n"
        "    return result;\n"
        "}\n"
        "int add(int a, int b) {\n"
        "    return a + b;\n"
        "}\n",
        true,
        "测试函数声明、定义和调用"
    },
    
    // 测试5: 函数参数不匹配错误
    {
        "Function Parameter Mismatch",
        "int add(int a, int b);\n"
        "int main() {\n"
        "    int result = add(5);\n"
        "    return result;\n"
        "}\n",
        false,
        "测试函数调用参数数量不匹配的错误检测"
    },
    
    // 测试6: 重复声明错误
    {
        "Redeclaration Error",
        "int main() {\n"
        "    int x = 5;\n"
        "    int x = 10;\n"
        "    return 0;\n"
        "}\n",
        false,
        "测试重复声明变量的错误检测"
    },
    
    // 测试7: 返回类型不匹配
    {
        "Return Type Mismatch",
        "int getValue() {\n"
        "    return \"hello\";\n"
        "}\n"
        "int main() {\n"
        "    return 0;\n"
        "}\n",
        false,
        "测试返回类型不匹配的错误检测"
    },
    
    // 测试8: 作用域测试
    {
        "Scope Test",
        "int main() {\n"
        "    int x = 5;\n"
        "    {\n"
        "        int y = 10;\n"
        "        x = y + 5;\n"
        "    }\n"
        "    return x;\n"
        "}\n",
        true,
        "测试变量作用域的正确处理"
    },
    
    // 测试9: if语句条件类型检查
    {
        "If Statement Condition Type",
        "int main() {\n"
        "    int x = 5;\n"
        "    if (x > 0) {\n"
        "        x = x - 1;\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
        true,
        "测试if语句条件表达式的类型检查"
    },
    
    // 测试10: 缺少main函数
    {
        "Missing Main Function",
        "int add(int a, int b) {\n"
        "    return a + b;\n"
        "}\n",
        false,
        "测试缺少main函数的错误检测"
    }
};

const int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

void runSemanticTest(const SemanticTestCase* test_case) {
    printf("\n=== 测试: %s ===\n", test_case->name);
    printf("描述: %s\n", test_case->description);
    printf("预期结果: %s\n", test_case->should_pass ? "通过" : "失败");
    printf("测试代码:\n%s\n", test_case->code);
    
    // 解析代码生成AST
    ASTNode* ast = parse_and_build_ast(test_case->code);
    if (!ast) {
        printf("❌ 语法分析失败\n");
        return;
    }
    
    // 进行语义分析
    bool result = performSemanticAnalysis(ast);
    
    // 检查测试结果
    if (result == test_case->should_pass) {
        printf("✅ 测试通过\n");
    } else {
        printf("❌ 测试失败 - 期望 %s，实际 %s\n", 
               test_case->should_pass ? "成功" : "失败",
               result ? "成功" : "失败");
    }
    
    // 清理AST（假设有相应的函数）
    // destroyAST(ast);
}

void runAllSemanticTests() {
    printf("=== 语义分析器测试套件 ===\n");
    printf("总共 %d 个测试用例\n", num_test_cases);
    
    int passed = 0;
    int failed = 0;
    
    for (int i = 0; i < num_test_cases; i++) {
        runSemanticTest(&test_cases[i]);
        
        // 解析代码生成AST
        ASTNode* ast = parse_and_build_ast(test_cases[i].code);
        if (ast) {
            bool result = performSemanticAnalysis(ast);
            if (result == test_cases[i].should_pass) {
                passed++;
            } else {
                failed++;
            }
        } else {
            failed++;
        }
        
        printf("\n" "=" * 50 "\n");
    }
    
    printf("\n=== 测试结果汇总 ===\n");
    printf("通过: %d\n", passed);
    printf("失败: %d\n", failed);
    printf("总计: %d\n", passed + failed);
    printf("成功率: %.2f%%\n", (float)passed / (passed + failed) * 100);
}

int main() {
    printf("语义分析器测试程序\n");
    printf("==================\n\n");
    
    runAllSemanticTests();
    
    return 0;
}