---
title: map和set
date: 2026-09-06
tags: []
draft: false
---

## 前言

由于 set 是 key 搜索场景的结构， map 是 key/value 搜索场景的结构。如果要支持 set 和 map 就需要实现两个红黑树。因此，可以对红黑树进行改动，从而能够同时支持 set 和 map 。

## 对红黑树的结点进行改造
在 set 中，insert 的参数为 key , 而在 map 中， insert 的参数为 pair 类型。

因此，将红黑树的结点改造为一个类型，可以同时传 key 和 pair<K, V> 类型的值。
```cpp
enum Color
{
	RED,
	BLACK
};
 
// 接收 set 或 map 类型的数据
template<class T>
struct RBTreeNode
{
	RBTreeNode(const T& data)
		:_data(data)
		, _left(nullptr)
		, _right(nullptr)
		, _parent(nullptr)
	{}
 
	T _data;
 
	RBTreeNode<T>* _left;
	RBTreeNode<T>* _right;
	RBTreeNode<T>* _parent;
 
	Color _color;
};
```

## insert 参数的改动

将结点改造为 T 类型之后，insert 的参数在使用过程中就会有问题，其不知道传入的参数为 set 还是 map 类型的数据，因此我们可以通过实现一个仿函数来控制数据类型。
由于要将红黑树封装从而实现 set 和 map ，因此数据为 set 还是 map 类型，set 和 map 一定知道，所以仿函数要在 set 和 map 中实现，然后传给红黑树。所以红黑树还要增加一个模板参数用于控制数据类型。

先来看仿函数的实现：
```cpp
struct SetKeyOfT
{
	const K& operator()(const K& data)
	{
		return data;
	}
};
```

```cpp
struct MapKeyOfT
{
	const K& operator()(const pair<K, V>& kv)
	{
		return kv.first;
	}
};
```

再对红黑树进行改造：
```cpp
template<class K, class T, class KeyOfT>
class RBTree
{
	typedef RBTreeNode<T> Node;
public:
	bool insert(const T& data)
	{
		if (_root == nullptr)
		{
			_root = new Node(data);
			_root->_color = BLACK;
 
			return true;
		}
 
		Node* parent = _root;
		Node* cur = _root;
 
		KeyOfT kot;
 
		while (cur)
		{
			if (kot(cur->_data)  > kot(data))
			{
				parent = cur;
				cur = cur->_left;
			}
			else if (kot(cur->_data) < kot(data))
			{
				parent = cur;
				cur = cur->_right;
			}
			else
			{
				return false;
			}
		}
 
		cur = new Node(data);
		cur->_color = RED;
 
		Node* newnode = cur;
 
		if (kot(parent->_data) < kot(data))
		{
			parent->_right = cur;
		}
		else
		{
			parent->_left = cur;
		}
 
		cur->_parent = parent;
 
		// 对红黑树进行调整
		// 如果父节点为红色，则需要进行调整
		while (parent && parent->_color == RED)
		{
			Node* grandfather = parent->_parent;
 
			if (grandfather->_left == parent)
			{
				Node* uncle = grandfather->_right;
 
				if (uncle && uncle->_color == RED)
				{
					parent->_color = uncle->_color = BLACK;
					grandfather->_color = RED;
 
					// 继续向上调整
					cur = grandfather;
					parent = cur->_parent;
				}
				// uncle不存在或存在且为黑
				else 
				{
					if (parent->_left == cur)
					{
						// 右单旋+变色
						RotateR(grandfather);
 
						parent->_color = BLACK;
						grandfather->_color = RED;
					}
					else
					{
						// 左右双旋+变色
						RotateL(parent);
						RotateR(grandfather);
 
						cur->_color = BLACK;
						grandfather->_color = RED;
					}
 
					break;
				}
			}
			else // grandfather->_right == parent
			{
				Node* uncle = grandfather->_left;
 
				if (uncle && uncle->_color == RED)
				{
					parent->_color = uncle->_color = BLACK;
					grandfather->_color = RED;
 
					// 继续向上调整
					cur = grandfather;
					parent = cur->_parent;
				}
				else // uncle不存在或存在且为黑
				{
					if (parent->_right == cur)
					{
						// 左单旋+变色
						RotateL(grandfather);
 
						parent->_color = BLACK;
						grandfather->_color = RED;
					}
					else
					{
						// 右左双旋+变色
						RotateR(parent);
						RotateL(grandfather);
 
						cur->_color = BLACK;
						grandfather->_color = RED;
					}
 
					break;
				}
			}
		}
 
		_root->_color = BLACK;
 
		return true;
	}
 
	// 右单旋
	void RotateR(Node* parent)
	{
		Node* subL = parent->_left;
		Node* subLR = subL->_right;
		
		// 调整结点的链接关系
		parent->_left = subLR;
		
		// 要考虑subLR为nullptr的情况
		if (subLR)
		{
			subLR->_parent = parent;
		}
		
		Node* pparent = parent->_parent;
		
		parent->_parent = subL;
		subL->_right = parent;
		
		// 考虑是否为根节点的情况
		if (parent == _root)
		{
			_root = subL;
			subL->_parent = nullptr;
		}
		else
		{
			if (pparent->_left == parent)
			{
				pparent->_left = subL;
			}
			else
			{
				pparent->_right = subL;
			}
		
			subL->_parent = pparent;
		}
	}
 
	// 左单旋
	void RotateL(Node* parent)
	{
 
		Node* subR = parent->_right;
		Node* subRL = subR->_left;
	
		parent->_right = subRL;
		if (subRL)
		{
			subRL->_parent = parent;
		}
	
		Node* pparent = parent->_parent;
	
		parent->_parent = subR;
		subR->_left = parent;
	
		if (parent == _root)
		{
			_root = subR;
			subR->_parent = nullptr;
		}
		else
		{
			if (pparent->_left == parent)
			{
				pparent->_left = subR;
			}
			else
			{
				pparent->_right = subR;
			}
	
			subR->_parent = pparent;
		}
	}
 
private:
	Node* _root = nullptr;
};
```

