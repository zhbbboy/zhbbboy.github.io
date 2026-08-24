---
title: string类
date: 2026-08-22
tags: []
draft: false
---

# 🐉版
## C++STL 详解（一）—— sring类
本次内容大纲：![](../../_Pasted_Image/Pasted%20image%2020260822131925.png)

## 一、string 的定义方式

string类实现了多个构造函数 的重载，常用的构造函数如下：
```cpp
string();  //构造一个空字符串

string(const char* s);  //复制s所指的字符序列

string(const char* s, size_t n);  //复制s所指字符序列的前n个字符

string(size_t n, char c);  //生成n个c字符的字符串

string(const string& str);  //生成str的复制品

string(const string& str, size_t pos, size_t len = npos);  //复制str中从字符位置pos开始并跨越len个字符的部分
```

使用示例：
```cpp
string s1;                     //构造空字符串
string s2("hello string");     //复制"hello string"
string s3("hello string", 3);  //复制"hello string"的前3个字符
string s4(10, 's');            //生成10个's'字符的字符串
string s5(s2);                 //生成s2的复制品
string s6(s2, 0, 4);           //复制s2中从字符位置0开始并跨越4个字符的部分
```

## 二、string的插入

**1、使用push_back进行尾插**

```
void push_back (char c);
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s;
	s.push_back('C');
	s.push_back('S');
	s.push_back('D');
	s.push_back('N');
	cout << s << endl; //CSDN
	return 0;
}
```

2、使用insert插入

```cpp
string& insert (size_t pos, const string& str);
string& insert (size_t pos, const char* s);
iterator insert (iterator p, char c);
```


```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("C"); //C

	//insert(pos, str)在pos位置插入字符串str
	s.insert(1, "S"); //CS

	//insert(pos, string)在pos位置插入string对象
	string t("D");
	s.insert(2, t); //CSD

	//insert(pos, char)在pos位置插入字符char
	s.insert(s.end(), 'N'); //CSDN
	
	cout << s << endl; //CSDN
	return 0;
}
```

## 三、string的拼接

**使用append函数完成string的拼接：**
```
string& append (const string& str);  
string& append (const char* s);  
string& append (size_t n, char c);
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s1("I");
	string s2(" like");

	//append(string)完成两个string对象的拼接
	s1.append(s2); //I like

	//append(str)完成string对象和字符串str的拼接
	s1.append(" C++"); //I like C++

	//append(n, char)将n个字符char拼接到string对象后面
	s1.append(3, '!'); //I like C++!!!
	
	cout << s1 << endl; //I like C++!!!
	return 0;
}
```
## 四、string的删除

**1、使用pop_back进行尾删**

```
void pop_back();
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("C++");
	s.pop_back();
	s.pop_back();
	cout << s << endl; //C
	return 0;
}
```

**2、使用erase删除**

```cpp
string& erase (size_t pos = 0, size_t len = npos);  
iterator erase (iterator p);
iterator erase (iterator first, iterator last);
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("I like C++!!!");

	//erase(pos, n)删除pos位置开始的n个字符
	s.erase(8, 5); //I like C

	//erase(pos)删除pos位置的字符
	s.erase(s.end()-1); //I like

	//erase(pos1, pos2)删除[pos1pos2)上所有字符
	s.erase(s.begin() + 1, s.end()); //I

	cout << s << endl; //I
	return 0;
}
```

## 五、string的查找
**1、使用find函数正向搜索第一个匹配项**
```cpp
size_t find (const string& str, size_t pos = 0) const;
size_t find (const char* s, size_t pos = 0) const;
size_t find (char c, size_t pos = 0) const;
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s1("http://www.cplusplus.com/reference/string/string/find/");

	//find(string)正向搜索与string对象所匹配的第一个位置
	string s2("www");
	size_t pos1 = s1.find(s2);
	cout << pos1 << endl; //7

	//find(str)正向搜索与字符串str所匹配的第一个位置
	char str[] = "cplusplus.com";
	size_t pos2 = s1.find(str);
	cout << pos2 << endl;  //11

	//find(char)正向搜索与字符char所匹配的第一个位置
	size_t pos3 = s1.find(':');
	cout << pos3 << endl; //4
	return 0;
}
```

