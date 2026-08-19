---
title: C++异常
date: 2026-08-19
tags: []
draft: false
---
# 🐉版
## 异常
### C语言传统的处理错误的方式
传统的错误处理机制： 
1. 终止程序，如assert。缺陷：用户难以接受。如发生内存错误，除0错误时就会终止程序。
2. 返回错误码。缺陷：需要程序员自己去查找对应的错误。如系统的很多库的接口函数都是通过把错误码放到error中，表示错误。
3. C标准库中setjmp和longjmp组合。（不常用）

实际中C语言基本都是使用返回错误码的方式处理错误，部分情况下使用终止程序处理非常严重的错误。

### C++异常概念

异常是面向对象语言常用的一种处理错误的方式，当一个函数发现自己无法处理的错误时就可以抛出异常，让函数直接或间接的调用者处理这个错误。
- throw：当程序出现问题时，可以通过throw关键字抛出一个异常。
- try：try块中放置的是可能抛出异常的代码，该代码块在执行时将进行异常错误检测，try块后面通常跟着一个或多个catch块。
- catch：如果try块中发生错误，则可以在catch块中定义对应要执行的代码块。

使用try/catch语句的语法如下所示：
```cpp
try
{
	//被保护的代码
}
catch (ExceptionName e1)
{
	//catch块
}
catch (ExceptionName e2)
{
	//catch块
}
catch (ExceptionName eN)
{
	//catch块
}
```
### 异常的用法

#### 异常的抛出和捕获
异常的抛出和捕获的匹配原则：
1. 异常是通过抛出对象而引发的，该对象的类型决定了应该激活哪个catch的处理代码，如果抛出的异常对象没有捕获，或是没有匹配类型的捕获，那么程序会终止报错。
2. 被选中的处理代码（catch块）是调用链中与该对象类型匹配且离抛出异常位置最近的那一个。
3. 抛出异常对象后，会生成一个异常对象的拷贝，因为抛出的异常对象可能是一个临时对象，所以会生成一个拷贝对象，这个拷贝的临时对象会在被catch以后销毁。（类似于函数的传值返回）
4. `catch(...)`可以捕获任意类型的异常，但捕获后无法知道异常错误是什么。
5. 实际异常的抛出和捕获的匹配原则有个例外，捕获和抛出的异常类型并不一定要完全匹配，可以抛出派生类对象，使用基类进行捕获，这个在实际中非常有用。

在函数调用链中异常栈展开的匹配原则：
1. 当异常被抛出后，首先检查throw本身是否在try块内部，如果在则查找匹配的catch语句，如果有匹配的，则跳到catch的地方进行处理。
2. 如果当前函数栈没有匹配的catch则退出当前函数栈，继续在上一个调用函数栈中进行查找匹配的catch。找到匹配的catch子句并处理以后，会沿着catch子句后面继续执行，而不会跳回到原来抛异常的地方。
3. 如果到达main函数的栈，依旧没有找到匹配的catch，则终止程序。

比如下面的代码中main函数中调用了func3，func3中调用了func2，func2中调用了func1，在func1中抛出了一个string类型的异常对象。
```cpp
void func1()
{
	throw string("这是一个异常");
}
void func2()
{
	func1();
}
void func3()
{
	func2();
}
int main()
{
	try
	{
		func3();
	}
	catch (const string& s)
	{
		cout << "错误描述：" << s << endl;
	}
	catch (...)
	{
		cout << "未知异常" << endl;
	}
	return 0;
}
```

当func1中的异常被抛出后：
- 首先会检查throw本身是否在try块内部，这里由于throw不在try块内部，因此会退出func1所在的函数栈，继续在上一个调用函数栈中进行查找，即func2所在的函数栈。
- 由于func2中也没有匹配的catch，因此会继续在上一个调用函数栈中进行查找，即func3所在的函数栈。
- func3中也没有匹配的catch，于是就会在main所在的函数栈中进行查找，最终在main函数栈中找到了匹配的catch。
- 这时就会跳到main函数中对应的catch块中执行对应的代码块，执行完后继续执行该代码块后续的代码。

