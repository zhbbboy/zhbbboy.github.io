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

# 模拟实现

## vector各函数接口总览
```cpp
namespace cl
{
	//模拟实现vector
	template<class T>
	class vector
	{
	public:
		typedef T* iterator;
		typedef const T* const_iterator;

		//默认成员函数
		vector();                                           //构造函数
		vector(size_t n, const T& val);                     //构造函数
		template<class InputIterator>                      
		vector(InputIterator first, InputIterator last);    //构造函数
		vector(const vector<T>& v);                         //拷贝构造函数
		vector<T>& operator=(const vector<T>& v);           //赋值运算符重载函数
		~vector();                                          //析构函数

		//迭代器相关函数
		iterator begin();
		iterator end();
		const_iterator begin()const;
		const_iterator end()const;

		//容量和大小相关函数
		size_t size()const;
		size_t capacity()const;
		void reserve(size_t n);
		void resize(size_t n, const T& val = T());
		bool empty()const;

		//修改容器内容相关函数
		void push_back(const T& x);
		void pop_back();
		void insert(iterator pos, const T& x);
		iterator erase(iterator pos);
		void swap(vector<T>& v);

		//访问容器相关函数
		T& operator[](size_t i);
		const T& operator[](size_t i)const;

	private:
		iterator _start;        //指向容器的头
		iterator _finish;       //指向有效数据的尾
		iterator _endofstorage; //指向容器的尾
	};
}
```
注：为了防止与标准库当中的vector产生命名冲突，模拟实现时需放在自己的命名空间当中。

## vector当中的成员变量介绍

在vector当中有三个成员变量_start、_ finish、_ endofstorage。
![](../../_Pasted_Image/Pasted%20image%2020260825203336.png)
_ start指向容器的头，_ finish指向容器当中有效数据的尾，_ endofstorage指向整个容器的尾。

## 默认成员函数

### 构造函数1

vector首先支持一个无参的构造函数，对于这个无参的构造函数，我们直接将构造对象的三个成员变量都设置为空指针即可。
```cpp
//构造函数1
vector()
	:_start(nullptr)
	, _finish(nullptr)
	, _endofstorage(nullptr)
{}
```

### 构造函数2

其次，vector还支持使用一段迭代器区间进行对象的构造。因为该迭代器区间可以是其他容器的迭代器区间，也就是说该函数接收到的迭代器的类型是不确定的，所以我们这里需要将该构造函数设计为一个函数模板，在函数体内将该迭代器区间的数据一个个尾插到容器当中即可。
```cpp
//构造函数2
template<class InputIterator> //模板函数
vector(InputIterator first, InputIterator last)
	:_start(nullptr)
	, _finish(nullptr)
	, _endofstorage(nullptr)
{
	//将迭代器区间在[first,last)的数据一个个尾插到容器当中
	while (first != last)
	{
		push_back(*first);
		first++;
	}
}
```
### 构造函数3

此外，vector还支持构造这样一种容器，该容器当中含有n个值为val的数据。对于该构造函数，我们可以先使用reserve函数将容器容量先设置为n，然后使用push_back函数尾插n个值为val的数据到容器当中即可。
```cpp
//构造函数3
vector(size_t n, const T& val)
	:_start(nullptr)
	, _finish(nullptr)
	, _endofstorage(nullptr)
{
	reserve(n); //调用reserve函数将容器容量设置为n
	for (size_t i = 0; i < n; i++) //尾插n个值为val的数据到容器当中
	{
		push_back(val);
	}
}
```
**注意：**  
1）该构造函数知道其需要用于存储n个数据的空间，所以最好用reserve函数一次性开辟好空间，避免调用push_back函数时需要增容多次，导致效率降低。  
2）该构造函数还需要实现两个重载函数。
```cpp
vector(long n, const T& val)
	:_start(nullptr)
	, _finish(nullptr)
	, _endofstorage(nullptr)
{
	reserve(n); //调用reserve函数将容器容量设置为n
	for (size_t i = 0; i < n; i++) //尾插n个值为val的数据到容器当中
	{
		push_back(val);
	}
}
vector(int n, const T& val)
	:_start(nullptr)
	, _finish(nullptr)
	, _endofstorage(nullptr)
{
	reserve(n); //调用reserve函数将容器容量设置为n
	for (int i = 0; i < n; i++) //尾插n个值为val的数据到容器当中
	{
		push_back(val);
	}
}
```
可以看到，这两个重载函数与之不同的就是其参数n的类型不同，但这却是必要的，否则当我们使用以下代码时，编译器 会优先与构造函数2相匹配。
```cpp
vector<int> v(5, 7); //调用构造函数3 ？？？
```
并且因为构造函数2当中对参数first和last进行了解引用（而int类型不能进行解引用操作）而报错。


