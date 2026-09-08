#pragma once
#include <iostream>
#include <utility>


namespace bbn
{
	template <class K, class V>
	struct BSTreeNode
	{
		typedef BSTreeNode<K, V> Node;

		Node* _left;
		Node* _right;
		K _key;
		V _value;

		BSTreeNode(const K& key,const V& value)
			: _left(nullptr)
			, _right(nullptr)
			, _key(key)
			, _value(value)
		{}
	};
	

	// =====================
	// 二叉搜索树类用map的KV模型
	// =====================
	template<class K,class V>
	class BSTreeKV
	{
		typedef BSTreeNode<K, V> Node;

	public:
		// 构造函数
		BSTreeKV()
			: _root(nullptr)
		{ }

		// 向二叉搜索树中插入一个键值对 (key, value)
		// 返回值：如果插入成功返回 true，如果插入失败(已存在相同key)返回 false
		bool Insert(const K& key, const V& value)
		{
			if (_root == nullptr)
			{
				_root = new Node(key, value);
				return true;
			}

			Node* parent = nullptr;
			Node* cur = _root;
			while (cur)
			{
				if (cur->_key < key)
				{
					parent = cur;
					cur = cur->_right;
				}
				else if (cur->_key > key)
				{
					parent = cur;
					cur = cur->_left;
				}
				else
				{
					return false;
				}
			}

			cur = new Node(key, value);
			if (parent->_key < key)
			{
				parent->_right = cur;
			}
			else
			{
				parent->_left = cur;
			}

			return true;
		}

		// 查找给定 key 对应的节点，返回指向该节点的指针，如果不存在则返回 nullptr
		Node* Find(const K& key)
		{
			Node* cur = _root;
			while (cur)
			{
				if (cur->_key < key)
				{
					cur = cur->_right;
				}
				else if (cur->_key > key)
				{
					cur = cur->_left;
				}
				else
				{
					return cur;
				}
			}
			return nullptr;
		}

		// 从二叉搜索树中删除 key 对应的节点，成功返回 true，失败返回 false
		bool Erase(const K& key)
		{
			Node* parent = nullptr;
			Node* cur = _root;

			// 1. 先找到要删除的节点 cur 及其父节点 parent
			while (cur)
			{
				if (cur->_key < key)
				{
					parent = cur;
					cur = cur->_right;
				}
				else if (cur->_key > key)
				{
					parent = cur;
					cur = cur->_left;
				}
				else
				{
					// 找到了要删除的节点
					// 2. 根据 cur 是否存在子树分情况处理
					//    a) cur 没有左子树
					if (cur->_left == nullptr)
					{
						if (cur == _root)
						{
							_root = cur->_right;
						}
						else
						{
							// 如果 cur 不是根节点，则根据 parent 判断
							// cur 是 parent 的左子树还是右子树
							if (cur == parent->_right)
							{
								parent->_right = cur->_right;
							}
							else
							{
								parent->_left = cur->_right;
							}
						}
						delete cur;
						return true;
					}
					//    b) cur 没有右子树
					else if (cur->_right == nullptr)
					{
						if (cur == _root)
						{
							_root = cur->_left;
						}
						else
						{
							if (cur == parent->_right)
							{
								parent->_right = cur->_left;
							}
							else
							{
								parent->_left = cur->_left;
							}
						}
						delete cur;
						return true;
					}
					//    c) cur 既有左子树又有右子树
					else
					{
						// 使用替换法：找到 cur 的右子树中的最小值节点替换掉 cur
						Node* rightMinParent = cur;
						Node* rightMin = cur->_right;
						while (rightMin->_left)
						{
							rightMinParent = rightMin;
							rightMin = rightMin->_left;
						}

						cur->_key = rightMin->_key;
						cur->_value = rightMin->_value;

						if (rightMin == rightMinParent->_left)
						{
							rightMinParent->_left = rightMin->_right;
						}
						else
						{
							rightMinParent->_right = rightMin->_right;
						}
						delete rightMin;
						return true;
					}
				}
			}
			return false;
		}

		void InOrder()
		{
			_InOrder(_root);
			std::cout << std::endl;
		}



	private:
		// 辅助函数：中序遍历
		void _InOrder(Node* root)
		{
			if (root == nullptr)
			{
				return;
			}
			_InOrder(root->_left);
			std::cout << root->_key << " ";
			_InOrder(root->_right);
				
		}

		Node* _root;
	};