如下图所示：
![](../../_Pasted_Image/Pasted%20image%2020260819201949.png)
上述这个沿着调用链查找匹配的catch子句的过程称为栈展开。在实际中我们最后都要加一个`catch(...)`捕获任意类型的异常，否则当有异常没捕获时，程序就会直接终止。

#### 异常的重新抛出
有时候单个的catch可能不能完全处理一个异常，在进行一些校正处理以后，希望再交给更外层的调用链函数来处理，比如最外层可能需要拿到异常进行日志信息的记录，这时就需要通过重新抛出将异常传递给更上层的函数进行处理。

但如果直接让最外层捕获异常进行处理可能会引发一些问题。比如：
```cpp
void func1()
{
	throw string("这是一个异常");
}
void func2()
{
	int* array = new int[10];
	func1();

	//do something...

	delete[] array;
}
int main()
{
	try
	{
		func2();
	}
	catch (const string& s)
	{
		cout << s << endl;
	}
	catch (...)
	{
		cout << "未知异常" << endl;
	}
	return 0;
}
```
其中func2中通过new操作符申请了一块内存空间，并且在func2最后通过delete对该空间进行了释放，但由于func2中途调用的func1内部抛出了一个异常，这时会直接跳转到main函数中的catch块执行对应的异常处理程序，并且在处理完后继续沿着catch块往后执行。

这时就导致func2中申请的内存块没有得到释放，造成了内存泄露。这时可以在func2中先对func1抛出的异常进行捕获，捕获后先将申请到的内存释放再将异常重新抛出，这时就避免了内存泄露。比如：
```cpp
void func2()
{
	int* array = new int[10];
	try
	{
		func1();
		//do something...
	}
	catch (...)
	{
		delete[] array;
		throw; //将捕获到的异常再次重新抛出
	}
	delete[] array;
}
```
**说明一下：**
- func2中的new和delete之间可能还会抛出其他类型的异常，因此在fun2中最好以`catch(...)`的方式进行捕获，将申请到的内存delete后再通过throw重新抛出。
- 重新抛出异常对象时，throw后面可以不用指明要抛出的异常对象（正好也不知道以`catch(...)`的方式捕获到的具体是什么异常对象）。

#### 异常安全
将抛异常导致的安全问题叫做异常安全问题，对于异常安全问题下面给出几点建议：
1. 构造函数完成对象的构造和初始化，最好不要在构造函数中抛出异常，否则可能导致对象不完整或没有完全初始化。
2. 析构函数主要完成对象资源的清理，最好不要在析构函数中抛出异常，否则可能导致资源泄露（内存泄露、句柄未关闭等）。
3. C++中异常经常会导致资源泄露的问题，比如在new和delete中抛出异常，导致内存泄露，在lock和unlock之间抛出异常导致死锁，C++经常使用RAII的方式来解决以上问题。
#### 异常规范
为了让函数使用者知道某个函数可能抛出哪些类型的异常，C++标准规定：
1. 在函数的后面接`throw(type1, type2, ...)`，列出这个函数可能抛掷的所有异常类型。
2. 在函数的后面接`throw()`或`noexcept`（C++11），表示该函数不抛异常。
3. 若无异常接口声明，则此函数可以抛掷任何类型的异常。（异常接口声明不是强制的）

比如：
```cpp
//表示func函数可能会抛出A/B/C/D类型的异常
void func() throw(A, B, C, D);
//表示这个函数只会抛出bad_alloc的异常
void* operator new(std::size_t size) throw(std::bad_alloc);
//表示这个函数不会抛出异常
void* operator new(std::size_t size, void* ptr) throw();
```

### 自定义异常体系

实际中很多公司都会自定义自己的异常体系进行规范的异常管理。
- 公司中的项目一般会进行模块划分，让不同的程序员或小组完成不同的模块，如果不对抛异常这件事进行规范，那么负责最外层捕获异常的程序员就非常难受了，因为他需要捕获大家抛出的各种类型的异常对象。
- 因此实际中都会定义一套继承的规范体系，先定义一个最基础的异常类，所有人抛出的异常对象都必须是继承于该异常类的派生类对象，因为异常语法规定可以用基类捕获抛出的派生类对象，因此最外层就只需捕获基类就行了。