### 拷贝构造函数

vector的构造函数涉及深拷贝问题，这里提供两种深拷贝的写法：  
**写法一：传统写法**  
拷贝构造的传统写法的思想是我们最容易想到的：先开辟一块与该容器大小相同的空间，然后将该容器当中的数据一个个拷贝过来即可，最后更新_finish和_endofstorage的值即可。
```cpp
//传统写法
vector(const vector<T>& v)
	:_start(nullptr)
	, _finish(nullptr)
	, _endofstorage(nullptr)
{
	_start = new T[v.capacity()]; //开辟一块和容器v大小相同的空间
	for (size_t i = 0; i < v.size(); i++) //将容器v当中的数据一个个拷贝过来
	{
		_start[i] = v[i];
	}
	_finish = _start + v.size(); //容器有效数据的尾
	_endofstorage = _start + v.capacity(); //整个容器的尾
}
```
**注意**： 将容器当中的数据一个个拷贝过来时不能使用memcpy函数，当vector存储的数据是内置类型或无需进行深拷贝的自定义类型时，使用memcpy函数是没什么问题的，但当vector存储的数据是需要进行深拷贝的自定义类型时，使用memcpy函数的弊端就体现出来了。例如，当vector存储的数据是string类的时候。![](../../_Pasted_Image/Pasted%20image%2020260825203625.png)
并且vector当中存储的每一个string都指向自己所存储的字符串。
![](../../_Pasted_Image/Pasted%20image%2020260825203634.png)
如果此时我们使用的是memcpy函数进行拷贝构造的话，那么拷贝构造出来的vector当中存储的每个string的成员变量的值，将与被拷贝的vector当中存储的每个string的成员变量的值相同，即两个vector当中的每个对应的string成员都指向同一个字符串空间。![](../../_Pasted_Image/Pasted%20image%2020260825203643.png)
这显然不是我们得到的结果，那么所给代码是如何解决这个问题的呢？
![](../../_Pasted_Image/Pasted%20image%2020260825203651.png)
代码中看似是使用普通的“=”将容器当中的数据一个个拷贝过来，实际上是调用了所存元素的赋值运算符重载函数，而string类的赋值运算符重载函数就是深拷贝，所以拷贝结果是这样的：![](../../_Pasted_Image/Pasted%20image%2020260825203659.png)
**总结一下：** 如果vector当中存储的元素类型是内置类型（int）或浅拷贝的自定义类型（Date），使用memcpy函数进行进行拷贝构造是没问题的，但如果vector当中存储的元素类型是深拷贝的自定义类型（string），则使用memcpy函数将不能达到我们想要的效果。

**写法二：现代写法**  
拷贝构造函数的现代写法也比较简单，使用范围for（或是其他遍历方式）对容器v进行遍历，在遍历过程中将容器v中存储的数据一个个尾插过来即可。
```cpp
//现代写法
vector(const vector<T>& v)
	:_start(nullptr)
	, _finish(nullptr)
	, _endofstorage(nullptr)
{
	reserve(v.capacity()); //调用reserve函数将容器容量设置为与v相同
	for (auto e : v) //将容器v当中的数据一个个尾插过来
	{
		push_back(e);
	}
}
```
**注意：** 在使用范围for对容器v进行遍历的过程中，变量e就是每一个数据的拷贝，然后将e尾插到构造出来的容器当中。就算容器v当中存储的数据是string类，在e拷贝时也会自动调用string的拷贝构造（深拷贝），所以也能够避免出现与使用memcpy时类似的问题。

### 赋值运算符重载函数

