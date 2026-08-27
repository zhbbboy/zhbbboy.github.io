#pragma once

#include <iostream>
#include <queue>
#include <utility>

namespace bbn
{
	template<class T, class Container = std::vector<T>, class Compare = std::less<T>>
	class priority_queue
	{
	public:
		priority_queue()
			:_con()
			, _comp()
		{
		}

		template<class InputIterator>
		priority_queue(InputIterator first, InputIterator last)
			: _con(first, last)
			, _comp()
		{
			if (_con.size() > 1)
			{
				for (int i = static_cast<int>(_con.size() - 2) / 2; i >= 0; i--)
				{
					AdjustDown(i);
				}
			}
		}

		void push(const T& x)
		{
			_con.push_back(x);
			AdjustUp(_con.size() - 1);
		}

		void pop()
		{
			if (!empty())
			{
				std::swap(_con[0], _con[_con.size() - 1]);
				_con.pop_back();
				AdjustDown(0, _con.size());
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
		Compare _comp;

		//向下调整
		void AdjustDown(int parent, int size)
		{
			// 先假设左孩子优先级更高
			int child = 2 * parent + 1;
			while (child < size)
			{
				// 如果右孩子存在，而且右孩子优先级更高
				if (child + 1 < size && _comp(_con[child], _con[child + 1]))
				{
					child++;
				}
				// 如果孩子优先级比父结点高，就交换
				if (_comp(_con[parent], _con[child]))
				{
					std::swap(_con[child], _con[parent]);
				}
				parent = child;
				child = 2 * parent + 1;
			}
		}

		//向上调整
		void AdjustUp(int child)
		{
			int parent = (child - 1) / 2;
			while (child > 0)
			{
				// 默认 less：
				// parent < child，说明 child 优先级更高
				if (_comp(_con[parent], _con[child]))
				{
					std::swap(_con[child], _con[parent]);
				}
				child = parent;
				parent = (child - 1) / 2;
			}
		}
	};

}
