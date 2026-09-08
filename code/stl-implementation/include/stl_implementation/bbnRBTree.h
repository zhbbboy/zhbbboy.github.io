#pragma once

#include<iostream>
#include<vector>

namespace bbn
{

	enum Color
	{
		RED,
		BLACK
	};

	template<class T>
	struct RBTreeNode
	{
		RBTreeNode(const T& data)
			: _data(data)
			, _left(nullptr)
			, _right(nullptr)
			, _parent(nullptr)
		{
		}

		T _data;

		RBTreeNode<T>* _left;
		RBTreeNode<T>* _right;
		RBTreeNode<T>* _parent;

		Color _color;
	};

	//红黑树迭代器的实现
	template<class T, class Ref, class Ptr>
	struct RBTreeIterator
	{
		typedef RBTreeNode<T> Node;
		typedef RBTreeIterator<T, Ref, Ptr> Self;

		RBTreeIterator(Node* node, Node* root)
			:_node(node)
			, _root(root)
		{
		}

		Self& operator++()
		{
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

		Self& operator--()
		{
			// --end()
			if (_node == nullptr)
			{
				Node* max = _root;

				while (max && max->_right)
				{
					max = max->_right;
				}

				_node = max;
			}
			else if (_node->_left)
			{
				Node* max = _node->_left;

				while (max->_right)
				{
					max = max->_right;
				}

				_node = max;
			}
			else
			{
				Node* cur = _node;
				Node* parent = cur->_parent;

				while (parent && cur == parent->_left)
				{
					cur = parent;
					parent = parent->_parent;
				}

				_node = parent;
			}

			return *this;
		}

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
		Node* _root;
	};

	//红黑树模拟实现
	template<class K,class T,class KeyOfT>
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


		std::pair<Iterator,bool> insert(const T& data)
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
				if (kot(cur->_data) > kot(data))
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
					return { Iterator(cur,_root),false};
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
				Node* grandfater = parent->_parent;

				if (grandfater->_left == parent->_parent)
				{
					Node* uncle = grandfater->_right;

					if (uncle && uncle->_color == RED)
					{
						parent->_color = uncle->_color = BLACK;
						grandfater->_color = RED;

						// 继续向上调整
						cur = grandfater;
						parent = grandfater->_parent;
					}
					// uncle不存在或存在且为黑
					else
					{
						if (parent->_left == cur)
						{
							RotateR(grandfater);

							parent->_color = BLACK;
							grandfater->_color = RED;
						}
						else
						{
							RotateL(parent);
							RotateR(grandfater);

							cur->_color = BLACK;
							grandfater->_color = RED;
						}
						break;
					}
				}
				else// grandfather->_right == parent
				{
					Node* uncle = grandfater->_left;

					if (uncle && uncle->_color == RED)
					{
						parent->_color = uncle->_color = BLACK;
						grandfater->_color = RED;

						cur = grandfater;
						parent = grandfater->_parent;
					}
					else
					{
						if (parent->_right == cur)
						{
							RotateL(grandfater);

							parent->_color = BLACK;
							grandfater->_color = RED
						}
						else
						{
							RotateR(parent);
							RotateL(grandfater);

							cur->_color = BLACK;
							grandfater->_color = RED;
						}
						break;
					}
				}
			}
			//父节点是红色，叔叔结点也是红色（左右子情况均包含）,解决：把父辈节点全部变成黑色，祖父结点变成红色
			// 祖父结点(G)是根结点祖父结点自身改为黑色即可
			_root->_color = BLACK;

			return {Iterator(newnode,_root),true};
		}

		void RotateR(Node* parent)
		{
			Node* subL = parent->_left;
			Node* subLR = subL->_right;

			parent->_left = subLR;

			if (subLR)
			{
				subLR->_parent = parent;
			}

			Node* pparent = parent->_parent;
			parent->_parent = subL;
			subL->_right = parent;

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

	private;
	Node* _root = nullptr;
	};



	


}
