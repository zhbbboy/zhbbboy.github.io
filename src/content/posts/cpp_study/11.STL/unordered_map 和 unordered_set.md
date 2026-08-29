---
title: unordered_map 和 unordered_set
date: 2026-08-29
tags: []
draft: false
---

 在 C++ 的标准模板库（STL）中，unordered_map和unordered_set是非常实用的容器，它们基于哈希表实现，能够提供高效的查找、插入和删除操作。在 SGI - STL30 版本中，虽然没有标准的unordered_map和unordered_set，但有类 似功能的hash_map和hash_set，它们作为非标准容器存在。

### 模拟实现的基础 —— 哈希表

首先，定义哈希表的节点结构`HashNode`。每个节点包含数据`_data`和指向下一个节点的指针`_next`，用于解决哈希冲突（链地址法）。
```cpp
namespace zdf {
// 哈希表节点定义
template<class T>
struct HashNode {
    T _data;
    HashNode<T>* _next;
    // 构造函数，初始化数据和指针
    HashNode(const T& data) : _data(data), _next(nullptr) {}
};
}
```

**功能**：这是哈希表中节点的基本结构，用于存储数据和指向下一个节点的指针，采用链表法解决哈希冲突。
构造函数 HashNode(const T& data)：
**参数**：data 是要存储在节点中的数据，类型为 T。
**作用**：初始化节点的数据成员 _ data 为传入的 data，并将指向下一个节点的指针 _ next 初始化为 nullptr。

#### `1. HashTable` 类

接着，定义哈希表类`HashTable`。它包含多个成员变量和成员函数，以实现哈希表的各种功能。
```cpp
namespace zdf {
template<class K, class T, class KeyOfT, class Hash>
class HashTable {
    // 定义节点类型
    typedef HashNode<T> Node;
public:
    // 定义迭代器类型
    typedef HTIterator<K, T, T*, T&, KeyOfT, Hash> Iterator;
    // 定义常量迭代器类型
    typedef HTIterator<K, T, const T*, const T&, KeyOfT, Hash> ConstIterator;
 
    // 返回哈希表起始位置的迭代器
    Iterator Begin() {
        if (_n == 0) {
            return End();
        }
        for (size_t i = 0; i < _tables.size(); ++i) {
            Node* cur = _tables[i];
            if (cur) {
                return Iterator(cur, this);
            }
        }
        return End();
    }
 
    // 返回哈希表结束位置的迭代器
    Iterator End() {
        return Iterator(nullptr, this);
    }
 
    // 返回常量哈希表起始位置的迭代器
    ConstIterator Begin() const {
        if (_n == 0) {
            return End();
        }
        for (size_t i = 0; i < _tables.size(); ++i) {
            Node* cur = _tables[i];
            if (cur) {
                return ConstIterator(cur, this);
            }
        }
        return End();
    }
 
    // 返回常量哈希表结束位置的迭代器
    ConstIterator End() const {
        return ConstIterator(nullptr, this);
    }
 
    // 插入数据到哈希表
    pair<Iterator, bool> Insert(const T& data) {
        KeyOfT kot;
        Iterator it = Find(kot(data));
        if (it != End()) {
            return make_pair(it, false);
        }
        Hash hs;
        size_t hashi = hs(kot(data)) % _tables.size();
        // 负载因子达到1时进行扩容
        if (_n == _tables.size()) {
            vector<Node*> newtables(__stl_next_prime(_tables.size() + 1), nullptr);
            for (size_t i = 0; i < _tables.size(); ++i) {
                Node* cur = _tables[i];
                while (cur) {
                    Node* next = cur->_next;
                    // 计算在新表中的位置并插入
                    size_t newHashi = hs(kot(cur->_data)) % newtables.size();
                    cur->_next = newtables[newHashi];
                    newtables[newHashi] = cur;
                    cur = next;
                }
                _tables[i] = nullptr;
            }
            _tables.swap(newtables);
        }
        // 在当前位置插入新节点
        Node* newnode = new Node(data);
        newnode->_next = _tables[hashi];
        _tables[hashi] = newnode;
        ++_n;
        return make_pair(Iterator(newnode, this), true);
    }
 
    // 在哈希表中查找数据
    Iterator Find(const K& key) {
        KeyOfT kot;
        Hash hs;
        size_t hashi = hs(key) % _tables.size();
        Node* cur = _tables[hashi];
        while (cur) {
            if (kot(cur->_data) == key) {
                return Iterator(cur, this);
            }
            cur = cur->_next;
        }
        return End();
    }
 
    // 从哈希表中删除数据
    bool Erase(const K& key) {
        KeyOfT kot;
        Hash hs;
        size_t hashi = hs(key) % _tables.size();
        Node* prev = nullptr;
        Node* cur = _tables[hashi];
        while (cur) {
            if (kot(cur->_data) == key) {
                if (prev == nullptr) {
                    _tables[hashi] = cur->_next;
                }
                else {
                    prev->_next = cur->_next;
                }
                delete cur;
                --_n;
                return true;
            }
            prev = cur;
            cur = cur->_next;
        }
        return false;
    }
private:
    // 计算下一个素数，用于扩容
    inline unsigned long __stl_next_prime(unsigned long n) {
        static const int __stl_num_primes = 28;
        static const unsigned long __stl_prime_list[__stl_num_primes] = {
            53, 97, 193, 389, 769,
            1543, 3079, 6151, 12289, 24593,
            49157, 98317, 196613, 393241, 786433,
            1572869, 3145739, 6291469, 12582917, 25165843,
            50331653, 100663319, 201326611, 402653189, 805306457,
            1610612741, 3221225473, 4294967291
        };
        const unsigned long* first = __stl_prime_list;
        const unsigned long* last = __stl_prime_list + __stl_num_primes;
        const unsigned long* pos = lower_bound(first, last, n);
        return pos == last? *(last - 1) : *pos;
    }
    // 哈希表存储数据的桶，是一个指针数组
    vector<Node*> _tables;
    // 哈希表中存储数据的个数
    size_t _n = 0;
};
}
```