**2、使用rfind函数反向搜索第一个匹配项**
```cpp
size_t rfind (const string& str, size_t pos = npos) const;
size_t rfind (const char* s, size_t pos = npos) const;
size_t rfind (char c, size_t pos = npos) const;
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s1("http://www.cplusplus.com/reference/string/string/find/");

	//rfind(string)反向搜索与string对象所匹配的第一个位置
	string s2("string");
	size_t pos1 = s1.rfind(s2);
	cout << pos1 << endl; //42

	//rfind(str)反向搜索与字符串str所匹配的第一个位置
	char str[] = "reference";
	size_t pos2 = s1.rfind(str);
	cout << pos2 << endl;  //25

	//rfind(char)反向搜索与字符char所匹配的第一个位置
	size_t pos3 = s1.rfind('/');
	cout << pos3 << endl; //53
	return 0;
}
```

## 六、string的比较

**使用compare函数完成比较：**
```cpp
int compare (const string& str) const;
int compare (size_t pos, size_t len, const string& str) const;
int compare (size_t pos, size_t len, const string& str, size_t subpos, size_t sublen) const;
```
**比较规则：**  
 1、比较字符串中第一个不匹配的字符值较小，或者所有比较字符都匹配，但比较字符串较短，则返回小于0的值。  
 2、比较字符串中第一个不匹配的字符值较大，或者所有比较字符都匹配，但比较字符串较长，则返回大于0的值。  
 3、比较的两个字符串相等，则返回0。
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s1("hello world");
	string s2("hello CSDN");

	//"hello world"和"hello CSDN"比较
	cout << s1.compare(s2) << endl; //1

	//"ell"和"hello CSDN"比较
	cout << s1.compare(1, 3, s2) << endl; //-1

	//"hello"和"hello"比较
	cout << s1.compare(0, 4, s2, 0, 4) << endl; //0

	return 0;
}
```
**注意**：除了支持string类之间进行比较，compare函数还支持string类和字符串进行比较。

## 七、string的替换

**使用replace函数完成string的替换：**
```cpp
string& replace (size_t pos, size_t len, const char* s);  
string& replace (size_t pos, size_t len, size_t n, char c);
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("hello world");

	//replace(pos, len, str)将pos位置开始的len个字符替换为字符串str
	s.replace(6, 4, "CSDN"); //hello CSDNd

	//replace(pos, len, n, char)将pos位置开始的len个字符替换为n个字符char
	s.replace(10, 1, 3, '!'); //hello CSDN!!!

	cout << s << endl;
	return 0;
}
```

## 八、string的交换

**使用swap函数完成两个string类的交换：**
```cpp
void swap (string& x, string& y);  
void swap (string& str);
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s1("hello");
	string s2("CSDN");
	
	//使用string类的成员函数swap交换s1和s2
	s1.swap(s2);
	cout << s1 << endl; //CSDN
	cout << s2 << endl; //hello

	//使用非成员函数swap交换s1和s2
	swap(s1, s2);
	cout << s1 << endl; //hello
	cout << s2 << endl; //CSDN
	return 0;
}
```

## 九、string的大小和容量

**1、使用size函数或length函数获取当前有效字符的个数**
```cpp
size_t size() const;  
size_t length() const;
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("CSDN");
	cout << s.size() << endl; //4
	cout << s.length() << endl; //4
	return 0;
}
```

**2、使用max_size函数获取string对象对多可包含的字符数**
```
size_t max_size() const;
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("CSDN");
	cout << s.max_size() << endl; //4294967294
	return 0;
}
```

**3、使用capacity函数获取当前对象所分配的存储空间的大小**

```
 size_t capacity() const;
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("CSDN");
	cout << s.capacity() << endl; //15
	return 0;
}
```
**4、使用resize改变当前对象的有效字符的个数**

```
void resize (size_t n);  
void resize (size_t n, char c);
```

**resize规则：**  
 1、当n大于对象当前的size时，将size扩大到n，扩大的字符为c，若c未给出，则默认为’\0’。  
 2、当n小于对象当前的size时，将size缩小到n。
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s1("CSDN");
	//resize(n)n大于对象当前的size时，将size扩大到n，扩大的字符默认为'\0'
	s1.resize(20);
	cout << s1 << endl; //CSDN
	cout << s1.size() << endl; //20
	cout << s1.capacity() << endl; //31

	string s2("CSDN");
	//resize(n, char)n大于对象当前的size时，将size扩大到n，扩大的字符为char
	s2.resize(20, 'x');
	cout << s2 << endl; //CSDNxxxxxxxxxxxxxxxx
	cout << s2.size() << endl; //20
	cout << s2.capacity() << endl; //31

	string s3("CSDN");
	//resize(n)n小于对象当前的size时，将size缩小到n
	s3.resize(2);
	cout << s3 << endl; //CS
	cout << s3.size() << endl; //2
	cout << s3.capacity() << endl; //15
	return 0;
}
```
**注意**：若给出的n大于对象当前的capacity，则capacity也会根据自己的增长规则进行扩大。

