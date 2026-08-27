---
title: priority_queue
date: 2026-08-27
tags: []
draft: false
---
## 一、优先级队列的介绍

1.优先级队列是一种容器适配器，根据严格的弱排序标准，它的第一个元素总是它所包含的元素中最大的。  
2.优先级队列类似于堆，在堆中可以随时插入元素，并且只能检索最大堆元素（优先队列中位于顶部的元素）。  
3.优先级队列被实现为容器适配器，容器适配器即将特定容器类封装作为其底层容器类，queue提供一组特定的成员函数来访问其元素。元素从特定容器的”尾部”弹出，其称为优先队列的顶部。  
4.底层容器可以是任何标准容器类模板，也可以是其他特定设计的容器类。容器应该可以通过随机访问迭代器访问，并支持以下操作：

- empty()：检测容器是否为空
- size()：返回容器中有效元素个数
- front()：返回容器中第一个元素的引用
- push_back()：在容器尾部插入元素
- pop_back():删除容器尾部元素

5.标准容器类vector和deque满足这些需求。默认情况下，如果没有为特定的priority_queue类实例化指定容器类，则使用vector。

在C++中，`priority_queue`模板类定义在`<queue>`头文件中，可以通过指定元素类型和比较函数来创建不同类型的优先队列。比较函数用于确定元素的优先级。
![](../../_Pasted_Image/Pasted%20image%2020260827135628.png)
默认情况下，`priority_queue`使用`std::less`作为比较函数，即元素的优先级按照从大到小的顺序排列。如果需要按照从小到大的顺序排列，可以使用`std::greater`作为比较函数。

## 二、优先级队列的使用

### 2.1 构造函数

优先级队列 默认使用 vector 作为底层存储数据的容器，在 vector 上又使用了 堆算法 将 vector 中的元素构造成堆的结构，因此 priority_queue 就是 ---- 堆，所以在需要用到 堆 的地方，都可以考虑使用 priority_queue

优先级队列的构造函数原型如下所示：![](../../_Pasted_Image/Pasted%20image%2020260827135645.png)
#### 2.1.1 构造一个空的优先级队列

```cpp
#include <iostream>
#include <vector>
#include <queue>
using namespace std;
int main()
{
	priority_queue pq;	//直接构造一个空对象，默认为大堆
	cout << typeid(pq).name() << endl;	//查看类型
	return 0;
}
```
![](../../_Pasted_Image/Pasted%20image%2020260827135705.png)**默认比较方式为 `less`，最终为 优先级高的值排在上面（`大堆`）**

#### 2.1.2 **通过迭代器区间构造**优先级队列

构造大堆：
```cpp
#include <iostream>
#include <vector>
#include <queue>

using namespace std;
int main()
{
	vector v1 = { 11,14,15,42,51,62,12,87,67 };
	priority_queue<int> pq(v1.begin(), v1.end());
	while (!pq.empty())
	{
		cout << pq.top() <<" ";
		pq.pop();
	}
	cout << endl;
	return 0;
}
```
![](../../_Pasted_Image/Pasted%20image%2020260827135753.png)
构造小堆：

```cpp
#include <iostream>
#include <vector>
#include <queue>
#include <functional>

using namespace std;

int main()
{
    vector<int> v1 = {11,14,15,42,51,62,12,87,67};

    priority_queue<int, vector<int>, greater<int>> pq(v1.begin(), v1.end());

    while (!pq.empty())
    {
        cout << pq.top() << " ";
        pq.pop();
    }

    cout << endl;
}
```
### 2.2 push函数

在优先级队列的尾部插入 一个 新的元素，每次插入前调用 堆排序算法，将其重新排序在堆中的位置
```cpp
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

int main()
{
	priority_queue pq;
	pq.push(10);
	pq.push(20);
	pq.push(30);
	pq.push(40);
	pq.push(50);
	return 0;
}
```
在上面的代码，我们默认构建的是大堆

### 2.3 pop函数

 删除位于优先级队列顶部的元素，其实就是删除队头元素
 ```cpp
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

int main()
{
	priority_queue pq;
	pq.push(10);
	pq.push(20);
	pq.push(30);
	pq.push(25);
	pq.push(50);
	cout << pq.top() << endl;
	pq.pop();
	cout << pq.top() << endl;
 ```
 