##### 1.1 迭代器相关函数

###### `Iterator Begin()`
```cpp
Iterator Begin() {
    if (_n == 0) {
        return End();
    }
    for (size_t i = 0; i < _tables.size(); ++i) {
        Node* cur = _tables[i];
        if (cur) {
            return Iterator(cur, this);
        }
    }
    return End();
}
```
**功能**：返回哈希表起始位置的迭代器。
**步骤：**
检查哈希表中元素数量 _ n 是否为 0，如果是则直接返回表示结束位置的迭代器。
遍历哈希表的每个桶（_ tables），找到第一个不为空的桶。
如果找到不为空的桶，返回指向该桶第一个节点的迭代器。
如果遍历完所有桶都没有找到非空桶，则返回表示结束位置的迭代器。

**`Iterator End()`**
```cpp
Iterator End() {
    return Iterator(nullptr, this);
}
```

- **功能**：返回哈希表结束位置的迭代器，通常用一个指向 `nullptr` 的迭代器表示。
- **步骤**：创建一个指向 `nullptr` 的迭代器并返回。

**`ConstIterator Begin() const` 和 `ConstIterator End() const`**
- **功能**：与上述 `Begin()` 和 `End()` 类似，但用于常量哈希表，返回常量迭代器。
- **实现细节**：实现逻辑与非常量版本相同，只是返回的是常量迭代器。

##### 1.2 插入函数 `Insert`
```cpp
pair<Iterator, bool> Insert(const T& data) {
    KeyOfT kot;
    Iterator it = Find(kot(data));
    if (it != End()) {
        return make_pair(it, false);
    }
    Hash hs;
    size_t hashi = hs(kot(data)) % _tables.size();
    if (_n == _tables.size()) {
        vector<Node*> newtables(__stl_next_prime(_tables.size() + 1), nullptr);
        for (size_t i = 0; i < _tables.size(); ++i) {
            Node* cur = _tables[i];
            while (cur) {
                Node* next = cur->_next;
                size_t newHashi = hs(kot(cur->_data)) % newtables.size();
                cur->_next = newtables[newHashi];
                newtables[newHashi] = cur;
                cur = next;
            }
            _tables[i] = nullptr;
        }
        _tables.swap(newtables);
    }
    Node* newnode = new Node(data);
    newnode->_next = _tables[hashi];
    _tables[hashi] = newnode;
    ++_n;
    return make_pair(Iterator(newnode, this), true);
}
```

- **功能**：向哈希表中插入一个新的数据项。
- **步骤**：
1. 使用 Find 函数检查数据是否已经存在于哈希表中，如果存在则返回该元素的迭代器和 false。
2. 计算数据的哈希值，确定其在哈希表中的桶位置 hashi。
3. 检查负载因子（元素数量 _ n 与桶数量 _ tables.size() 相等），如果达到阈值则进行扩容操作：
   - 创建一个新的更大的哈希表 newtables，桶的数量为下一个素数。
   - 遍历原哈希表的每个桶，将每个节点重新计算哈希值并插入到新的哈希表中。
   - 交换原哈希表和新哈希表的指针。