## 红黑树迭代器的实现

实现 set 和 map 必不可少的要对红黑树加迭代器。其中最重要的就是 ++ 、-- 的实现。

首先定义出迭代器的结构，以及 operator* 、operator-> 、operator== 等函数。
```cpp
// Ref -> T& 、 Ptr -> T* 
template<class T, class Ref, class Ptr>
struct RBTreeIterator
{
	typedef RBTreeNode<T> Node;
	typedef RBTreeIterator<T, Ref, Ptr> Self;
 
	RBTreeIterator(Node* node)
		:_node(node)
	{}
 
	Self operator++();
 
	Self operator--();
 
	Ref operator*()
	{
		return _node->_data;
	}
 
	Ptr operator->()
	{
		return &_node->_data;
	}
 
	bool operator==(const Self& s) const
	{
		return _node == s._node;
	}
 
	bool operator!=(const Self& s) const
	{
		return _node != s._node;
	}
 
	Node* _node;
};
```

接着来看 operator++ ( ) 的实现：

此时传入的参数为当前结点(cur)，要按照中序遍历的寻找该节点的下一个结点(next)，所以如果 cur 的右子树不为空，就要找到 cur 右子树的最左结点，该节点即为 next 的位置；如果 cur 右子树为空，则当前结点及其子树已经访问完毕，需要向上寻找其父亲节点(parent)，找到 cur 为 parent 左子树的结点，如果找到，则 next 为 parent ，如果没有找到，则会向上回溯至根，则 parent 为空，说明整棵树已被访问完，走到了空。
```cpp
Self operator++()
{
	// 找结点右子树的最左结点
	if (_node->_right)
	{
		Node* min = _node->_right;
 
		while (min->_left)
		{
			min = min->_left;
		}
 
		_node = min;
	}
	else
	{
		Node* cur = _node;
		Node* parent = cur->_parent;
 
		while (parent && cur == parent->_right)
		{
			cur = parent;
			parent = cur->_parent;
		}
		_node = parent;
	}
 
	return *this;
}
```

最后看 operator-- ( ) 的实现：

-- 和 ++ 的逻辑正好是相反的，传入的参数为当前结点(cur)，要按照中序遍历的寻找该节点的上一个结点(prev)，所以如果 cur 的左子树不为空，就要找到 cur 左子树的最右结点，该节点即为 prev 的位置；如果 cur 左子树为空，则当前结点及其子树已经访问完毕，需要向上寻找其父亲节点(parent)，找到 cur 为 parent 右子树的结点，如果找到，则 next 为 parent ，如果没有找到，则会向上回溯至根，则 parent 为空，说明整棵树已被访问完，走到了空。

