#pragma once

#include <algorithm>
#include <cassert>
#include <cctype>
#include <iostream>
#include <utility>

namespace bbnV
{

	template<class T>
	class vector
	{
	public:
		typedef T* iterator;
		typedef const T* const_iterator;

		//默认成员函数
		vector();
		vector(size_t n, const T& val);
		vector(int n, const T& val);
		vector(long n, const T& val);
		template<class InputIterator>
		vector(InputIterator first, InputIterator last);
		vector(const vector<T>& v);
		vector<T>& operator=(vector<T> v);
		~vector();

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
		iterator _start;//指向容器的头
		iterator _finish;//指向有效数据的尾，size
		iterator _endofstorage;//指向容器的尾，capacity
	};

	template<class T>
	vector<T>::vector()
		:_start(nullptr)
		, _finish(nullptr)
		, _endofstorage(nullptr)
	{
	}

	template<class T>
	vector<T>::vector(size_t n, const T& val)
		:_start(nullptr)
		, _finish(nullptr)
		, _endofstorage(nullptr)
	{
		reserve(n);
		for (size_t i = 0; i < n; i++)
		{
			push_back(val);
		}
	}

	template<class T>
	vector<T>::vector(int n, const T& val)
		:_start(nullptr)
		, _finish(nullptr)
		, _endofstorage(nullptr)
	{
		reserve(n);
		for (size_t i = 0; i < n; i++)
		{
			push_back(val);
		}
	}

	template<class T>
	vector<T>::vector(long n, const T& val)
		:_start(nullptr)
		, _finish(nullptr)
		, _endofstorage(nullptr)
	{
		reserve(n);
		for (size_t i = 0; i < n; i++)
		{
			push_back(val);
		}
	}

	template<class T>
	template<class InputIterator>
	vector<T>::vector(InputIterator first, InputIterator last)
		:_start(nullptr)
		, _finish(nullptr)
		, _endofstorage(nullptr)
	{
		while (first != last)
		{
			push_back(*first);
			first++;
		}
	}

	template<class T>
	vector<T>::vector(const vector<T>& v)
		:_start(nullptr)
		, _finish(nullptr)
		, _endofstorage(nullptr)
	{
		reserve(v.capacity());
		for (auto e : v)
		{
			push_back(e);
		}
	}

	template<class T>
	vector<T>& vector<T>::operator=(vector<T> v)
	{
		swap(v);
		return *this;
	}

	template<class T>
	vector<T>::~vector()
	{
		if (_start)
		{
			delete[] _start;
			_start = nullptr;
			_finish = nullptr;
			_endofstorage = nullptr;
		}
	}

	template<class T>
	typename vector<T>::iterator vector<T>::begin()
	{
		return _start;
	}

	template<class T>
	typename vector<T>::iterator vector<T>::end()
	{
		return _finish;
	}

	template<class T>
	typename vector<T>::const_iterator vector<T>::begin()const
	{
		return _start;
	}

	template<class T>
	typename vector<T>::const_iterator vector<T>::end()const
	{
		return _finish;
	}

	template<class T>
	size_t vector<T>::size()const
	{
		return _finish - _start;
	}

	template<class T>
	size_t vector<T>::capacity()const
	{
		return _endofstorage - _start;
	}

	template<class T>
	void vector<T>::reserve(size_t n)
	{
		if (n > capacity())
		{
			size_t sz = size();
			T* tmp = new T[n];
			if (_start)
			{
				for (size_t i = 0; i < sz; i++)
				{
					tmp[i] = _start[i];
				}
				delete[] _start;
			}
			_start = tmp;
			_finish = _start + sz;
			_endofstorage = _start + n;
		}
	}

	template<class T>
	void vector<T>::resize(size_t n, const T& val)
	{
		if (n < size())
		{
			_finish = _start + n;
		}
		else
		{
			if (n > capacity())
			{
				reserve(n);
			}
			while (_finish < _start + n)
			{
				*_finish = val;
				_finish++;
			}
		}
	}

	template<class T>
	bool vector<T>::empty() const
	{
		return _start == _finish;
	}

	template<class T>
	void vector<T>::push_back(const T& x)
	{
		if (_finish == _endofstorage)
		{
			size_t newcapacity = capacity() == 0 ? 4 : 2 * capacity();
			reserve(newcapacity);
		}
		*_finish = x;
		_finish++;
	}

	template<class T>
	void vector<T>::pop_back()
	{
		assert(!empty());
		_finish--;
	}

	template<class T>
	void vector<T>::insert(typename vector<T>::iterator pos, const T& x)
	{
		if (_finish == _endofstorage)
		{
			size_t len = pos - _start;
			size_t newcapacity = capacity() == 0 ? 4 : 2 * capacity();
			reserve(newcapacity);
			pos = _start + len;
		}

		iterator end = _finish;
		while (end >= pos + 1)
		{
			*end = *(end - 1);
			end--;
		}
		*pos = x;
		_finish++;
	}
	template<class T>
	typename vector<T>::iterator vector<T>::erase(typename vector<T>::iterator pos)
	{
		assert(!empty());
		iterator it = pos + 1;
		while (it != _finish)
		{
			*(it - 1) = *it;
			it++;
		}
		_finish--;
		return pos;
	}

	template<class T>
	void vector<T>::swap(vector<T>& v)
	{
		std::swap(_start, v._start);
		std::swap(_finish, v._finish);
		std::swap(_endofstorage, v._endofstorage);
	}

	template<class T>
	T& vector<T>::operator[](size_t i)
	{
		assert(i < size());
		return _start[i];
	}

	template<class T>
	const T& vector<T>::operator[](size_t i)const
	{
		assert(i < size());
		return _start[i];
	}

}