4. 创建一个新的节点 newnode，将其插入到对应桶的头部。
5. 元素数量  _ n 加 1。
6. 返回新插入节点的迭代器和 true。

##### 1.3 查找函数 `Find`
```cpp
Iterator Find(const K& key) {
    KeyOfT kot;
    Hash hs;
    size_t hashi = hs(key) % _tables.size();
    Node* cur = _tables[hashi];
    while (cur) {
        if (kot(cur->_data) == key) {
            return Iterator(cur, this);
        }
        cur = cur->_next;
    }
    return End();
}
```

- **功能**：在哈希表中查找指定键 `key` 对应的数据项。
- **步骤**：
  1. 计算键的哈希值，确定其在哈希表中的桶位置 `hashi`。
  2. 遍历该桶对应的链表，检查每个节点的数据是否与键匹配。
  3.  如果找到匹配的节点，则返回指向该节点的迭代器。
  4. 如果遍历完链表都没有找到匹配的节点，则返回表示结束位置的迭代器。

##### 1.4 删除函数 `Erase`
```cpp
bool Erase(const K& key) {
    KeyOfT kot;
    Hash hs;
    size_t hashi = hs(key) % _tables.size();
    Node* prev = nullptr;
    Node* cur = _tables[hashi];
    while (cur) {
        if (kot(cur->_data) == key) {
            if (prev == nullptr) {
                _tables[hashi] = cur->_next;
            }
            else {
                prev->_next = cur->_next;
            }
            delete cur;
            --_n;
            return true;
        }
        prev = cur;
        cur = cur->_next;
    }
    return false;
}
```
**功能**：从哈希表中删除指定键 key 对应的数据项。
**步骤**：
计算键的哈希值，确定其在哈希表中的桶位置 hashi。
遍历该桶对应的链表，查找键匹配的节点。
如果找到匹配的节点：
	如果该节点是链表的第一个节点，则将桶的头指针指向该节点的下一个节点。
	否则，将前一个节点的 _next 指针指向该节点的下一个节点。
	删除该节点，并将元素数量 _n 减 1。
	返回 true 表示删除成功。
如果遍历完链表都没有找到匹配的节点，则返回 false 表示删除失败。

##### 1.5 辅助函数 `__stl_next_prime`
```cpp
inline unsigned long __stl_next_prime(unsigned long n) {
    static const int __stl_num_primes = 28;
    static const unsigned long __stl_prime_list[__stl_num_primes] = {
        53, 97, 193, 389, 769,
        1543, 3079, 6151, 12289, 24593,
        49157, 98317, 196613, 393241, 786433,
        1572869, 3145739, 6291469, 12582917, 25165843,
        50331653, 100663319, 201326611, 402653189, 805306457,
        1610612741, 3221225473, 4294967291
    };
    const unsigned long* first = __stl_prime_list;
    const unsigned long* last = __stl_prime_list + __stl_num_primes;
    const unsigned long* pos = lower_bound(first, last, n);
    return pos == last? *(last - 1) : *pos;
}
```

- **功能**：计算大于等于 `n` 的下一个素数，用于哈希表扩容时确定新的桶数量。
- **步骤**：
    1. 定义一个素数列表 `__stl_prime_list`。
    2. 使用 `lower_bound` 函数在素数列表中查找第一个大于等于 `n` 的素数。
    3. 如果找到则返回该素数，否则返回素数列表中的最后一个素数。