## 5、使用reserve改变当前对象的容量大小

```
void reserve (size_t n = 0);
```

**reserve规则：**
 1、当n大于对象当前的capacity时，将capacity扩大到n或大于n。
 2、当n小于对象当前的capacity时，什么也不做。
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("CSDN");
	cout << s << endl; //CSDN
	cout << s.size() << endl; //4
	cout << s.capacity() << endl; //15

	//reverse(n)当n大于对象当前的capacity时，将当前对象的capacity扩大为n或大于n
	s.reserve(20); 
	cout << s << endl; //CDSN
	cout << s.size() << endl; //4
	cout << s.capacity() << endl; //31

	//reverse(n)当n小于对象当前的capacity时，什么也不做
	s.reserve(2);
	cout << s << endl; //CDSN
	cout << s.size() << endl; //4
	cout << s.capacity() << endl; //31
	return 0;
}
```
**注意**：此函数对字符串的size没有影响，并且无法更改其内容。

**6、使用clear删除对象的内容，删除后对象变为空字符串**

```
 void clear();
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("CSDN");

	//clear()删除对象的内容，该对象将变为空字符串
	s.clear();
	cout << s << endl; //空字符串
	return 0;
}
```

**7、使用empty判断对象是否为空**

```
 bool empty() const;
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("CSDN");
	cout << s.empty() << endl; //0

	//clear()删除对象的内容，该对象将变为空字符串
	s.clear();
	cout << s.empty() << endl; //1
	return 0;
}
```

## 十、string中元素的访问

**1、[ ]+下标**  
 因为string类对[ ]运算符进行了重载，所以我们可以直接使用[ ]+下标访问对象中的元素。并且该重载使用的是引用返回，所以我们可以通过[ ]+下标修改对应位置的元素。
```
   char& operator[] (size_t pos);  
const char& operator[] (size_t pos) const;
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("CSDN");
	//[]+下标访问对象元素
	for (size_t i = 0; i < s.size(); i++)
	{
		cout << s[i];
	}
	cout << endl;

	//[]+下标修改对象元素内容
	for (size_t i = 0; i < s.size(); i++)
	{
		s[i] = 'x';
	}
	cout << s << endl; //xxxx
	return 0;
}
```

**2、使用at访问对象中的元素**  
 因为at函数也是使用的引用返回，所以我们也可以通过at函数修改对应位置的元素。
 ```
    char& at (size_t pos);  