	template <class K>
	//树节点的结构体
	struct  BSTreeNodeK
	{
		typedef BSTreeNodeK<K> Node;//千万别少了这句！！！！！！

		Node* _left;
		Node* _right;
		K _key;
		BSTreeNodeK(const K& key)//这里是K是大写，一定要注意，改了好多
			:_left(nullptr)
			, _right(nullptr)
			, _key(key)
		{
		}


	};

	// =====================
	// 二叉搜索树类用set的K模型
	// =====================
	template<class K>
	class BSTreeK
	{
		typedef BSTreeNodeK<K> Node;

	public:
		//默认构造
		BSTreeK() = default;

		//拷贝构造
		BSTreeK(const BSTree<K>& t)
		{
			_root = Copy(t._root);
		}

		//赋值重载
		//完整的函数名每次忘记怎么写
		//BSTree<K>& ：返回值类型是对当前对象的引用，以便支持链式赋值（a = b = c; ）。
		//operator=：定义赋值运算符。
		//const BSTree<K>& t：参数类型是一个const引用，表示要赋值的对象不应被修改。


		//使用 swap 交换资源
		BSTreeK<K>& operator=(const BSTree<K>& t)
		{
			std::swap(_root, t._root);
			return *this;
		}

		//析构
		~BSTreeK()
		{
			Destory(_root);
		}



	private:
		//下面这部分隐藏，对外只提供InOrder().用户只用关注接口不必关心细节。
		void _InOrder(Node* root)
		{
			if (root == nullptr)
			{
				return ;
			}

			_InOrder(root->_left);
			std::cout << root->_key << " ";
			_InOrder(root->_right);
		}

		Node* Copy(Node* root)
		{
			if (root == nullptr)
			{
				return nullptr;
			}
			Node* newRoot = new Node(root->_key);

			newRoot->_left = Copy(root->_left);
			newRoot->_right = Copy(root->_right);
			return newRoot;
		}

		//赋值运算重载的第二种写法要用到，析构函数也要用到
		void Destory(Node* root)
		{
			if (root == nullptr)
			{
				return;
			}
			Destory(root->_left);
			Destory(root->_right);
			delete root;
		}

		//这些函数的实现通常会更简洁，因为递归结构自然而然地处理了树的遍历：

		//	bool _FindR(Node* root, const K& key)
		//	如果当前节点为空，则返回 false。
		//	根据键值与当前节点的比较，决定向左或向右子树递归查找。
		bool _FindR(Node* root, const K& key)
		{
			if (root == nullptr)
			{
				return false;
			}
			if (root->_key < key)
			{
				return _FindR(root->_right, key);
			}
			else if (root->_key > key)
			{
				return _FindR(root->_left, key);
			}
			else
			{
				return true;
			}
		}

		//	bool _InsertR(Node*& root, const K& key)
		//	处理插入操作的递归逻辑。
		//	当找到合适的插入位置（当前节点为空）时，创建新节点。
		bool _InsertR(Node*& root, const K& key)
		{
			if (root == nullptr)
			{
				root = new Node(key);
				return true;
			}
			if (root->_key < key)
			{
				return _InsertR(root->_right, key);
			}
			else if(root->_key > key)
			{
				return _InsertR(root->_left, key);
			}
			else
			{
				//遇到值相等的，  重复了 无法插入
				return false;
			}
		}

		//	bool _EraseR(Node*& root, const K& key)
		//	处理删除操作的递归逻辑。
		//	逻辑与非递归的 _Erase 类似，但通过递归方式实现。
		bool _EraseR(Node*& root, const K& key)
		{
			if (root == nullptr)
			{
				return false;
			}
			if (root->_key < key)
			{
				return _EraseR(root->_right, key);
			}
			else if(root->_key > key)
			{
				return _EraseR(root->_left, key);
			}
			else
			{
				//如果右子树为空
				if (root->_right == nullptr)
				{
					Node* del = root;
					root = root->_left;
					delete del;
					return true;
				}
				else if(root->_left == nullptr)
				{
					Node* del = root;
					root = root->_right;
					delete del;
					return true;
				}
				else
				{
					Node* rightMin = root->_right;
					while (rightMin->_left)
					{
						rightMin = rightMin->_left;
					}
					root->_key = rightMin->_key;
					return _EraseR(root->_right, rightMin->key);
				}
			}
		}

	private:
		Node* _root = nullptr;
	};
}