vector的赋值运算符重载当然也涉及深拷贝问题，我们这里也提供两种深拷贝的写法：  
**写法一：传统写法**  
首先判断是否是给自己赋值，若是给自己赋值则无需进行操作。若不是给自己赋值，则先开辟一块和容器v大小相同的空间，然后将容器v当中的数据一个个拷贝过来，最后更新_finish和_endofstorage的值即可。
```cpp
//传统写法
vector<T>& operator=(const vector<T>& v)
{
	if (this != &v) //防止自己给自己赋值
	{
		delete[] _start; //释放原来的空间
		_start = new T[v.capacity()]; //开辟一块和容器v大小相同的空间
		for (size_t i = 0; i < v.size(); i++) //将容器v当中的数据一个个拷贝过来
		{
			_start[i] = v[i];
		}
		_finish = _start + v.size(); //容器有效数据的尾
		_endofstorage = _start + v.capacity(); //整个容器的尾
	}
	return *this; //支持连续赋值
}
```
**注意：** 这里和拷贝构造函数的传统写法类似，也不能使用memcpy函数进行拷贝。

**写法二：现代写法**  
赋值运算符重载的现代写法非常精辟，首先在右值传参时并没有使用引用传参，因为这样可以间接调用vector的拷贝构造函数，然后将这个拷贝构造出来的容器v与左值进行交换，此时就相当于完成了赋值操作，而容器v会在该函数调用结束时自动析构。
```cpp
//现代写法
vector<T>& operator=(vector<T> v) //编译器接收右值的时候自动调用其拷贝构造函数
{
	swap(v); //交换这两个对象
	return *this; //支持连续赋值
}
```
**注意：** 赋值运算符重载的现代写法也是进行的深拷贝，只不过是调用的vector的拷贝构造函数进行的深拷贝，在赋值运算符重载函数当中仅仅是将深拷贝出来的对象与左值进行了交换而已。

### 析构函数
对容器进行析构时，首先判断该容器是否为空容器，若为空容器，则无需进行析构操作，若不为空，则先释放容器存储数据的空间，然后将容器的各个成员变量设置为空指针即可。
```cpp
//析构函数
~vector()
{
	if (_start) //避免对空指针进行释放
	{
		delete[] _start; //释放容器存储数据的空间
		_start = nullptr; //_start置空
		_finish = nullptr; //_finish置空
		_endofstorage = nullptr; //_endofstorage置空
	}
}
```
## 迭代器相关函数

vector当中的迭代器实际上就是容器当中所存储数据类型的指针。

```cpp
typedef T* iterator;
typedef const T* const_iterator;
```
## begin和end
vector当中的begin函数返回容器的首地址，end函数返回容器当中有效数据的下一个数据的地址。
```cpp
iterator begin()
{
	return _start; //返回容器的首地址
}
iterator end()
{
	return _finish; //返回容器当中有效数据的下一个数据的地址
}
```
我们还需要重载一对适用于const对象的begin和end函数，使得const对象调用begin和end函数时所得到的迭代器只能对数据进行读操作，而不能进行修改。

```cpp
const_iterator begin()const
{
	return _start; //返回容器的首地址
}
const_iterator end()const
{
	return _finish; //返回容器当中有效数据的下一个数据的地址
}
```
此时再让我们来看看vector使用迭代器的代码也就一目了然了，实际上就是使用指针遍历容器。
```cpp
vector<int> v(5, 3);
vector<int>::iterator it = v.begin();
while (it != v.end())
{
	cout << *it << " ";
	it++;
}
cout << endl;
```
现在我们实现了迭代器，实际上也就可以使用范围for遍历容器了，因为编译器在编译时会自动将范围for替换为迭代器的形式。
```cpp
vector<int> v(5, 3);
//范围for进行遍历
for (auto e : v)
{
	cout << e << " ";
}
cout << endl;
```
## 容量和大小相关函数

### size和capacity