```cpp
Self operator--()
{
	if (_node->_left)
	{
		Node* min = _node->_left;
 
		while (min->_right)
		{
			min = min->_right;
		}
 
		_node = min;
	}
	else
	{
		Node* cur = _node;
		Node* parent = cur->_parent;
 
		while (parent && cur == parent->_left)
		{
			cur = parent;
			parent = cur->_parent;
		}
		_node = parent;
	}
 
	return *this;
}
```
但 -- 操作还要考虑当前结点为 nullptr 的情况，此时 -- 之后要访问的是该树的最大节点。要找到该节点，就需要借助根节点或对整棵树设置一个头节点连接第一个和最后一个结点 ( 则 end() 位置的迭代器就为头节点 ) 。这里我借助根节点来实现：
```cpp
Self operator--()
{
	if (_node == nullptr)
	{
		Node* cur = _root;
 
		while (cur && cur->_right)
		{
			cur = cur->_right;
		}
 
		_node = cur;
	}
	else if (_node->_left)
	{
		Node* min = _node->_left;
 
		while (min->_right)
		{
			min = min->_right;
		}
 
		_node = min;
	}
	else
	{
		Node* cur = _node;
		Node* parent = cur->_parent;
 
		while (parent && cur == parent->_left)
		{
			cur = parent;
			parent = cur->_parent;
		}
		_node = parent;
	}
 
	return *this;
}
```
## 对红黑树加迭代器

由于红黑树的迭代器要在 set 和 map 中使用，所以把其定义为公有。同时改变 insert 的返回类型为 pair<iterator, bool> 。
```cpp
template<class K, class T, class KeyOfT>
class RBTree
{
	typedef RBTreeNode<T> Node;
public:
	typedef RBTreeIterator<T, T&, T*> Iterator;
	typedef RBTreeIterator<T, const T&, const T*> ConstIterator;
 
	Iterator Begin()
	{
		Node* cur = _root;
		while (cur && cur->_left)
		{
			cur = cur->_left;
		}
 
		return Iterator(cur, _root);
	}
 
	Iterator End()
	{
		return Iterator(nullptr, _root);
	}
 
	ConstIterator Begin() const
	{
		Node* cur = _root;
		while (cur && cur->_left)
		{
			cur = cur->_left;
		}
 
		return ConstIterator(cur, _root);
	}
 
	ConstIterator End() const
	{
		return ConstIterator(nullptr, _root);
	}
 
	pair<Iterator, bool> insert(const T& data)
	{
		if (_root == nullptr)
		{
			_root = new Node(data);
			_root->_color = BLACK;
 
			return { Iterator(_root, _root), true };
		}
 
		Node* parent = _root;
		Node* cur = _root;
 
		KeyOfT kot;
 
		while (cur)
		{
			if (kot(cur->_data)  > kot(data))
			{
				parent = cur;
				cur = cur->_left;
			}
			else if (kot(cur->_data) < kot(data))
			{
				parent = cur;
				cur = cur->_right;
			}
			else
			{
				return { Iterator(cur, _root), false };
			}
		}
 
		cur = new Node(data);
		cur->_color = RED;
 
		Node* newnode = cur;
 
		if (kot(parent->_data) < kot(data))
		{
			parent->_right = cur;
		}
		else
		{
			parent->_left = cur;
		}
 
		cur->_parent = parent;
 
		// 对红黑树进行调整
		// 如果父节点为红色，则需要进行调整
		while (parent && parent->_color == RED)
		{
			Node* grandfather = parent->_parent;
 
			if (grandfather->_left == parent)
			{
				Node* uncle = grandfather->_right;
 
				if (uncle && uncle->_color == RED)
				{
					parent->_color = uncle->_color = BLACK;
					grandfather->_color = RED;
 
					// 继续向上调整
					cur = grandfather;
					parent = cur->_parent;
				}
				// uncle不存在或存在且为黑
				else 
				{
					if (parent->_left == cur)
					{
						// 右单旋+变色
						RotateR(grandfather);
 
						parent->_color = BLACK;
						grandfather->_color = RED;
					}
					else
					{
						// 左右双旋+变色
						RotateL(parent);
						RotateR(grandfather);
 
						cur->_color = BLACK;
						grandfather->_color = RED;
					}
 
					break;
				}
			}
			else // grandfather->_right == parent
			{
				Node* uncle = grandfather->_left;
 
				if (uncle && uncle->_color == RED)
				{
					parent->_color = uncle->_color = BLACK;
					grandfather->_color = RED;
 
					// 继续向上调整
					cur = grandfather;
					parent = cur->_parent;
				}
				else // uncle不存在或存在且为黑
				{
					if (parent->_right == cur)
					{
						// 左单旋+变色
						RotateL(grandfather);
 
						parent->_color = BLACK;
						grandfather->_color = RED;
					}
					else
					{
						// 右左双旋+变色
						RotateR(parent);
						RotateL(grandfather);
 
						cur->_color = BLACK;
						grandfather->_color = RED;
					}
 
					break;
				}
			}
		}
 
		_root->_color = BLACK;
 
		return { Iterator(newnode, _root), true };
	}
 
	// 右单旋
	void RotateR(Node* parent)
	{
		Node* subL = parent->_left;
		Node* subLR = subL->_right;
		
		// 调整结点的链接关系
		parent->_left = subLR;
		
		// 要考虑subLR为nullptr的情况
		if (subLR)
		{
			subLR->_parent = parent;
		}
		
		Node* pparent = parent->_parent;
		
		parent->_parent = subL;
		subL->_right = parent;
		
		// 考虑是否为根节点的情况
		if (parent == _root)
		{
			_root = subL;
			subL->_parent = nullptr;
		}
		else
		{
			if (pparent->_left == parent)
			{
				pparent->_left = subL;
			}
			else
			{
				pparent->_right = subL;
			}
		
			subL->_parent = pparent;
		}
	}
 
	// 左单旋
	void RotateL(Node* parent)
	{
 
		Node* subR = parent->_right;
		Node* subRL = subR->_left;
	
		parent->_right = subRL;
		if (subRL)
		{
			subRL->_parent = parent;
		}
	
		Node* pparent = parent->_parent;
	
		parent->_parent = subR;
		subR->_left = parent;
	
		if (parent == _root)
		{
			_root = subR;
			subR->_parent = nullptr;
		}
		else
		{
			if (pparent->_left == parent)
			{
				pparent->_left = subR;
			}
			else
			{
				pparent->_right = subR;
			}
	
			subR->_parent = pparent;
		}
	}
 
private:
	Node* _root = nullptr;
};
```
## 封装实现 set 和 map