最基础的异常类至少需要包含错误编号和错误描述两个成员变量，甚至还可以包含当前函数栈帧的调用链等信息。该异常类中一般还会提供两个成员函数，分别用来获取错误编号和错误描述。比如：
```cpp
class Exception
{
public:
	Exception(int errid, const char* errmsg)
		:_errid(errid)
		, _errmsg(errmsg)
	{}
	int GetErrid() const
	{
		return _errid;
	}
	virtual string what() const
	{
		return _errmsg;
	}
protected:
	int _errid;     //错误编号
	string _errmsg; //错误描述
	//...
};
```

其他模块如果要对这个异常类进行扩展，必须继承这个基础的异常类，可以在继承后的异常类中按需添加某些成员变量，或是对继承下来的虚函数what进行重写，使其能告知程序员更多的异常信息。比如：
```cpp
class CacheException : public Exception
{
public:
	CacheException(int errid, const char* errmsg)
		:Exception(errid, errmsg)
	{}
	virtual string what() const
	{
		string msg = "CacheException: ";
		msg += _errmsg;
		return msg;
	}
protected:
	//...
};
class SqlException : public Exception
{
public:
	SqlException(int errid, const char* errmsg, const char* sql)
		:Exception(errid, errmsg)
		, _sql(sql)
	{}
	virtual string what() const
	{
		string msg = "CacheException: ";
		msg += _errmsg;
		msg += "sql语句: ";
		msg += _sql;
		return msg;
	}
protected:
	string _sql; //导致异常的SQL语句
	//...
};
```
**说明一下：**
- 异常类的成员变量不能设置为私有，因为私有成员在子类中是不可见的。
- 基类Exception中的what成员函数最好定义为虚函数，方便子类对其进行重写，从而达到多态的效果。

### 标准库异常体系

C++标准库当中的异常也是一个基础体系，其中exception就是各个异常类的基类，我们可以在程序中使用这些标准的异常，它们之间的继承关系如下：![](../../_Pasted_Image/Pasted%20image%2020260819203554.png)
下表是对上面继承体系中出现的每个异常的说明：![](../../_Pasted_Image/Pasted%20image%2020260819203609.png)

**说明一下：**
- exception类的what成员函数和析构函数都定义成了虚函数，方便子类对其进行重写，从而达到多态的效果。
- 实际中我们也可以去继承exception类来实现自己的异常类，但实际中很多公司都会自己定义一套异常继承体系。

### 异常的优缺点

异常的优点：
1. 异常对象定义好了，相比错误码的方式可以清晰准确的展示出错误的各种信息，甚至可以包含堆栈调用等信息，这样可以帮助更好的定位程序的bug。
2. 返回错误码的传统方式有个很大的问题就是，在函数调用链中，深层的函数返回了错误，那么我们得层层返回错误码，最终最外层才能拿到错误。
3. 很多的第三方库都会使用异常，比如boost、gtest、gmock等等常用的库，如果我们不用异常就不能很好的发挥这些库的作用。
4. 很多测试框架也都使用异常，因此使用异常能更好的使用单元测试等进行白盒的测试。
5. 部分函数使用异常更好处理，比如`T& operator`这样的函数，如果pos越界了只能使用异常或者终止程序处理，没办法通过返回值表示错误。

异常的缺点：
1. 异常会导致程序的执行流乱跳，并且非常的混乱，这会导致我们跟踪调试以及分析程序时比较困难。
2. 异常会有一些性能的开销，当然在现代硬件速度很快的情况下，这个影响基本忽略不计。
3. C++没有垃圾回收机制，资源需要自己管理。有了异常非常容易导致内存泄露、死锁等异常安全问题。这个需要使用RAII来处理资源的管理问题，学习成本比较高。
4. C++标准库的异常体系定义得不够好，导致大家各自定义自己的异常体系，非常的混乱。
5. 异常尽量规范使用，否则后果不堪设想，随意抛异常，也会让外层捕获的用户苦不堪言。
6. 异常接口声明不是强制的，对于没有声明异常类型的函数，无法预知该函数是否会抛出异常。

