---
title: vector
date: 2026-08-22
tags: []
draft: false
---

# 🐉版
## vector的介绍
1、vector是表示可变大小数组的序列容器。
2、vector就像数组一样，也采用的连续空间来存储元素，这也意味着可以采用下标对vector的元素进行访问。
3、vector与普通数组不同的是，vector的大小是可以动态改变的。
4、当vector需要重新分配大小时，其做法是，分配一个新的数组，然后将全部元素移到这个数组当中，并释放原来的数组空间。
5、vector分配空间策略：vector会分配一些额外的空间以适应可能的增长，因此存储空间比实际需要的存储空间一般更大。不同的库采用不同的策略权衡空间的使用和重新分配，以至于在末尾插入一个元素的时候是在常数的时间复杂度完成的。
6、由于vector采用连续的空间来存储元素，与其他动态序列容器相比，vector在访问元素的时候更加高效，在其末尾添加和删除元素相对高效，而对于不在其末尾进行的删除和插入操作效率则相对较低。

## vector的使用

### vector的定义方式

**方式一：** 构造一个某类 型的空容器。
```cpp
vector<int> v1; //构造int类型的空容器
```

**方式二：** 构造一个含有n个val的某类型容器。
```cpp
vector<int> v2(10, 2); //构造含有10个2的int类型容器
```
**方式三：** 拷贝构造某类型容器的复制品。
```cpp
vector<int> v3(v2); //拷贝构造int类型的v2容器的复制品
```
**方式四：** 使用迭代器拷贝构造某一段内容。
```cpp
vector<int> v4(v2.begin(), v2.end()); //使用迭代器拷贝构造v2容器的某一段内容
```
注意：该方式也可用于拷贝其他容器的某一段内容。
```cpp
string s("hello world");
vector<char> v5(s.begin(), s.end()); //拷贝构造string对象的某一段内容
```

### vector的空间增长问题

#### size和capacity

通过size函数获取当前容器中的有效元素个数，通过capacity函数获取当前容器的最大容量。
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v(10, 2);
	cout << v.size() << endl; //获取当前容器中的有效元素个数
	cout << v.capacity() << endl; //获取当前容器的最大容量
	return 0;
}
```

#### reserve和resize
通过reserse函数改变容器的最大容量，resize函数改变容器中的有效元素个数。

**reserve规则：**  
 1、当所给值大于容器当前的capacity时，将capacity扩大到该值。
 2、当所给值小于容器当前的capacity时，什么也不做。

**resize规则：**  
 1、当所给值大于容器当前的size时，将size扩大到该值，扩大的元素为第二个所给值，若未给出，则默认为0。
 2、当所给值小于容器当前的size时，将size缩小到该值。
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v(10, 2);
	cout << v.size() << endl; //10
	cout << v.capacity() << endl; //10
	v.reserve(20); //改变容器的capacity为20，size不变
	cout << v.size() << endl; //10
	cout << v.capacity() << endl; //20
	v.resize(15); //改变容器的size为15
	cout << v.size() << endl; //15
	cout << v.capacity() << endl; //20
	return 0;
}
```

#### empty
通过empty函数判断当前容器是否为空。
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v(10, 2);
	cout << v.empty() << endl;
	return 0;
}
```

### vector的迭代器使用
![](../../_Pasted_Image/Pasted%20image%2020260822205443.png)

#### begin和end
通过begin函数可以得到容器中第一个元素的正向迭代器，通过end函数可以得到容器中最后一个元素的后一个位置的正向迭代器。  
**正向迭代器遍历容器：**
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v(10, 2);
	//正向迭代器遍历容器
	vector<int>::iterator it = v.begin();
	while (it != v.end())
	{
		cout << *it << " ";
		it++;
	}
	cout << endl;
	return 0;
}
```
#### rbegin和rend

通过rbegin函数可以得到容器中最后一个元素的反向迭代器，通过rend函数可以得到容器中第一个元素的前一个位置的反向迭代器。  
**反向迭代器遍历容器：**
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v(10, 2);
	//反向迭代器遍历容器
	vector<int>::reverse_iterator rit = v.rbegin();
	while (rit != v.rend())
	{
		cout << *rit << " ";
		rit++;
	}
	cout << endl;
	return 0;
}
```

### vector的增删查改

#### push_back和pop_back

通过push_back函数对容器进行尾插，pop_back函数对容器进行尾删。
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v;
	v.push_back(1); //尾插元素1
	v.push_back(2); //尾插元素2
	v.push_back(3); //尾插元素3
	v.push_back(4); //尾插元素4

	v.pop_back(); //尾删元素
	v.pop_back(); //尾删元素
	v.pop_back(); //尾删元素
	v.pop_back(); //尾删元素
	return 0;
}
```

#### insert和erase
通过insert函数可以在所给迭代器位置插入一个或多个元素，通过erase函数可以删除所给迭代器位置的元素，或删除所给迭代器区间内的所有元素（左闭右开）。
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	v.insert(v.begin(), 0); //在容器开头插入0
	
	v.insert(v.begin(), 5, -1); //在容器开头插入5个-1

	v.erase(v.begin()); //删除容器中的第一个元素

	v.erase(v.begin(), v.begin() + 5); //删除在该迭代器区间内的元素（左闭右开）
	
	return 0;
}
```
以上是按位置进行插入或删除元素的方式，若要按值进行插入或删除（在某一特定值位置进行插入或删除），则需要用到find函数。  
**find函数：**
find函数共三个参数，前两个参数确定一个迭代器区间（左闭右开），第三个参数确定所要寻找的值。  
find函数在所给迭代器区间寻找第一个匹配的元素，并返回它的迭代器，若未找到，则返回所给的第二个参数。
```cpp
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

