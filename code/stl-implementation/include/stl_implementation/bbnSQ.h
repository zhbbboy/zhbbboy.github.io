#pragma once

#include<deque>

namespace bbn
{
	template<class T, class Container = std::deque<T>>
	class stack
	{
	public:
		void push(const T& x)
		{
			_con.push_back(x);
		}

		void pop()
		{
			_con.pop_back();
		}

		T& top()
		{
			return _con.back();
		}

		const T& top()const
		{
			return _con.back();
		}

		size_t size()const
		{
			return _con.size();
		}

		bool empty() const
		{
			return _con.empty();
		}

		void swap(stack<T, Container>& st)
		{
			_con.swap(st._con);
		}

	private:
		Container _con;

	};

	template<class T, class Container = std::deque<T>>
	class queue
	{
	public:
		//队尾入队
		void push(const T& x)
		{
			_con.push_back(x);
		}

		//队头出队
		void pop()
		{
			_con.pop_front();
		}

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