但总体而言，异常的利大于弊，所以工程中我们还是鼓励使用异常的，并且OO的语言基本都使用异常处理错误，这也可以看出这是大势所趋。

# AI讲解版C++ 异常机制

C++ 异常机制的目的，是：

> **把“正常业务流程”和“错误处理流程”分离。**

核心关键字只有三个：

```cpp
try
catch
throw
```

下面压缩成大约 20 个面试重点。

---

## 1. 基本语法

```cpp
try
{
    if (error)
    {
        throw std::runtime_error("error");
    }
}
catch (const std::exception& e)
{
    std::cout << e.what();
}
```

执行流程：

```
try 中执行代码
↓
遇到 throw
↓
立即停止当前正常流程
↓
寻找匹配的 catch
↓
进入 catch 处理
```

---
## 2. `throw` 到底做了什么

```cpp
throw value;
```

表示：

> 抛出一个异常对象。

例如：

```cpp
throw 10;
throw std::string("error");
throw std::runtime_error("file open failed");
```

C++ 异常可以是几乎任意类型。

但工程中通常优先使用：

```cpp
std::exception
```

体系中的标准异常类。

---

## 3. `catch` 如何匹配异常

例如：

```cpp
try
{
    throw 10;
}
catch (int e)
{
}
catch (double e)
{
}
```

抛出 `int`，就匹配：

```cpp
catch (int)
```

匹配原则主要看：

- 异常类型；
- 引用/const；
- 类继承关系。

---

## 4. 类异常推荐按 `const 引用` 捕获

推荐：

```cpp
catch (const std::exception& e)
{
}
```

而不是：

```cpp
catch (std::exception e)
{
}
```

原因：

1. 避免复制异常对象；
2. 避免对象切片；
3. 可以正确保留派生异常类型。

例如：

```cpp
throw std::runtime_error("error");
```

如果：

```cpp
catch (std::exception e)
```

可能发生对象切片。

而：

```cpp
catch (const std::exception& e)
```

可以保持动态类型。

---

## 5. `catch(...)`

```cpp
catch (...)
{
}
```

表示：

> 捕获任何类型的异常。

例如：

```cpp
try
{
    throw 10;
}
catch (...)
{
    std::cout << "unknown exception";
}
```

通常用于：

- 最外层兜底；
- 日志记录；
- 防止异常继续传播。

但它拿不到具体异常对象。

---

## 6. 多个 `catch` 的顺序

对于继承关系：

```cpp
catch (const std::runtime_error& e)
{
}
catch (const std::exception& e)
{
}
```

具体类型应该放前面。

不能把基类放前面：

```cpp
catch (const std::exception& e)
{
}

catch (const std::runtime_error& e)
{
}
```

否则后面的派生类处理逻辑永远到不了。

所以：

> `catch` 应该从具体类型到一般类型排列。

---

## 7. 异常传播

如果当前函数没有处理异常：

```cpp
void Func3()
{
    throw std::runtime_error("error");
}

void Func2()
{
    Func3();
}

void Func1()
{
    Func2();
}
```

如果 `Func3`、`Func2` 都没有 `catch`，异常会继续向调用者传播。

```
Func3
↓
Func2
↓
Func1
↓
main
```

直到找到匹配的 `catch`。

---

## 8. 栈展开 Stack Unwinding

这是异常机制最核心的底层概念之一。

假设：

```cpp
void Func()
{
    A a;
    B b;

    throw std::runtime_error("error");
}
```

异常抛出后：

```
停止正常流程
↓
销毁已经构造完成的局部对象
↓
b 析构
↓
a 析构
↓
继续向上寻找 catch
```

这个过程叫：

> **stack unwinding，栈展开。**

---

## 9. 为什么异常和 RAII 天生配套

看：

```cpp
void Func()
{
    std::vector<int> data(1000);

    throw std::runtime_error("error");
}
```

异常发生后：

```
栈展开
↓
vector 析构
↓
自动释放内存
```

锁也是一样：

```cpp
std::lock_guard<std::mutex> lock(mutex);

throw std::runtime_error("error");
```

栈展开时：

```
lock_guard 析构
↓
mutex 自动解锁
```

所以：

> C++ 异常安全高度依赖 RAII。