对照着vector当中三个成员遍历各自的指向，我们可以很容易得出当前容器中的有效数据个数和最大容量。![](../../_Pasted_Image/Pasted%20image%2020260825203939.png)
由于两个指针相减的结果，就是这两个指针之间对应类型的数据个数，所以size可以由_finish - _ start得到，而capacity可以由_ endofstorage - _ start得到。
```cpp
size_t size()const
{
	return _finish - _start; //返回容器当中有效数据的个数
}
size_t capacity()const
{
	return _endofstorage - _start; //返回当前容器的最大容量
}
```
### reserve

**reserve规则：**  
 1、当n大于对象当前的capacity时，将capacity扩大到n或大于n。  
 2、当n小于对象当前的capacity时，什么也不做。

reserve函数的实现思路也是很简单的，先判断所给n是否大于当前容器的最大容量（否则无需进行任何操作），操作时直接开辟一块可以容纳n个数据的空间，然后将原容器当中的有效数据拷贝到该空间，之后将原容器存储数据的空间释放，并将新开辟的空间交给该容器维护，最好更新容器当中各个成员变量的值即可。
 ```cpp
void reserve(size_t n)
{
	if (n > capacity()) //判断是否需要进行操作
	{
		size_t sz = size(); //记录当前容器当中有效数据的个数
		T* tmp = new T[n]; //开辟一块可以容纳n个数据的空间
		if (_start) //判断是否为空容器
		{
			for (size_t i = 0; i < sz; i++) //将容器当中的数据一个个拷贝到tmp当中
			{
				tmp[i] = _start[i];
			}
			delete[] _start; //将容器本身存储数据的空间释放
		}
		_start = tmp; //将tmp所维护的数据交给_start进行维护
		_finish = _start + sz; //容器有效数据的尾
		_endofstorage = _start + n; //整个容器的尾
	}
}
 ```
在reserve函数的实现当中有两个地方需要注意：
**1）在进行操作之前需要提前记录当前容器当中有效数据的个数。**
因为我们最后需要更新_finish指针的指向，而_finish指针的指向就等于_start指针加容器当中有效数据的个数，当_start指针的指向改变后我们再调用size函数通过_finish - _start计算出的有效数据的个数就是一个随机值了。![](../../_Pasted_Image/Pasted%20image%2020260825204115.png)

**2）拷贝容器当中的数据时，不能使用memcpy函数进行拷贝。**
可能你会想，当vector当中存储的是string的时候，虽然使用memcpy函数reserve出来的容器与原容器当中每个对应的string成员都指向同一个字符串空间，但是原容器存储数据的空间不是已经被释放了，相当于现在只有一个容器维护这这些字符串空间，这还有什么影响。
但是不要忘了，当你释放原容器空间的时候，原容器当中存储的每个string在释放时会调用string的析构函数，将其指向的字符串也进行释放，所以使用memcpy函数reserve出来的容器当中的每一个string所指向的字符串实际上是一块已经被释放的空间，访问该容器时就是对内存空间进行非法访问。
![](../../_Pasted_Image/Pasted%20image%2020260825204134.png)
所以说我们还是得用for循环将容器当中的string一个个赋值过来，因为这样能够间接调用string的赋值运算符重载，实现string的深拷贝。![](../../_Pasted_Image/Pasted%20image%2020260825204146.png)
### resize

**resize规则：**  
 1、当n大于当前的size时，将size扩大到n，扩大的数据为val，若val未给出，则默认为容器所存储类型的默认构造函数所构造出来的值。  
 2、当n小于当前的size时，将size缩小到n。
根据resize函数的规则，进入函数我们可以先判断所给n是否小于容器当前的size，若小于，则通过改变_finish的指向，直接将容器的size缩小到n即可，否则先判断该容器是否需要增容，然后再将扩大的数据赋值为val即可。
```cpp
void resize(size_t n, const T& val = T())
{
	if (n < size()) //当n小于当前的size时
	{
		_finish = _start + n; //将size缩小到n
	}
	else //当n大于当前的size时
	{
		if (n > capacity()) //判断是否需要增容
		{
			reserve(n);
		}
		while (_finish < _start + n) //将size扩大到n
		{
			*_finish = val;
			_finish++;
		}
	}
}

```
**注意：** 在C++当中内置类型也可以看作是一个类，它们也有自己的默认构造函数，所以在给resize函数的参数val设置缺省值时，设置为T( )即可。
### empty