set 和 map 还有一个特点，就是无论是普通迭代器还是 const 迭代器，其 key 值不能被修改。所以要将其定义为 const 类型。
```cpp
namespace Friend
{
	template<class K>
	class set
	{
		struct SetKeyOfT
		{
			const K& operator()(const K& data)
			{
				return data;
			}
		};
 
	public:
		typedef typename RBTree<K, const K, SetKeyOfT>::Iterator iterator;
		typedef typename RBTree<K, const K, SetKeyOfT>::ConstIterator const_iterator;
 
		iterator begin()
		{
			return _t.Begin();
		}
 
		iterator end()
		{
			return _t.End();
		}
 
		const_iterator begin() const
		{
			return _t.Begin();
		}
 
		const_iterator end() const
		{
			return _t.End();
		}
 
		pair<iterator, bool> insert(const K& data)
		{
			return _t.insert(data);
		}
 
	private:
		RBTree<K, const K, SetKeyOfT> _t;
	};
 
 
	template<class K, class V>
	class map
	{
		struct MapKeyOfT
		{
			const K& operator()(const pair<K, V>& kv)
			{
				return kv.first;
			}
		};
 
	public:
		typedef typename RBTree<K, pair<const K, V>, MapKeyOfT>::Iterator iterator;
		typedef typename RBTree<K, pair<const K, V>, MapKeyOfT>::ConstIterator const_iterator;
 
		iterator begin()
		{
			return _t.Begin();
		}
 
		iterator end()
		{
			return _t.End();
		}
 
		const_iterator begin() const
		{
			return _t.Begin();
		}
 
		const_iterator end() const
		{
			return _t.End();
		}
 
		pair<iterator, bool> insert(const pair<K, V>& kv)
		{
			return _t.insert(kv);
		}
 
		V& operator[](const K& key)
		{
			pair<iterator, bool> cur = _t.insert({ key, V() });
			return cur.first->second;
		}
 
	private:
		RBTree<K, pair<const K, V>, MapKeyOfT> _t;
	};
}
```