---

## 10. 为什么裸资源遇到异常很危险

错误：

```cpp
void Func()
{
    int* p = new int[100];

    DoSomething(); // 这里可能抛异常

    delete[] p;
}
```

如果 `DoSomething()` 抛异常：

```cpp
delete[] p;
```

不会执行。

造成内存泄漏。

应该：

```cpp
std::vector<int> data(100);
```

或者：

```cpp
auto p = std::make_unique<int[]>(100);
```

---

# 11. 构造函数可以抛异常

例如：

```cpp
class File
{
public:
    File()
    {
        if (open_failed)
        {
            throw std::runtime_error("open failed");
        }
    }
};
```

如果构造函数抛异常：

> 当前对象被认为没有构造成功。

因此不会调用这个对象本身的析构函数。

但是：

> 已经成功构造的基类和成员对象会自动析构。

---

## 12. 析构函数为什么通常不能抛异常

这是高频面试题。

假设正在栈展开：

```
异常 A 正在传播
↓
调用局部对象析构
↓
析构函数又抛异常 B
```

此时同时有两个未处理异常。

程序会调用：

```cpp
std::terminate()
```

所以现代 C++ 通常要求：

> 析构函数不要让异常逃出去。

C++11 起，很多析构函数默认就是 `noexcept`。

---

## 13. `noexcept`

```cpp
void Func() noexcept
{
}
```

表示：

> 这个函数承诺不会让异常传播到函数外部。

如果真的有异常逃出去：

```cpp
void Func() noexcept
{
    throw std::runtime_error("error");
}
```

程序会调用：

```cpp
std::terminate()
```

不是正常进入外层 `catch`。

---

## 14. 为什么移动构造常写 `noexcept`

例如：

```cpp
class Buffer
{
public:
    Buffer(Buffer&& other) noexcept;
};
```

原因和 `std::vector` 扩容有关。

如果移动构造可能抛异常：

```
已经移动了一部分元素
↓
中途抛异常
↓
旧对象可能已被破坏
```

为了强异常安全，标准容器有时会优先复制。

如果移动构造声明：

```
noexcept
```

容器更愿意使用移动。

所以面试常答：

> 能保证不抛异常的移动构造和移动赋值应声明 `noexcept`。

---

## 15. `throw;` 和 `throw e;` 不一样

在 `catch` 中重新抛异常：

推荐：

```cpp
catch (const std::exception& e)
{
    throw;
}
```

这表示：

> 重新抛出当前原始异常对象。

而：

```cpp
throw e;
```

相当于根据 `e` 再抛一个新对象。

可能发生：

- 复制；
- 类型信息丢失；
- 对象切片。

因此：

> 重新抛出当前异常时优先使用裸 `throw;`。

---

## 16. 标准异常体系

标准库常见异常：

```
std::exception
│
├── std::logic_error
│   ├── invalid_argument
│   ├── out_of_range
│   └── length_error
│
└── std::runtime_error
```

另外常见：

```
std::bad_alloc
std::bad_cast
std::bad_function_call
```

都可以通过：

```
e.what()
```

获得错误描述。

---

## 17. `new` 失败通常抛什么异常

普通：

```cpp
int* p = new int[huge_size];
```

分配失败通常抛：

```cpp
std::bad_alloc
```

如果不希望抛异常，可以：

```cpp
int* p = new (std::nothrow) int[100];
```

失败时：

```cpp
p == nullptr
```

---

## 18. 异常对象的生命周期

例如：

```cpp
throw std::runtime_error("error");
```

异常对象不是简单依赖当前函数栈上的局部对象。

运行时会保存一个异常对象，使它能够跨越多个函数栈帧传播。

直到：

```
异常被 catch
↓
处理结束
```

对应异常对象才被销毁。

实现细节由 ABI/运行时决定，标准规定的是行为，不要求具体存储位置。

---

## 19. 异常安全的三个常见级别

面试加分点。

### 基本保证

发生异常后：

> 对象仍然有效，没有资源泄漏，但状态可能改变。

### 强保证

发生异常后：

> 操作仿佛从未发生。

也叫：

```
commit or rollback
```

