---
title: STL
date: 2026-08-21
tags: []
draft: false
---
# STL

## String类的模拟实现

1.其本质就是对char数值的包装，然后丰富其功能
2.里面调用了挺多str的函数，例如：
- `std::strlen(str)`：计算 C 风格字符串长度。
- `std::strcmp(left, right)`：比较两个 C 风格字符串，用于 `==`、`<` 等比较运算符。
- `std::strstr(text, pattern)`：在字符串中查找子串，用于 `find(const char*)`。
3.里面实现的函数可以互相调用来增强代码可读性

4.`std::memcpy` 是“按字节复制内存”的函数，定义在 `<cstring>` 中。
```
std::memcpy(目标地址, 源地址, 要复制的字节数);
```
例如：
```
std::memcpy(_str, str, _size + 1);
```
含义是：把 `str` 中的 `_size + 1` 个字节复制到 `_str`。

这里多复制 `1`，是为了连字符串结束符 `'\0'` 一起复制过去。

它和 `strcpy` 的区别：
- `strcpy`：一直复制到遇到 `'\0'`，无法指定长度。
- `memcpy`：严格复制你指定的字节数，不关心内容是不是字符串。

注意：`memcpy` 的源、目标内存区域不能重叠；如果可能重叠，应使用 `std::memmove`。