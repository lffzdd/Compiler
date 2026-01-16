// sample.c - 用于测试词法分析器的示例程序
// 这是一个单行注释

/*
 * 这是一个块注释
 * 可以跨越多行
 */

int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main() {
    int x = 5;
    float pi = 3.14159;
    char c = 'A';
    
    int result = factorial(x);
    
    if (result > 100) {
        return 1;
    } else {
        return 0;
    }
}