### 2.4 top函数

返回 优先级队列中顶部元素的常量引用，顶部元素实在优先级队列中比较高的元素
```cpp
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

int main()
{
	priority_queue pq;
	pq.push(10);
	pq.push(20);
	pq.push(30);
	pq.push(40);
	pq.push(50);
	cout << pq.top();
	return 0;
}
```
### 2.5 empty函数

测试容器是否为空

### 2.6 size函数

返回优先级队列的元素数量 

## 三、优先级队列的模拟实现

我们知道「**priority_ queue**」 的底层就是**堆**，所以在模拟实现之前，要先实现堆的调整算法。 

### 3.1 向上调整算法AdjustUp

以小堆为例：

假如，我们现在有一个数组a:15 18 19 25 28 34 65 49 27 37，他从逻辑上看，是一个小根堆结构，我们将它的逻辑结构画出来如下所示：
![](../../_Pasted_Image/Pasted%20image%2020260827135931.png)
我们现在想要将10插入堆里面去，插入10之后，要不允许破坏原本小根堆的结构，所以我们就需要将10一步一步的调整，如下所示：
![](../../_Pasted_Image/Pasted%20image%2020260827135940.png)
```cpp
//向上调整,建立大根堆
void AdjustUp(HPDataType* a, int child) {
	assert(a);
	int parent = (child - 1) / 2;
	while (child > 0) {
		if (a[child] < a[parent]) {
			Swap(&a[child], &a[parent]);
		 }
		child = parent;
		parent = (child - 1) / 2;
	}
}
```

### 3.2 向下调整算法AdjustDown
```cpp
void AdjustDown(HPDataType* a, int parent, int size) {
	int child = 2 * parent + 1;
	while (child < size) {
		if (child + 1 < size && a[child + 1] < a[child]) {
			child++;
		}
		if (a[child] < a[parent]) {
			Swap(&a[child], &a[parent]);
		}
		parent = child;
		child = 2 * parent + 1;
	}
}
```

### 3.3 优先级队列的模拟实现

优先级队列是一种容器适配器，默认使用vector作为底层的容器，并且默认是构建大堆的优先级队列，现在我们实现优先级队列，先不加仿函数，下面再加，优先级队列的声明如下：

```cpp
#pragma once
#include <iostream>
#include <vector>
#include <queue>

using namespace std;

namespace zx {
	template>
	class priority_queue
	{
	public:
	private:
		Container _con;
	};
}
```

#### 3.3.1 push函数的实现

往优先级队列里面插入数据，在插入数据之前我们的容器已经是一个堆了。在尾部插入一个数据，然后执行向上调整算法，代码如下所示：
```cpp
//向上调整,建立大根堆
void AdjustUp(int child) {
	int parent = (child - 1) / 2;
	while (child > 0) {
		if (_con[child] > _con[parent]) {
			swap(_con[child], _con[parent]);
		}
		child = parent;
		parent = (child - 1) / 2;
	}
}
void push(const T& x)
{
	_con.push_back(x);
	AdjustUp(_con.size() - 1);
}
```
#### 3.3.2 pop函数的实现

删除优先级队列的第一个元素，实际上是与最后一个元素交换，然后执行向下调整算法。代码如下：

```cpp
//向下调整，建立大根堆
void AdjustDown(int parent, int size) {
	int child = 2 * parent + 1;
	while (child < size) {
		if (child + 1 < size && _con[child + 1] > _con[child]) {
			child++;
		}
		if (_con[child] > _con[parent]) {
			swap(_con[child], _con[parent]);
		}
		parent = child;
		child = 2 * parent + 1;
	}
}
void pop()
{
	swap(_con[0], _con[_con.size() - 1]);
	_con.pop_back();
	AdjustDown(0, _con.size());
}
```

#### 3.3.3 top函数的实现

取出第0个数据，代码如下：

```cpp
const T& top(){	return _con[0];}
```

#### 3.3.4 size函数的实现

```cpp
size_t size() const{	return _con.size();}
```

#### 3.3.5 empty函数的实现

```cpp
size_t empty() const{	return _con.empty();}
```

### 3.4 测试

