---
title: stack和queue
date: 2026-08-26
tags: []
draft: false
---

# 🐉版介绍和使用
## stack

stack是一种容器适配器，专门用在具有后进先出操作的上下文环境中，其只能从容器的一端进行元素的插入与提取操作。![](../../_Pasted_Image/Pasted%20image%2020260826130345.png)

### stack的定义方式

**方式一：** 使用默认的适配器定义栈。
```cpp
stack<int> st1;
```
**方式二：** 使用特定的适配器定义栈。
```cpp
stack<int, vector<int>> st2;
stack<int, list<int>> st3;
```
**注意：** 如果没有为stack指定特定的底层容器，默认情况下使用deque。

### stack的使用

stack当中常用的成员函数如下：![](../../_Pasted_Image/Pasted%20image%2020260826130420.png)

```cpp
#include <iostream>
#include <vector>
#include <stack>
using namespace std;

int main()
{
	stack<int, vector<int>> st;
	st.push(1);
	st.push(2);
	st.push(3);
	st.push(4);
	cout << st.size() << endl; //4
	while (!st.empty())
	{
		cout << st.top() << " ";
		st.pop();
	}
	cout << endl; //4 3 2 1
	return 0;
}

```

## queue

队列是一种容器适配器，专门用在具有先进先出操作的上下文环境中，其只能从容器的一端插入元素，另一端提取元素。
![](../../_Pasted_Image/Pasted%20image%2020260826130447.png)
### queue的定义方式

**方式一：** 使用默认的适配器定义队列。

```cpp
queue<int> q1;
```
**方式二：** 使用特定的适配器定义队列。

```cpp
queue<int, vector<int>> q2;
queue<int, list<int>> q3;
```
**注意：** 如果没有为queue指定特定的底层容器，默认情况下使用deque。

### queue的使用

queue当中常用的成员函数如下：
![](../../_Pasted_Image/Pasted%20image%2020260826130514.png)

```cpp
#include <iostream>
#include <list>
#include <queue>
using namespace std;

int main()
{
	queue<int, list<int>> q;
	q.push(1);
	q.push(2);
	q.push(3);
	q.push(4);
	cout << q.size() << endl; //4
	while (!q.empty())
	{
		cout << q.front() << " ";
		q.pop();
	}
	cout << endl; //1 2 3 4
	return 0;
}
```

# stack和queue的模拟实现
stack和queue的模拟实现都比较简单，所以这里就一起进行实现。

## 容器适配器
stack和queue有一点需要注意的是，虽然stack和queue中也可以存放元素，但在STL中并没有将其划分在容器的行列，而是将其称为容器适配器，这是因为stack和queue只是对其他容器的接口进行了包装，STL中stack和queue默认使用deque容器。
在stack和queue的类 模板声明当中我们就可以看到，它们的模板参数有两个，第一个是stack和queue当中所存储的元素类型，而另一个就是指定使用的容器类型。只不过当我们不指定使用何种容器的情况下，stack和queue都默认使用deque作为指定容器。
![](../../_Pasted_Image/Pasted%20image%2020260826131136.png)
![](../../_Pasted_Image/Pasted%20image%2020260826131140.png)
**简单理解：** 学过数据结构后我们都知道，stack和queue既可以使用顺序表实现，也可以使用链表实现。在这里我们若是定义一个stack，并指定使用vector容器，则定义出来的stack实际上就是对vector容器进行了包装

## stack的模拟实现

知道了容器适配器后，stack的模拟实现就显得相当简单，我们只需要调用所指定容器的各个成员函数即可实现stack的各个函数接口。
![](../../_Pasted_Image/Pasted%20image%2020260826131203.png)

实现代码如下：
```cpp
namespace cl //防止命名冲突
{
	template<class T, class Container = std::deque<T>>
	class stack
	{
	public:
		//元素入栈
		void push(const T& x)
		{
			_con.push_back(x);
		}
		//元素出栈
		void pop()
		{
			_con.pop_back();
		}
		//获取栈顶元素
		T& top()
		{
			return _con.back();
		}
		const T& top() const
		{
			return _con.back();
		}
		//获取栈中有效元素个数
		size_t size() const
		{
			return _con.size();
		}
		//判断栈是否为空
		bool empty() const
		{
			return _con.empty();
		}
		//交换两个栈中的数据
		void swap(stack<T, Container>& st)
		{
			_con.swap(st._con);
		}
	private:
		Container _con;
	};
}
```

## queue的模拟实现

同样的方式，我们也是通过调用所指定容器的各个成员函数来实现queue的。
![](../../_Pasted_Image/Pasted%20image%2020260826131231.png)
实现代码如下：
```cpp
namespace cl //防止命名冲突
{
	template<class T, class Container = std::deque<T>>
	class queue
	{
	public:
		//队尾入队列
		void push(const T& x)
		{
			_con.push_back(x);
		}
		//队头出队列
		void pop()
		{
			_con.pop_front();
		}
		//获取队头元素
		T& front()
		{
			return _con.front();
		}
		const T& front() const
		{
			return _con.front();
		}
		//获取队尾元素
		T& back()
		{
			return _con.back();
		}
		const T& back() const
		{
			return _con.back();
		}
		//获取队列中有效元素个数
		size_t size() const
		{
			return _con.size();
		}
		//判断队列是否为空
		bool empty() const
		{
			return _con.empty();
		}
		//交换两个队列中的数据
		void swap(queue<T, Container>& q)
		{
			_con.swap(q._con);
		}
	private:
		Container _con;
	};
}
```
