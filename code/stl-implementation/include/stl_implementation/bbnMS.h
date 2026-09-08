#pragma once
//封装实现 set 和 map

#include "bbnRBTree.h"

namespace bbn
{
	template<class K>
	class set
	{
		struct SetKeyOfT
		{
			const K& operater()(const K& data)
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

		std::pair<iterator, bool> insert(const K& data)
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
			const K& operater()(const std::pair<K, V>& kv)
			{
				return kv.first;
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

		std::pair<iterator, bool> insert(const std::pair<K, V>& kv)
		{
			return _t.insert(kv);
		}

		V& operator[](const K& key)
		{
			std::pair<iterator, bool> cur = _t.insert({ key,V() });
			return cur.first->second;
		}

	private:
		RBTree<K, std::pair<const K, V>, MapKeyOfT> _t;
	};

}