```cpp
#include"Priority_Queue.h"
int main() {
	zx::priority_queue pq;
	pq.push(4);
	pq.push(1);
	pq.push(5);
	pq.push(7);
	pq.push(9);
	while (!pq.empty())
	{
		cout << pq.top() << " ";
		pq.pop();
	}
	cout << endl;
	return 0;
}
```

运行如下：
![](../../_Pasted_Image/Pasted%20image%2020260827140112.png)

## 四、仿函数

### 4.1 我们模拟实现的优先级队列问题

在我们模拟实现的优先级队列里面，默认是建立的大根堆，如果我们想要建立小根堆呢？就需要修改里面的逻辑，把大于号改成小于号，或者把小于号改成大于号，来实现两个优先级队列，这样就有大量的重复代码了。

C++可以使用仿函数来解决上面的问题。

### 4.2 仿函数的介绍

仿函数是一个类，并不是一个函数，它的使用类似函数。

例如，我想要实现两个整型的比较，首先声明一个类，然后重载小括号运算符()。如下所示：
```cpp
//仿函数的本质是一个类，这个类重载了operator()，它的对象可以像函数一样使用。
template
class Less
{
public:
	bool operator()(const T& x, const T& y)
	{
		return x < y;
	}
};
int main()
{
	less lessFunc;
	cout << lessFunc(1, 2) << endl;
	cout << lessFunc.operator()(1, 2) << endl;
	return 0;
}
```

### 4.3 仿函数的使用

我们拿冒泡排序来使用一下仿函数，如下所示：
```cpp
void BubbleSort(int* a, int n)
{
	for (int j = 0; j < n; j++)
	{
		//单趟
		int flag = 0;
		for (int i = 1; i < n - j; i++)
		{
			if (a[i] < a[i - 1] )
			{
				swap(a[i - 1], a[i]);
				flag = 1;
			}
		}
		if (flag == 0)
		{
			break;
		}
	}
}
```

这个冒泡排序模式是排升序的，如果我们想要排降序，就可以来实现仿函数来实现，代码如下所示：
```cpp
//仿函数的本质是一个类，这个类重载了operator()，它的对象可以像函数一样使用。
template
class Less
{
public:
	bool operator()(const T& x, const T& y)
	{
		return x < y;
	}
};
template
class Greater
{
public:
	bool operator()(const T& x, const T& y)
	{
		return x > y;
	}
};
template
void BubbleSort(int* a, int n,Compare com)
{
	for (int j = 0; j < n; j++)
	{
		//单趟
		int flag = 0;
		for (int i = 1; i < n - j; i++)
		{
			//if (a[i] < a[i - 1] )
			if(com(a[i],a[i - 1]))
			{
				swap(a[i - 1], a[i]);
				flag = 1;
			}
		}
		if (flag == 0)
		{
			break;
		}
	}
}
int main()
{
	Less LessFunc;
	Greater GreaterFunc;
	int a[] = { 9,1,2,5,7,4,6,3 };
	BubbleSort(a, 8, LessFunc);
	for (auto ele : a) {
		cout << ele << " ";
	}
	cout << endl;
	BubbleSort(a, 8, GreaterFunc);
	for (auto ele : a) {
		cout << ele << " ";
	}
	cout << endl;
	//也可以传匿名对象
	BubbleSort(a, 8, Less());
	BubbleSort(a, 8, Greater());
	return 0;
}
```
### 4.4 使用仿函数实现优先级队列

我们在比较的时候就不能写大于号或者小于号，应该写仿函数。

代码如下：
```cpp
#pragma once

#include <vector>
#include <functional>
#include <utility>

namespace zx
{
    template<class T,class Container = std::vector<T>,class Compare = std::less<T>>
    class priority_queue
    {
    public:
        void AdjustUp(int child)
        {
            Compare com;

            int parent = (child - 1) / 2;

            while (child > 0)
            {
                if (com(_con[parent], _con[child]))
                {
                    std::swap(_con[parent], _con[child]);
                }
                else
                {
                    break;
                }

                child = parent;
                parent = (child - 1) / 2;
            }
        }

        void push(const T& x)
        {
            _con.push_back(x);
            AdjustUp(static_cast<int>(_con.size()) - 1);
        }

        void AdjustDown(int parent, int size)
        {
            Compare com;

            int child = 2 * parent + 1;

            while (child < size)
            {
                if (child + 1 < size &&
                    com(_con[child], _con[child + 1]))
                {
                    ++child;
                }

                if (com(_con[parent], _con[child]))
                {
                    std::swap(_con[parent], _con[child]);
                }
                else
                {
                    break;
                }

                parent = child;
                child = 2 * parent + 1;
            }
        }

        void pop()
        {
            std::swap(_con[0], _con[_con.size() - 1]);
            _con.pop_back();

            if (!_con.empty())
            {
                AdjustDown(0, static_cast<int>(_con.size()));
            }
        }

        const T& top() const
        {
            return _con[0];
        }

        size_t size() const
        {
            return _con.size();
        }

        bool empty() const
        {
            return _con.empty();
        }

    private:
        Container _con;
    };
}
```