int main()
{
	vector<int> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	vector<int>::iterator pos = find(v.begin(), v.end(), 2); //获取值为2的元素的迭代器
	
	v.insert(pos, 10); //在2的位置插入10

	pos = find(v.begin(), v.end(), 3); //获取值为3的元素的迭代器
	
	v.erase(pos); //删除3

	return 0;
}
```

**注意：** find函数是在算法模块（algorithm）当中实现的，不是vector的成员函数。

#### swap
通过swap函数可以交换两个容器的数据空间，实现两个容器的交换。
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v1(10, 1);
	vector<int> v2(10, 2);

	v1.swap(v2); //交换v1,v2的数据空间

	return 0;
}
```

#### 元素访问
vector当中实现了 [ ] 操作符的重载，因此我们也可以通过“下标+[ ]”的方式对容器当中的元素进行访问。
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v(10, 1);
	//使用“下标+[]”的方式遍历容器
	for (size_t i = 0; i < v.size(); i++)
	{
		cout << v[i] << " ";
	}
	cout << endl;
	return 0;
}
```
我们上面说到vector是支持迭代器的，所以我们还可以用范围for对vector容器进行遍历。（支持迭代器就支持范围for，因为在编译时编译器会自动将范围for替换为迭代器的形式）
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v(10, 1);
	//范围for
	for (auto e : v)
	{
		cout << e << " ";
	}
	cout << endl;
	return 0;
}
```

### vector迭代器失效问题
迭代器的主要作用就是让我们在使用各个容器时不用关心其底层的数据结构，而vector的迭代器在底层实际上就是一个指针。迭代器失效就是指迭代器底层对应指针所指向的空间被销毁了，而指向的是一块已经被释放的空间，如果继续使用已经失效的迭代器，程序可能会崩溃。

#### 迭代器失效问题举例
**实例一：**
```cpp
#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

int main()
{
	vector<int> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	v.push_back(5);
	//v: 1 2 3 4 5
	vector<int>::iterator pos = find(v.begin(), v.end(), 2); //获取值为2的元素的迭代器
	v.insert(pos, 10); //在值为2的元素的位置插入10
	//v: 1 10 2 3 4 5
	v.erase(pos); //删除元素2 ？？？error（迭代器失效）
	//v: 1 2 3 4 5
	return 0;
}
```
在该代码中，我们本意是使用元素2的迭代器在原序列中2的位置插入一个10，然后将2删除，但我们实际上获取的是指向2的指针，当我们在2的位置插入10后，该指针就指向了10，所以我们之后删除的实际上是10，而不是2。

**实例二：**
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v;
	for (size_t i = 1; i <= 6; i++)
	{
		v.push_back(i);
	}
	vector<int>::iterator it = v.begin();
	while (it != v.end())
	{
		if (*it % 2 == 0) //删除容器当中的全部偶数
		{
			v.erase(it);
		}
		it++;
	}
	return 0;
}
```
该代码看上去实际上并没有什么错误，但如果你画图仔细分析，你就会发现该代码的问题所在，迭代器访问到了不属于容器的内存 空间，导致程序崩溃。
![](../../_Pasted_Image/Pasted%20image%2020260822205755.png)
不仅如此，而且在迭代器遍历容器中的元素进行判断时，并没有对1、3、5元素进行判断。

#### 迭代器失效解决方法
使用迭代器时，永远记住一句话：**每次使用前，对迭代器进行重新赋值。**
**实例一解决方案：**
```cpp
#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;

int main()
{
	vector<int> v;
	v.push_back(1);
	v.push_back(2);
	v.push_back(3);
	v.push_back(4);
	v.push_back(5);
	//v: 1 2 3 4 5
	vector<int>::iterator pos = find(v.begin(), v.end(), 2); //获取值为2的元素的迭代器
	v.insert(pos, 10); //在值为2的元素的位置插入10
	//v: 1 10 2 3 4 5
	pos = find(v.begin(), v.end(), 2); //重新获取值为2的元素的迭代器
	v.erase(pos); //删除元素2
	//v: 1 10 3 4 5
	return 0;
}
```
对于实例一，我们在使用迭代器删除元素2时对其进行重新赋值便可以解决。

**实例二解决方案：**
```cpp
#include <iostream>
#include <vector>
using namespace std;

int main()
{
	vector<int> v;
	for (size_t i = 1; i <= 6; i++)
	{
		v.push_back(i);
	}
	vector<int>::iterator it = v.begin();
	while (it != v.end())
	{
		if (*it % 2 == 0) //删除容器当中的全部偶数
		{
			it = v.erase(it); //删除后获取下一个元素的迭代器
		}
		else
		{
			it++; //是奇数则it++
		}
	}
	return 0;
}
```
对于实例二，我们可以接收erase函数的返回值（erase函数返回删除元素的后一个元素的新位置），并且控制代码的逻辑：当元素被删除后继续判断该位置的元素（因为该位置的元素已经更新，需要再次判断）。