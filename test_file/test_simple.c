// test_simple.c - Simple test for parser

int add(int a, int b) { return a + b; }

int main() {
  int x = 1 + 2 * 3;
  int y = add(x, 10);

  if (y > 15) {
    return 1;
  }

  return 0;
}