测试代码：构建大堆
```cpp
#include"Priority_Queue.h"
int main() {
	zx::priority_queue pq;
	pq.push(4);
	pq.push(1);
	pq.push(5);
	pq.push(7);
	pq.push(9);
	while (!pq.empty())
	{
		cout << pq.top() << " ";
		pq.pop();
	}
	cout << endl;
	return 0;
}
```

测试代码，构建小堆
```cpp
#include"Priority_Queue.h"
int main() {
    zx::priority_queue<int,std::vector<int>,std::greater<int>> pq;

	pq.push(4);
	pq.push(1);
	pq.push(5);
	pq.push(7);
	pq.push(9);
	while (!pq.empty())
	{
		cout << pq.top() << " ";
		pq.pop();
	}
	cout << endl;
	return 0;
}
```
## 五、需要我们自己写仿函数的场景
在实际开发中，是不需要我们自己写仿函数的，因为库里面提供了仿函数，如
std::less< T >和std::greater< T >

但是有两种场景是需要我们自己写仿函数的，

场景一：优先级队列里面存放的是自定义类型，并且自定义类型里面没有重载比较函数。

场景二：优先级队列里面存放的是自定义类型的指针，自定义类型里面重载了比较函数。但是优先级队列里面比较的是指针大小，不是我们想要的，我们想要的是指针所指向的内容大小，这就需要自己写一个仿函数。

 对于场景一，我们优先级队列里面存放的是自定义类型，如Date类型，如果Date类型里面没有提供比较的函数，那么在优先级队列的情况下就会出错，如下所示：
```cpp
#include
using namespace std;
#include
class Date
{
	friend ostream& operator<<(ostream& _cout, const Date& d);
public:
	Date(int year = 1900, int month = 1, int day = 1)
		: _year(year)
		, _month(month)
		, _day(day)
	{
	}
private:
	int _year;
	int _month;
	int _day;
};
ostream& operator<<(ostream& _cout, const Date& d)
{
	_cout << d._year << "-" << d._month << "-" << d._day;
	return _cout;
}
int main()
{
	// 大堆，需要用户在自定义类型中提供<的重载
	priority_queue q1;
	q1.push(Date(2018, 10, 29));
	q1.push(Date(2018, 10, 28));
	q1.push(Date(2018, 10, 30));
	cout << q1.top() << endl;
	// 如果要创建小堆，需要用户提供>的重载
	priority_queue, greater> q2;
	q2.push(Date(2018, 10, 29));
	q2.push(Date(2018, 10, 28));
	q2.push(Date(2018, 10, 30));
	cout << q2.top() << endl;
	return 0;
}
```

此时运行上面代码就会出错，我们就需要写一个比较Date类型的仿函数，或者在类里面重载比较运算符。