const char& at (size_t pos) const;
 ```
 
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("CSDN");
	for (size_t i = 0; i < s.size(); i++)
	{
		//at(pos)访问pos位置的元素
		cout << s.at(i);
	}
	cout << endl;

	for (size_t i = 0; i < s.size(); i++)
	{
		//at(pos)访问pos位置的元素，并对其进行修改
		s.at(i) = 'x';
	}
	cout << s << endl; //xxxx
	return 0;
}

```
**3、使用范围for访问对象中的元素**  
 需要特别注意的是：若是需要通过范围for修改对象的元素，则用于接收元素的变量e的类型必须是引用类型，否则e只是对象元素的拷贝，对e的修改不会影响到对象的元素。
 ```cpp
 #include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("CSDN");
	//使用范围for访问对象元素
	for (auto e : s)
	{
		cout << e;
	}
	cout << endl; //CSDN

	//使用范围for访问对象元素，并对其进行修改
	for (auto& e : s) //需要修改对象的元素，e必须是引用类型
	{
		e = 'x';
	}
	cout << s << endl; //xxxx
	return 0;
}
 ```
 **4、使用迭代器访问对象中的元素**
 ```cpp
 #include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("CSDN");
	
	//使用迭代器访问对象元素
	string::iterator it1 = s.begin();
	while (it1 != s.end())
	{
		cout << *it1;
		it1++;
	}
	cout << endl; //CSDN

	//使用迭代器访问对象元素，并对其进行修改
	string::iterator it2 = s.begin();
	while (it2 != s.end())
	{
		*it2 += 1;
		it2++;
	}
	cout << s << endl; //DTEO
	return 0;
}
 ```
 
##  十一、string中运算符的使用

**1、operator=**  
 string类中对=运算符进行了重载，重载后的=运算符支持string类的赋值、字符串的赋值以及字符的赋值。

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s1;
	string s2("CSDN");

	//支持string类的赋值
	s1 = s2;
	cout << s1 << endl; //CSDN

	//支持字符串的赋值
	s1 = "hello";
	cout << s1 << endl;  //hello

	//支持字符的赋值
	s1 = 'x';
	cout << s1 << endl; //x
	return 0;
}
```

**2、operator+=**  
 string类中对+=运算符进行了重载，重载后的+=运算符支持string类的复合赋值、字符串的复合赋值以及字符复合的赋值。
 ```cpp
 #include <iostream>
#include <string>
using namespace std;
int main()
{
	string s1;
	string s2("hello");

	//支持string类的复合赋值
	s1 += s2;
	cout << s1 << endl; //hello

	//支持字符串的复合赋值
	s1 += " CSDN";
	cout << s1 << endl; //hello CSDN

	//支持字符的复合赋值
	s1 += '!';
	cout << s1 << endl; //hello CSDN!
	return 0;
}
 ```

**3、operator+**
 string类中对+运算符进行了重载，重载后的+运算符支持以下几种类型的操作：
 string类 + string类
 string类 + 字符串
 字符串 + string类
 string类 + 字符
 字符 + string类
它们相加后均返回一个string类对象。
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s;
	string s1("super");
	string s2("man");
	char str[] = "woman";
	char ch = '!';

	//string类 + string类
	s = s1 + s2;
	cout << s << endl; //superman

	//string类 + 字符串
	s = s1 + str;
	cout << s << endl; //superwoman

	//字符串 + string类
	s = str + s1;
	cout << s << endl; //womansuper

	//string类 + 字符
	s = s1 + ch;
	cout << s << endl; //super!
	
	//字符 + string类
	s = ch + s1;
	cout << s << endl; //!super
	return 0;
}
```
**4、operator>> 和 operator<<**  
 string类中也对>>和<<运算符进行了重载，这就是为什么我们可以直接使用>>和<<对string类进行输入和输出的原因。
 ```
istream& operator>> (istream& is, string& str);  
ostream& operator<< (ostream& os, const string& str);
 ```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s;
	cin >> s; //输入
	cout << s << endl; //输出
	return 0;
}
```
5、relational operators
 string类中还对一系列关系运算符进行了重载，它们分别是 == 、!=、<、<=、>、>=。重载后的关系运算符支持string类和string类之间的关系比较、string类和字符串之间的关系比较、字符串和string类之间的关系比较。
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s1("abcd");
	string s2("abde");
	cout << (s1 > s2) << endl; //0
	cout << (s1 < s2) << endl; //1
	cout << (s1 == s2) << endl; //0
	return 0;
}
```
**注意**：这些重载的关系比较运算符所比较的都是对应字符的ASCII码 值。

## 十二、string中与迭代器相关的函数

**1、与正向迭代器相关的函数**  
**begin函数**：返回一个指向字符串第一个字符的迭代器。

```
函数原型：
    iterator begin();
 const_iterator begin() const;
```

