#pragma once

#include "bbnHT.h"

namespace bbn
{
	template<class K>
	struct HashFunc
	{
		size_t operator()(const K& key) const
		{
			return key;
		}
	};

	template<class K, class Hash = HashFunc<K>>
	class unordered_set
	{
		struct SetKeyOfT
		{
			const K& operator()(const K& key)
			{
				return key;
			}
		};

	public:
		// 定义迭代器类型
		typedef typename HashTable<K, const K, SetKeyOfT, Hash>::Iterator iterator;
		// 定义常量迭代器类型
		typedef typename HashTable<K, const K, SetKeyOfT, Hash>::ConstIterator const_iterator;

		iterator begin()
		{
			return _ht.Begin();
		}

		iterator end()
		{
			return _ht.End();
		}

		const_iterator begin() const
		{
			return _ht.Begin();
		}

		const_iterator end() const
		{
			return _ht.End();
		}

		std::pair<iterator, bool> insert(const K& key)
		{
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
		//使用哈希表存储数据
		HashTable<K, const K, SetKeyOfT, Hash> _ht;
	};


	template<class K, class V, class Hash = HashFunc<K>>
	class unordered_map {
		// 仿函数，用于从键值对中提取键
		struct MapKeyOfT {
			const K& operator()(const std::pair<const K, V>& kv) {
				return kv.first;
			}
		};
	public:
		// 定义迭代器类型
		typedef typename HashTable<K, std::pair<const K, V>, MapKeyOfT, Hash>::Iterator iterator;
		// 定义常量迭代器类型
		typedef typename HashTable<K, std::pair<const K, V>, MapKeyOfT, Hash>::ConstIterator const_iterator;

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
		std::pair<iterator, bool> insert(const std::pair<K, V>& kv) {
			return _ht.Insert(kv);
		}

		// 通过键访问对应的值，若键不存在则插入默认值
		V& operator[](const K& key) {
			std::pair<iterator, bool> ret = _ht.Insert(std::make_pair(key, V()));
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
		HashTable<K, std::pair<const K, V>, MapKeyOfT, Hash> _ht;
	};
}
