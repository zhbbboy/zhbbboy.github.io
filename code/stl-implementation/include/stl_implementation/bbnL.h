#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <utility>

namespace bbn
{
	//模拟实现list当中的结点类
	template<class T>
	struct _list_node
	{
		_list_node(const T& val = T())
			:_val(val)
			, _next(nullptr)
			, _prev(nullptr)
		{
		}

		T _val;
		_list_node<T>* _next;
		_list_node<T>* _prev;
	};

	//模拟实现list迭代器
	template<class T, class Ref, class Ptr>
	struct _list_iterator
	{
		typedef _list_node<T> node;
		typedef _list_iterator<T, Ref, Ptr> self;

		_list_iterator(node* pnode)
			:_pnode(pnode)
		{
		}

		//各种运算符重载函数
		self operator++()//前置++
		{
			_pnode = _pnode->_next;
			return *this;
		}
		self operator--()//前置--
		{
			_pnode = _pnode->_prev;
			return *this;
		}
		self operator++(int)//后置++
		{
			self tmp(*this);
			_pnode = _pnode->_next;
			return tmp;
		}
		self operator--(int)//后置--
		{
			self tmp(*this);
			_pnode = _pnode->_prev;
			return tmp;
		}
		bool operator==(const self& s)const
		{
			return _pnode == s._pnode;
		}
		bool operator!=(const self& s)const
		{
			return _pnode != s._pnode;
		}
		Ref operator*()
		{
			return _pnode->_val;
		}
		Ptr operator->()
		{
			return &_pnode->_val;
		}

		Ref operator*() const
		{
			return _pnode->_val;
		}
		Ptr operator->() const
		{
			return &_pnode->_val;
		}

		node* _pnode;
	};

	//模拟实现list
	template<class T>
	class list
	{
	public:
		typedef _list_node<T> node;
		typedef _list_iterator<T, T&, T*> iterator;
		typedef _list_iterator<T, const T&, const T*>const_iterator;

		//默认成员函数
		list();
		list(const list<T>& lt);
		list<T>& operator=(list<T> lt);
		~list();

		//迭代器相关函数
		iterator begin();
		iterator end();
		const_iterator begin() const;
		const_iterator end() const;

		//访问容器相关函数
		T& front();
		T& back();
		const T& front() const;
		const T& back() const;

		//插入与删除函数
		void insert(iterator pos, const T& x);
		iterator erase(iterator pos);
		void push_back(const T& x);
		void pop_back();
		void push_front(const T& x);
		void pop_front();

		//其它函数
		size_t size() const;
		void resize(size_t n, const T& val = T());
		void clear();
		bool empty() const;
		void swap(list<T>& lt);

	private:
		node* _head;
	};

	template<class T>
	list<T>::list()
	{
		_head = new node;
		_head->_next = _head;
		_head->_prev = _head;
	}

	template<class T>
	list<T>::list(const list<T>& lt)
	{
		_head = new node;
		_head->_next = _head;
		_head->_prev = _head;

		for (const auto& e : lt)
		{
			push_back(e);
		}
	}

	template<class T>
	list<T>& list<T>::operator=(list<T> lt)
	{
		swap(lt);
		return *this;
	}

	template<class T>
	list<T>::~list()
	{
		clear();
		delete _head;
		_head = nullptr;
	}


	template<class T>
	typename list<T>::iterator list<T>::begin()
	{
		return iterator(_head->_next);
	}

	template<class T>
	typename list<T>::iterator list<T>::end()
	{
		return iterator(_head);
	}

	template<class T>
	typename list<T>::const_iterator list<T>::begin()const
	{
		return const_iterator(_head->_next);
	}

	template<class T>
	typename list<T>::const_iterator list<T>::end()const
	{
		return const_iterator(_head);
	}

	template<class T>
	T& list<T>::front()
	{
		return *begin();
	}

	template<class T>
	T& list<T>::back()
	{
		return *(--end());
	}

	template<class T>
	const T& list<T>::front()const
	{
		return *begin();
	}

	template<class T>
	const T& list<T>::back()const
	{
		return *(--end());
	}

	template<class T>
	void list<T>::insert(typename list<T>::iterator pos, const T& x)
	{
		assert(pos._pnode);

		node* cur = pos._pnode;
		node* prev = cur->_prev;
		node* newnode = new node(x);

		newnode->_next = cur;
		cur->_prev = newnode;
		newnode->_prev = prev;
		prev->_next = newnode;
	}

	template<class T>
	typename list<T>::iterator list<T>::erase(iterator pos)
	{
		assert(pos._pnode);
		assert(pos != end());

		node* cur = pos._pnode;
		node* prev = cur->_prev;
		node* next = cur->_next;

		delete cur;

		prev->_next = next;
		next->_prev = prev;
		return iterator(next);
	}

	template<class T>
	void list<T>::push_back(const T& x)
	{
		insert(end(), x);
	}

	template<class T>
	void list<T>::pop_back()
	{
		erase(--end());
	}

	template<class T>
	void list<T>::push_front(const T& x)
	{
		insert(begin(), x);
	}

	template<class T>
	void list<T>::pop_front()
	{
		erase(begin());
	}

	template<class T>
	size_t list<T>::size()const
	{
		size_t sz = 0;
		const_iterator it = begin();
		while (it != end())
		{
			sz++;
			it++;
		}
		return sz;
	}

	template<class T>
	void list<T>::resize(size_t n, const T& val)
	{
		iterator it = begin();
		size_t len = 0;
		while (len < n && it != end())
		{
			len++;
			it++;
		}

		if (len == n)
		{
			while (it != end())
			{
				it = erase(it);
			}
		}
		else
		{
			while (len < n)
			{
				push_back(val);
				len++;
			}
		}
	}

	template<class T>
	void list<T>::clear()
	{
		iterator it = begin();
		while (it != end())
		{
			it = erase(it);
		}
	}

	template<class T>
	bool list<T>::empty() const
	{
		return _head == _head->_next;
	}

	template<class T>
	void list<T>::swap(list<T>& lt)
	{
		std::swap(_head, lt._head);
	}
}