**end函数：**返回一个指向字符串结束字符的迭代器，即’\0’。

```
函数原型：
    iterator end();
 const_iterator end() const;
```

使用示例：
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("hello string");

	//正向迭代器
	string::iterator it = s.begin();
	while (it != s.end())
	{
		cout << *it;
		it++;
	}
	cout << endl; //hello string

	return 0;
}
```
**2、与反向迭代器相关的函数**  
**rbegin函数**：返回指向字符串最后一个字符的反向迭代器。

```
 函数原型：  
     reverse_iterator rbegin();  
  const_reverse_iterator rbegin() const;
```


**rend函数**：返回指向字符串第一个字符前面的理论元素的反向迭代器。

```
 函数原型：  
     reverse_iterator rend();  
  const_reverse_iterator rend() const;
```
使用示例：
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("hello string");

	//反向迭代器
	string::reverse_iterator rit = s.rbegin();
	while (rit != s.rend())
	{
		cout << *rit;
		rit++;
	}
	cout << endl; //gnirts olleh

	return 0;
}
```

## 十三、string与字符串之间的转换

**1、将字符串转换为string**  
 将字符串转换为string很简单，在前面讲string的定义方式时就有说到。
 ```cpp
 #include <iostream>
#include <string>
using namespace std;
int main()
{
	//方式一
	string s1("hello world");

	//方式二
	char str[] = "hello world";
	string s2(str);

	cout << s1 << endl; //hello world
	cout << s2 << endl; //hello world
	return 0;
}
 ```

**2、使用c_str或data将string转换为字符串**
```
 const char* c_str() const;  
 const char* data() const;
```

区别：
- 在C++98中，c_str()返回 const char* 类型，返回的字符串会以空字符结尾。
- 在C++98中，data()返回 const char* 类型，返回的字符串不以空字符结尾。
- 但是在C++11版本中，c_str()与data()用法相同。
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("hello world ");
	const char* str1 = s.data();
	const char* str2 = s.c_str();

	cout << str1 << endl;
	cout << str2 << endl;
	return 0;
}
```

## 十四、string中子字符串的提取

**1、使用substr函数提取string中的子字符串**

```
 string substr (size_t pos = 0, size_t len = npos) const;
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s1("abcdef");
	string s2;

	//substr(pos, n)提取pos位置开始的n个字符序列作为返回值
	s2 = s1.substr(2, 4);
	cout << s2 << endl; //cdef
	return 0;
}

```

**2、使用copy函数将string的子字符串复制到字符数组中**

```
 size_t copy (char* s, size_t len, size_t pos = 0) const;
```

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s("abcdef");
	char str[20];

	//copy(str, n, pos)复制pos位置开始的n个字符到str字符串
	size_t length = s.copy(str, 4, 2);
	//copy函数不会在复制内容的末尾附加'\0'，需要手动加
	str[length] = '\0';
	cout << str << endl; //dcef
	return 0;
}
```

## 十五、string中的getline函数

 我们知道，使用>>进行输入操作时，当>>读取到空格便会停止读取，基于此，我们将不能用>>将一串含有空格的字符串读入到string对象中。
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s;
	cin >> s; //输入：hello CSDN
	cout << s << endl; //输出：hello
	return 0;
}
```
 这时，我们就需要用getline函数完成一串含有空格的字符串的读取操作了。

**用法一：**
```
 istream& getline (istream& is, string& str);
```
 getline函数将从is中提取到的字符存储到str中，直到读取到换行符’\n’为止。

```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s;
	getline(cin, s); //输入：hello CSDN
	cout << s << endl; //输出：hello CSDN
	return 0;
}
```

```
istream& getline (istream& is, string& str, char delim);
```

getline函数将从is中提取到的字符存储到str中，直到读取到分隔符delim或换行符’\n’为止。
```cpp
#include <iostream>
#include <string>
using namespace std;
int main()
{
	string s;
	getline(cin, s, 'D'); //输入：hello CSDN
	cout << s << endl; //输出：hello CS
	return 0;
}
```

完整思维导图：
![](../../_Pasted_Image/Pasted%20image%2020260822133502.png)