为了实现哈希表的迭代器功能，定义迭代器类`HTIterator`。
#### `2. HTIterator` 类
```cpp
namespace zdf {
template<class K, class T, class Ptr, class Ref, class KeyOfT, class Hash>
struct HTIterator {
    // 定义节点类型
    typedef HashNode<T> Node;
    // 定义自身类型
    typedef HTIterator<K, T, Ptr, Ref, KeyOfT, Hash> Self;
 
    Node* _node;
    const HashTable<K, T, KeyOfT, Hash>* _pht;
 
    // 构造函数，初始化节点指针和哈希表指针
    HTIterator(Node* node, const HashTable<K, T, KeyOfT, Hash>* pht) : _node(node), _pht(pht) {}
 
    // 解引用操作符重载，返回节点数据的引用
    Ref operator*() {
        return _node->_data;
    }
 
    // 箭头操作符重载，返回节点数据的指针
    Ptr operator->() {
        return &_node->_data;
    }
 
    // 不等于操作符重载，用于比较两个迭代器
    bool operator!=(const Self& s) {
        return _node != s._node;
    }
 
    // 前置递增操作符重载，移动到下一个节点
    Self& operator++() {
        if (_node->_next) {
            _node = _node->_next;
        }
        else {
            KeyOfT kot;
            Hash hs;
            size_t hashi = hs(kot(_node->_data)) % _pht->_tables.size();
            ++hashi;
            while (hashi < _pht->_tables.size()) {
                if (_pht->_tables[hashi]) {
                    break;
                }
                ++hashi;
            }
            if (hashi == _pht->_tables.size()) {
                _node = nullptr;
            }
            else {
                _node = _pht->_tables[hashi];
            }
        }
        return *this;
    }
};
}
```

##### 2.1 构造函数  `HTIterator(Node* node, const HashTable<K, T, KeyOfT, Hash>* pht)`
```cpp
HTIterator(Node* node, const HashTable<K, T, KeyOfT, Hash>* pht) : _node(node), _pht(pht) {}

```
- **功能**：初始化迭代器，将节点指针 `node` 和哈希表指针 `pht` 赋值给成员变量。
- **参数**：
    - `node`：指向当前节点的指针。
    - `pht`：指向哈希表的指针。

##### 2.2 解引用操作符 `operator*()`
```cpp
Ref operator*() {
    return _node->_data;
}
```
- **功能**：返回当前迭代器指向节点的数据的引用。
- **步骤**：直接返回节点的数据成员 `_data`。

##### 2.3 箭头操作符 `operator->()`

```cpp
Ptr operator->() {    
	return &_node->_data;
}
```

##### 2.4 不等于操作符 `operator!=`

```cpp
bool operator!=(const Self& s) {    
	return _node != s._node;
}
```
- **功能**：比较两个迭代器是否不相等。
- **步骤**：比较两个迭代器指向的节点指针是否不同。

##### 2.5 前置递增操作符 `operator++()`
```cpp
Self& operator++() {
    if (_node->_next) {
        _node = _node->_next;
    }
    else {
        KeyOfT kot;
        Hash hs;
        size_t hashi = hs(kot(_node->_data)) % _pht->_tables.size();
        ++hashi;
        while (hashi < _pht->_tables.size()) {
            if (_pht->_tables[hashi]) {
                break;
            }
            ++hashi;
        }
        if (hashi == _pht->_tables.size()) {
            _node = nullptr;
        }
        else {
            _node = _pht->_tables[hashi];
        }
    }
    return *this;
}
```

**功能**：将迭代器移动到下一个节点。
**步骤**：
如果当前节点有下一个节点，则直接将迭代器指向该下一个节点。
如果当前节点没有下一个节点，则需要找到下一个非空的桶：
	计算当前节点所在的桶位置 hashi。
	从下一个桶开始查找，直到找到一个非空的桶。
	如果遍历完所有桶都没有找到非空桶，则将迭代器指向 nullptr。
	否则，将迭代器指向该非空桶的第一个节点。
返回迭代器自身的引用。

### 基于哈希表实现 unordered_set

`unordered_set`是一个无序的集合容器，它存储 唯一的键值。通过复用前面实现的哈希表，定义`unordered_set`类。

```cpp
namespace zdf {
template<class K, class Hash = HashFunc<K>>
class unordered_set {
    // 仿函数，用于从键值中提取键
    struct SetKeyOfT {
        const K& operator()(const K& key) {
            return key;
        }
    };
public:
    // 定义迭代器类型
    typedef typename HashTable<K, const K, SetKeyOfT, Hash>::Iterator iterator;
    // 定义常量迭代器类型
    typedef typename HashTable<K, const K, SetKeyOfT, Hash>::ConstIterator const_iterator;
 
    // 返回容器起始位置的迭代器
    iterator begin() {
        return _ht.Begin();
    }
 
    // 返回容器结束位置的迭代器
    iterator end() {
        return _ht.End();
    }
 
    // 返回常量容器起始位置的迭代器
    const_iterator begin() const {
        return _ht.Begin();
    }
 
    // 返回常量容器结束位置的迭代器
    const_iterator end() const {
        return _ht.End();
    }
 
    // 插入键值到unordered_set
    pair<iterator, bool> insert(const K& key) {
        return _ht.Insert(key);
    }
 
    // 在unordered_set中查找键值
    iterator Find(const K& key) {
        return _ht.Find(key);
    }
 
    // 从unordered_set中删除键值
    bool Erase(const K& key) {
        return _ht.Erase(key);
    }
private:
    // 使用哈希表存储数据
    HashTable<K, const K, SetKeyOfT, Hash> _ht;
};
}
```

