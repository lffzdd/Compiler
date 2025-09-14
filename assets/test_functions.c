// 函数声明
int add(int a, int b);
void print_hello();

// 函数定义
int add(int a, int b) {
    return a + b;
}

void print_hello() {
    int x = 5;
    return;
}

// 主函数
int main() {
    int result = add(3, 4);
    print_hello();
    return 0;
}