#pragma once

#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <cstddef>

namespace bbn
{
	//哈希表结点
	template<class T>
	struct HashNode
	{
		T _data;
		HashNode<T>* _next;

		HashNode(const T& data)
			:_data(data)
			, _next(nullptr)
		{
		}
	};

	// 迭代器要在 HashTable 定义前使用它，先前置声明
	template<class K, class V, class KeyOfT, class Hash>
	class HashTable;

	//哈希表迭代器功能
	template<class K, class T, class Ptr, class Ref, class KeyOfT, class Hash>
	struct HTIterator
	{
		typedef HashNode<T> Node;
		typedef HTIterator<K, T, Ptr, Ref, KeyOfT, Hash> Self;

		Node* _node;
		const HashTable<K, T, KeyOfT, Hash>* _pht;

		HTIterator(Node* node, const HashTable<K, T, KeyOfT, Hash>* pht)
			:_node(node)
			, _pht(pht)
		{
		}

		Ref operator*()
		{
			return _node->_data;
		}

		Ptr operator->()
		{
			return &_node->_data;
		}

		bool operator!=(const Self& s)
		{
			return _node != s._node;
		}
		//前置递增操作符
		Self& operator++()
		{
			if (_node->_next)
			{
				_node = _node->_next;
			}
			else
			{
				KeyOfT kot;
				Hash hs;
				size_t hashi = hs(kot(_node->_data)) % _pht->_tables.size();
				++hashi;
				while (hashi < _pht->_tables.size())
				{
					if (_pht->_tables[hashi])
					{
						break;
					}
					++hashi;
				}
				if (hashi == _pht->_tables.size())
				{
					_node = nullptr;
				}
				else
				{
					_node = _pht->_tables[hashi];
				}
			}
			return *this;
		}
	};

	//哈希表的模拟实现
	template<class K, class V, class KeyOfT, class Hash>
	class HashTable
	{
		typedef HashNode<V> Node;

	public:
		typedef HTIterator<K, V, V*, V&, KeyOfT, Hash> Iterator;
		typedef HTIterator<K, V, const V*, const V&, KeyOfT, Hash> ConstIterator;



		HashTable()
		{
			_tables.resize(10);
		}

		// 当前实现拥有裸指针，禁止默认浅拷贝，避免 double free。诺有需要实现可以copy—swap方法实现
		HashTable(const HashTable&) = delete;
		HashTable& operator=(const HashTable&) = delete;

		~HashTable()
		{
			for (size_t i = 0; i < _tables.size(); i++)
			{
				Node* cur = _tables[i];
				while (cur)
				{
					Node* next = cur->_next;
					delete cur;
					cur = next;
				}
				_tables[i] = nullptr;
			}
		}

		Iterator Begin()
		{
			if (_n == 0)
			{
				return End();
			}
			for (size_t i = 0; i < _tables.size(); ++i)
			{
				Node* cur = _tables[i];
				if (cur)
				{
					return Iterator(cur, this);
				}
			}
			return End();
		}

		Iterator End()
		{
			return Iterator(nullptr, this);
		}

		ConstIterator Begin() const
		{
			if (_n == 0)
			{
				return End();
			}
			for (size_t i = 0; i < _tables.size(); ++i)
			{
				Node* cur = _tables[i];
				if (cur)
				{
					return ConstIterator(cur, this);
				}
			}
			return End();
		}

		ConstIterator End() const
		{
			return ConstIterator(nullptr, this);
		}

		std::pair<Iterator, bool> Insert(const V& data)
		{
			KeyOfT kot;
			Iterator it = Find(kot(data));

			if (it != End())
			{
				return std::make_pair(it, false);
			}
			Hash hs;

			if (_n == _tables.size())
			{
				std::vector<Node*> newtables(__stl_next_prime(_tables.size() + 1), nullptr);

				for (size_t i = 0; i < _tables.size(); i++)
				{
					Node* cur = _tables[i];
					while (cur)
					{
						Node* next = cur->_next;
						//计算在新表中的位置并插入
						size_t newHashi = hs(kot(cur->_data)) % newtables.size();
						cur->_next = newtables[newHashi];
						newtables[newHashi] = cur;
						cur = next;
					}
					_tables[i] = nullptr;
				}
				_tables.swap(newtables);
			}
			// 必须在扩容完成后，用新桶数计算下标
			size_t hashi = hs(kot(data)) % _tables.size();
			Node* newnode = new Node(data);
			newnode->_next = _tables[hashi];
			_tables[hashi] = newnode;
			++_n;
			return std::make_pair(Iterator(newnode, this), true);
		}

		Iterator Find(const K& key)
		{
			KeyOfT kot;
			Hash hs;
			size_t hashi = hs(key) % _tables.size();
			Node* cur = _tables[hashi];
			while (cur)
			{
				if (kot(cur->_data) == key)
				{
					return Iterator(cur, this);
				}
				cur = cur->_next;
			}
			return End();
		}

		bool Erase(const K& key)
		{
			KeyOfT kot;
			Hash hs;

			size_t hashi = hs(key) % _tables.size();
			Node* prev = nullptr;
			Node* cur = _tables[hashi];

			while (cur)
			{
				if (kot(cur->_data) == key)
				{
					if (prev == nullptr)
					{
						_tables[hashi] = cur->_next;
					}
					else
					{
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
		template<class, class, class, class, class, class>
		friend struct HTIterator;

		inline unsigned long __stl_next_prime(unsigned long n)
		{
			static const int __stl_num_primes = 28;
			static const unsigned long __stl_prime_list[__stl_num_primes] =
			{
				53, 97, 193, 389, 769,
				1543, 3079, 6151, 12289, 24593,
				49157, 98317, 196613, 393241, 786433,
				1572869, 3145739, 6291469, 12582917, 25165843,
				50331653, 100663319, 201326611, 402653189, 805306457,
				1610612741, 3221225473, 4294967291
			};
			const unsigned long* first = __stl_prime_list;
			const unsigned long* last = __stl_prime_list + __stl_num_primes;
			const unsigned long* pos = std::lower_bound(first, last, n);
			return pos == last ? *(last - 1) : *pos;
		}


		std::vector<Node*> _tables;
		size_t _n = 0;

	};

}