```cpp
#include
using namespace std;
#include
class Date
{
	friend ostream& operator<<(ostream& _cout, const Date& d);
public:
	Date(int year = 1900, int month = 1, int day = 1)
		: _year(year)
		, _month(month)
		, _day(day)
	{
	}
	bool operator<(const Date& d) const
	{
		return (_year < d._year) ||
			(_year == d._year && _month < d._month) ||
			(_year == d._year && _month == d._month && _day < d._day);
	}
	bool operator>(const Date& d)const
	{
		return (_year > d._year) ||
			(_year == d._year && _month > d._month) ||
			(_year == d._year && _month == d._month && _day > d._day);
	}
private:
	int _year;
	int _month;
	int _day;
};
ostream& operator<<(ostream& _cout, const Date& d)
{
	_cout << d._year << "-" << d._month << "-" << d._day;
	return _cout;
}
int main()
{
	// 大堆，需要用户在自定义类型中提供<的重载
	priority_queue q1;
	q1.push(Date(2018, 10, 29));
	q1.push(Date(2018, 10, 28));
	q1.push(Date(2018, 10, 30));
	cout << q1.top() << endl;
	q1.pop();
	cout << q1.top() << endl;
	q1.pop();
	cout << q1.top() << endl;
	q1.pop();
	// 如果要创建小堆，需要用户提供>的重载
	priority_queue, greater> q2;
	q2.push(Date(2018, 10, 29));
	q2.push(Date(2018, 10, 28));
	q2.push(Date(2018, 10, 30));
	cout << q2.top() << endl;
	return 0;
}
```

对应场景二：优先级队列里面存放的是自定义类型的指针，自定义类型里面重载了比较函数。但是优先级队列里面比较的是指针大小，不是我们想要的，我们想要的是指针所指向的内容大小，这就需要自己写一个仿函数。代码如下：
```cpp
#include
using namespace std;
#include
class Date
{
	friend ostream& operator<<(ostream& _cout, const Date& d);
public:
	Date(int year = 1900, int month = 1, int day = 1)
		: _year(year)
		, _month(month)
		, _day(day)
	{
	}
	bool operator<(const Date& d) const
	{
		return (_year < d._year) ||
			(_year == d._year && _month < d._month) ||
			(_year == d._year && _month == d._month && _day < d._day);
	}
	bool operator>(const Date& d)const
	{
		return (_year > d._year) ||
			(_year == d._year && _month > d._month) ||
			(_year == d._year && _month == d._month && _day > d._day);
	}
private:
	int _year;
	int _month;
	int _day;
};
ostream& operator<<(ostream& _cout, const Date& d)
{
	_cout << d._year << "-" << d._month << "-" << d._day;
	return _cout;
}
int main()
{
	// 大堆，需要用户在自定义类型中提供<的重载
	priority_queue q1;
	q1.push(new Date(2018, 10, 29));
	q1.push(new Date(2018, 10, 28));
	q1.push(new Date(2018, 10, 30));
	cout << *q1.top() << endl;
	q1.pop();
	cout << *q1.top() << endl;
	q1.pop();
	cout << *q1.top() << endl;
	q1.pop();
	return 0;
}
```

虽然我们在类里面重载了比较运算符，但是我们在优先级队列里面存放的是Date类型的地址，优先级队列里面只会比较地址，并且这个地址是new出来的，每次的地址都不一样，就会导致每次的结果也都不一样。所以我们应该自己写一个仿函数来处理这种情况，不应该比较Date类型的地址，而是比较这个指针所指向的内容。如下所示：

```cpp
#include
using namespace std;
#include
class Date
{
	friend ostream& operator<<(ostream& _cout, const Date& d);
public:
	Date(int year = 1900, int month = 1, int day = 1)
		: _year(year)
		, _month(month)
		, _day(day)
	{
	}
	bool operator<(const Date& d) const
	{
		return (_year < d._year) ||
			(_year == d._year && _month < d._month) ||
			(_year == d._year && _month == d._month && _day < d._day);
	}
	bool operator>(const Date& d)const
	{
		return (_year > d._year) ||
			(_year == d._year && _month > d._month) ||
			(_year == d._year && _month == d._month && _day > d._day);
	}
private:
	int _year;
	int _month;
	int _day;
};
ostream& operator<<(ostream& _cout, const Date& d)
{
	_cout << d._year << "-" << d._month << "-" << d._day;
	return _cout;
}
class LessDate {
public:
	bool operator()(Date* d1, Date* d2)
	{
		return *d1 < *d2;
	}
};
int main()
{
	// 大堆，需要用户在自定义类型中提供<的重载
	priority_queue,LessDate> q1;
	q1.push(new Date(2018, 10, 29));
	q1.push(new Date(2018, 10, 28));
	q1.push(new Date(2018, 10, 30));
	cout << *q1.top() << endl;
	q1.pop();
	cout << *q1.top() << endl;
	q1.pop();
	cout << *q1.top() << endl;
	q1.pop();
	return 0;
}
```

