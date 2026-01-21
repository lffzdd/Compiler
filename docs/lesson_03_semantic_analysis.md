# 📖 第三课：语义分析 (Semantic Analysis)

## 🎯 学习目标

通过本课学习，你将：

1. 理解语义分析在编译器中的作用
2. 掌握**符号表 (Symbol Table)** 的概念和实现
3. 理解**作用域 (Scope)** 管理
4. 学会进行**类型检查 (Type Checking)**

---

## 🤔 为什么需要语义分析？

### 回顾：语法分析给了我们什么？

语法分析确保代码**语法正确**，生成了 AST。

但是，语法正确 ≠ 程序有意义！

### 语法正确但语义错误的例子

```c
int x = 5;
int x = 10;        // ❌ 错误：变量重复声明

y = 20;            // ❌ 错误：变量 y 未声明

int a = "hello";   // ❌ 错误：类型不匹配

int add(int x) { return x; }
add(1, 2, 3);      // ❌ 错误：参数数量不对
```

这些代码**语法上都是对的**（Parser 不会报错），但**语义上是错的**。

### 语义分析的职责

| 检查项   | 说明                     |
| :------- | :----------------------- |
| 变量声明 | 使用前必须声明           |
| 重复声明 | 同一作用域不能重复声明   |
| 类型检查 | 操作数类型必须兼容       |
| 函数调用 | 参数数量和类型必须匹配   |
| 返回值   | 返回类型必须匹配函数声明 |

---

## 📚 符号表 (Symbol Table)

### 什么是符号表？

符号表是一个**记录本**，记录程序中所有声明的变量和函数。

```
符号表:
┌──────────┬────────┬─────────┐
│ 名称     │ 类型   │ 作用域   │
├──────────┼────────┼─────────┤
│ x        │ int    │ main    │
│ pi       │ float  │ main    │
│ add      │ func   │ global  │
│ factorial│ func   │ global  │
└──────────┴────────┴─────────┘
```

### 符号表的作用

1. **查找**：使用变量时，检查它是否已声明
2. **记录**：声明变量时，记录其类型信息
3. **检测重复**：防止同一作用域重复声明

---

## 🔭 作用域 (Scope)

### 什么是作用域？

作用域是变量的**可见范围**。

```c
int x = 1;           // x 的作用域：全局

void foo() {
    int y = 2;       // y 的作用域：foo 函数内
  
    if (true) {
        int z = 3;   // z 的作用域：if 块内
        // 这里可以访问 x, y, z
    }
    // 这里可以访问 x, y，但不能访问 z
}
// 这里只能访问 x
```

### 作用域的嵌套

作用域形成**栈结构**：

```
进入 main() → 压入新作用域
  进入 if {} → 压入新作用域
    ...
  退出 if → 弹出作用域
退出 main() → 弹出作用域
```

### 变量查找规则

从**内向外**查找：

1. 先在当前作用域找
2. 找不到就去父作用域找
3. 一直找到全局作用域
4. 还找不到 → 报错"未定义"

---

## 🔍 类型检查 (Type Checking)

### 类型系统

我们支持的类型：

| 类型                                                    | 说明               |
| :------------------------------------------------------ | :----------------- |
| [int](file:///d:/Projects/C/Compiler/src/ast.c#214-318)    | 整数               |
| [float](file:///d:/Projects/C/Compiler/src/ast.c#202-209)  | 浮点数             |
| [char](file:///d:/Projects/C/Compiler/src/lexer.c#257-292) | 字符               |
| `void`                                                | 无返回值（函数用） |

### 类型兼容规则

```c
int + int    → int      ✅
int + float  → float    ✅ (int 隐式转换为 float)
int + char   → int      ✅ (char 隐式转换为 int)

int = float  → ⚠️ 警告 (可能丢失精度)
int = "str"  → ❌ 错误 (不兼容)
```

### 表达式类型推导

```c
int x = 5;
float y = 3.14;
??? z = x + y;    // z 应该是什么类型？
```

推导过程：

1. `x` 的类型是 [int](file:///d:/Projects/C/Compiler/src/ast.c#214-318)
2. `y` 的类型是 [float](file:///d:/Projects/C/Compiler/src/ast.c#202-209)
3. `int + float` 的结果是 [float](file:///d:/Projects/C/Compiler/src/ast.c#202-209)
4. 所以 `z` 应该是 [float](file:///d:/Projects/C/Compiler/src/ast.c#202-209)

---

## 🏗️ 实现架构

```
         AST
          │
          ▼
    ┌───────────────┐
    │ 语义分析器    │
    │               │
    │ ┌───────────┐ │
    │ │ 符号表    │ │
    │ └───────────┘ │
    │ ┌───────────┐ │
    │ │ 作用域栈  │ │
    │ └───────────┘ │
    │ ┌───────────┐ │
    │ │ 类型检查  │ │
    │ └───────────┘ │
    └───────────────┘
          │
          ▼
    带类型信息的 AST
    + 语义错误报告
```

---

## 💻 开始编码

我们将创建：

1. [include/semantic.h](file:///d:/Projects/C/Compiler/include/semantic.h) - 语义分析器头文件
2. [src/semantic.c](file:///d:/Projects/C/Compiler/src/semantic.c) - 语义分析器实现

主要数据结构：

- [Symbol](file:///d:/Projects/C/Compiler/src/semantic.c#148-163) - 符号（变量/函数信息）
- [Scope](file:///d:/Projects/C/Compiler/src/semantic.c#102-107) - 作用域（包含符号表）
- [SemanticAnalyzer](file:///d:/Projects/C/Compiler/src/semantic.c#8-23) - 语义分析器（管理作用域栈）

---

## ✨ 练习思考

1. 下面的代码有什么语义错误？

   ```c
   int x = 1;
   if (x > 0) {
       int x = 2;  // 这是合法的吗？
   }
   ```
2. 为什么函数要在调用前声明？
3. `int a = a + 1;` 这行代码有什么问题？