### 基于哈希表实现 unordered_map

   `unordered_map`是一个无序的键值对容器，它允许通过键快速查找对应的值。同样复用哈希表来实现`unordered_map`类。
```cpp
namespace zdf {
template<class K, class V, class Hash = HashFunc<K>>
class unordered_map {
    // 仿函数，用于从键值对中提取键
    struct MapKeyOfT {
        const K& operator()(const pair<K, V>& kv) {
            return kv.first;
        }
    };
public:
    // 定义迭代器类型
    typedef typename HashTable<K, pair<const K, V>, MapKeyOfT, Hash>::Iterator iterator;
    // 定义常量迭代器类型
    typedef typename HashTable<K, pair<const K, V>, MapKeyOfT, Hash>::ConstIterator const_iterator;
 
    // 返回容器起始位置的迭代器
    iterator begin() {
        return _ht.Begin();
    }
 
    // 返回容器结束位置的迭代器
    iterator end() {
        return _ht.End();
    }
 
    // 返回常量容器起始位置的迭代器
    const_iterator begin() const {
        return _ht.Begin();
    }
 
    // 返回常量容器结束位置的迭代器
    const_iterator end() const {
        return _ht.End();
    }
 
    // 插入键值对到unordered_map
    pair<iterator, bool> insert(const pair<K, V>& kv) {
        return _ht.Insert(kv);
    }
 
    // 通过键访问对应的值，若键不存在则插入默认值
    V& operator[](const K& key) {
        pair<iterator, bool> ret = _ht.Insert(make_pair(key, V()));
        return ret.first->second;
    }
 
    // 在unordered_map中查找键值对
    iterator Find(const K& key) {
        return _ht.Find(key);
    }
 
    // 从unordered_map中删除键值对
    bool Erase(const K& key) {
        return _ht.Erase(key);
    }
private:
    // 使用哈希表存储数据
    HashTable<K, pair<const K, V>, MapKeyOfT, Hash> _ht;
};
}
```
##### **`operator[]` 函数**
```cpp
V& operator[](const K& key) {
    pair<iterator, bool> ret = _ht.Insert(make_pair(key, V()));
    return ret.first->second;
}
```
- **功能**：通过键访问对应的值，如果键不存在则插入一个默认值。
- **步骤**：
    1. 调用底层哈希表的 `Insert` 函数插入键值对（如果键不存在）。
    2. 返回插入或已存在的键值对的值的引用。


### 测试代码

为了验证自定义的`unordered_map`和`unordered_set`的功能正确性，编写测试函数。
```cpp
using namespace std;
// 测试unordered_set的功能
void test_set() {
    zdf::unordered_set<int> s;
    int a[] = { 4,2,6,1,3,5,15,7,16,14,3,3,15 };
    for (auto e : a) {
        s.insert(e);
    }
    for (auto e : s) {
        cout << e << " ";
    }
    cout << endl;
    zdf::unordered_set<int>::iterator it = s.begin();
    while (it != s.end()) {
        cout << *it << " ";
        ++it;
    }
    cout << endl;
}
 
// 测试unordered_map的功能
void test_map() {
    zdf::unordered_map<string, string> dict;
    dict.insert({ "sort", "排序" });
    dict.insert({ "Left", "左边" });
    dict.insert({ "right", "右边" });
    dict["left"] = "左边,剩余";
    dict["insert"] = "插入";
    dict["string"];
    zdf::unordered_map<string, string>::iterator it = dict.begin();
    while (it != dict.end()) {
        it->second += 'x';
        cout << it->first << ":" << it->second << endl;
        ++it;
    }
    cout << endl;
}
 
int main() {
    test_set();
    test_map();
    return 0;
}
```
通过上述代码，我们基于哈希表成功模拟实现了自定义的`unordered_map`和`unordered_set`，并对其功能进行了测试验证。这不仅有助于深入理解哈希表的工作原理以及 STL 容器的实现机制，也为在实际开发中根据特定需求进行容器的定制化提供了思路和参考。