empty函数可以直接通过比较容器当中的_start和_finish指针的指向来判断容器是否为空，若所指位置相同，则该容器为空。
```cpp
bool empty()const
{
	return _start == _finish;
}
```
## 修改容器内容相关函数

### push_back

要尾插数据首先得判断容器是否已满，若已满则需要先进行增容，然后将数据尾插到_finish指向的位置，再将_finish++即可。
```cpp
//尾插数据
void push_back(const T& x)
{
	if (_finish == _endofstorage) //判断是否需要增容
	{
		size_t newcapacity = capacity() == 0 ? 4 : 2 * capacity(); //将容量扩大为原来的两倍
		reserve(newcapacity); //增容
	}
	*_finish = x; //尾插数据
	_finish++; //_finish指针后移
}
```
### pop_back

尾删数据之前也得先判断容器是否为空，若为空则做断言处理，若不为空则将_finish–即可。
```cpp
//尾删数据
void pop_back()
{
	assert(!empty()); //容器为空则断言
	_finish--; //_finish指针前移
}
```
### insert

insert函数可以在所给迭代器pos位置插入数据，在插入数据前先判断是否需要增容，然后将pos位置及其之后的数据统一向后挪动一位，以留出pos位置进行插入，最后将数据插入到pos位置即可。
```cpp
//在pos位置插入数据
void insert(iterator pos, const T& x)
{
	if (_finish == _endofstorage) //判断是否需要增容
	{
		size_t len = pos - _start; //记录pos与_start之间的间隔
		size_t newcapacity = capacity() == 0 ? 4 : 2 * capacity(); //将容量扩大为原来的两倍
		reserve(newcapacity); //增容
		pos = _start + len; //通过len找到pos在增容后的容器当中的位置
	}
	//将pos位置及其之后的数据统一向后挪动一位，以留出pos位置进行插入
	iterator end = _finish;
	while (end >= pos + 1)
	{
		*end = *(end - 1);
		end--;
	}
	*pos = x; //将数据插入到pos位置
	_finish++; //数据个数增加一个，_finish后移
}
```
**注意：** 若需要增容，则需要在增容前记录pos与_start之间的间隔，然后通过该间隔确定在增容后的容器当中pos的指向，否则pos还指向原来被释放的空间。

### erase

erase函数可以删除所给迭代器pos位置的数据，在删除数据前需要判断容器释放为空，若为空则需做断言处理，删除数据时直接将pos位置之后的数据统一向前挪动一位，将pos位置的数据覆盖即可。
```cpp
//删除pos位置的数据
iterator erase(iterator pos)
{
	assert(!empty()); //容器为空则断言
	//将pos位置之后的数据统一向前挪动一位，以覆盖pos位置的数据
	iterator it = pos + 1;
	while (it != _finish)
	{
		*(it - 1) = *it;
		it++;
	}
	_finish--; //数据个数减少一个，_finish前移
	return pos;
}
```
### swap 

swap函数用于交换两个容器的数据，我们可以直接调用库当中的swap函数将两个容器当中的各个成员变量进行交换即可。
```cpp
//交换两个容器的数据
void swap(vector<T>& v)
{
	//交换容器当中的各个成员变量
	::swap(_start, v._start);
	::swap(_finish, v._finish);
	::swap(_endofstorage, v._endofstorage);
}
```
**注意：** 在此处调用库当中的swap需要在swap之前加上“::”（作用域限定符），告诉编译器这里优先在全局范围寻找swap函数，否则编译器会认为你调用的就是你正在实现的swap函数（就近原则）。

## 访问容器相关函数
### operator[ ]

vector也支持我们使用“下标+[ ]”的方式对容器当中的数据进行访问，实现时直接返回对应位置的数据即可。
```cpp
T& operator[](size_t i)
{
	assert(i < size()); //检测下标的合法性

	return _start[i]; //返回对应数据
}
const T& operator[](size_t i)const
{
	assert(i < size()); //检测下标的合法性

	return _start[i]; //返回对应数据
}
```
**注意：** 重载运算符[ ]时需要重载一个适用于const容器的，因为const容器通过“下标+[ ]”获取到的数据只允许进行读操作，不能对数据进行修改。