### 不抛保证

```
no-throw guarantee
```

保证操作不会抛出异常。

例如析构和移动操作通常尽量做到这一点。

---

## 20. 异常和返回错误码的区别

错误码：

```cpp
int OpenFile();
```

调用者必须主动检查：

```cpp
if (OpenFile() != 0)
{
}
```

异常：

```cpp
throw std::runtime_error(...);
```

可以跨多层调用栈传播。

优点：

- 错误处理与正常逻辑分离；
- 不需要每层都手动传递错误码；
- RAII 可以自动清理资源。

缺点：

- 控制流不够直观；
- 异常路径成本较高；
- 实时系统/游戏引擎热点路径有时会限制异常使用；
- 必须设计好异常安全。

---

# 一个完整例子

```cpp
#include <iostream>
#include <stdexcept>
#include <vector>

void Process()
{
    std::vector<int> data(1000);

    throw std::runtime_error("Process failed");
}

int main()
{
    try
    {
        Process();
    }
    catch (const std::runtime_error& e)
    {
        std::cout << e.what() << '\n';
    }
    catch (const std::exception& e)
    {
        std::cout << "other std exception\n";
    }
    catch (...)
    {
        std::cout << "unknown exception\n";
    }
}
```

异常路径：

```
Process()
↓
vector 构造
↓
throw runtime_error
↓
开始栈展开
↓
vector 析构
↓
进入 main 的 runtime_error catch
```

---

## 高频追问

### 1. C++ 异常机制的基本流程是什么？

`throw` 抛出异常对象，运行时沿调用栈寻找匹配的 `catch`。在寻找过程中会进行栈展开，并析构已经构造完成的局部对象。

---

### 2. 为什么推荐 `catch(const std::exception& e)`？

因为可以：

- 避免复制；
- 避免对象切片；
- 保留派生异常动态类型；
- 又保证 catch 中不修改异常对象。

---

### 3. 什么是栈展开？

异常传播时，程序退出当前作用域，并按逆序析构已经成功构造的自动对象，直到找到匹配的 `catch`。

---

### 4. 析构函数能不能抛异常？

语法上可能，但通常绝对不应该让异常逃出析构函数。

如果析构发生在栈展开期间又抛出第二个异常，会导致：

```
std::terminate()
```

---

### 5. `noexcept` 有什么作用？

表示函数承诺不向外传播异常。

如果异常逃出 `noexcept` 函数，调用 `std::terminate()`。

它也会影响标准库的优化策略，比如 `vector` 是否优先移动对象。

---

### 6. `throw;` 和 `throw e;` 区别？

```
throw;
```

重新抛出当前异常，保留原异常类型。

```
throw e;
```

重新构造并抛出 `e`，可能发生复制和切片。

---

## 面试回答模板

面试官问：

> C++ 异常机制是什么？

可以回答：

C++ 异常机制主要通过 `try`、`throw` 和 `catch` 实现。发生错误时使用 `throw` 抛出异常对象，运行时会沿调用栈寻找类型匹配的 `catch`。传播过程中会进行栈展开，依次析构已经成功构造的局部对象，因此异常机制和 RAII 配合得非常紧密，可以保证资源自动释放。

异常对象通常推荐按 `const` 引用捕获，避免复制和对象切片。重新抛出当前异常时使用裸 `throw;`，避免丢失动态类型。

C++11 还引入了 `noexcept`，表示函数不允许异常向外传播，如果异常逃出会调用 `std::terminate`。移动构造通常尽量声明为 `noexcept`，这样标准容器可以更放心地使用移动操作。

---

# 最后只记 10 条

1. `throw` 抛异常，`catch` 捕获异常。
2. 异常会沿调用栈向上传播。
3. 传播过程中会进行 stack unwinding。
4. 栈展开会析构已经构造完成的局部对象。
5. RAII 是实现异常安全的核心手段。
6. 异常类推荐 `catch(const T&)`。
7. 重新抛出当前异常用 `throw;`。
8. 析构函数通常不能让异常逃出去。
9. `noexcept` 函数如果异常逃出，会 `std::terminate`。
10. 移动构造能保证不抛时应尽量写 `noexcept`。