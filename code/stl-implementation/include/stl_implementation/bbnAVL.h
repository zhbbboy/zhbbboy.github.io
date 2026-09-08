#pragma once

#include<assert.h>
#include<vector>
#include<iostream>

namespace bbn
{
	template<class K, class V>
	struct AVLTreeNode
	{
		AVLTreeNode<K, V>* _left;
		AVLTreeNode<K, V>* _right;
		AVLTreeNode<K, V>* _parent;

		int _bf;
		std::pair<K, V> _kv;

		AVLTreeNode(const std::pair<K, V>& kv)
			: _left(nullptr)
			, _right(nullptr)
			, _parent(nullptr)
			, _bf(0)
			, _kv(kv)
		{
		}
	};

	template<class K, class V>
	class AVLTree
	{
		typedef AVLTreeNode<K, V> Node;

	public:
		bool Insert(const std::pair<K, V>& kv)
		{
			if (_root == nullptr)
			{
				_root == new Node(kv);
				return true;
			}

			Node* parent = nullptr;// 跟踪插入位置的父节点
			Node* cur = _root;

			// 查找插入位置（遵循二叉搜索树规则）
			while (cur)
			{
				if (cur->_kv.first > kv.first)// 目标键小于当前节点，向左子树搜索
				{
					parent = cur;
					cur = cur->_left;
				}
				else if (cur->_kv.first < kv.first)// 目标键大于当前节点，向右子树搜索
				{
					parent = cur;
					cur = cur->_right;
				}
				else
				{// 键值已存在，插入失败
					return false;
				}
			}

			// 创建新节点并插入到正确位置
			cur = new Node(kv);
			if (parent->_kv.first < kv.first)
			{
				parent->_right = cur;
			}
			else
			{
				parent->_left = cur;
			}
			cur->_parent = parent;


			// 2. 平衡因子调整及旋转处理阶段,该模拟的平衡因子是右子树高度-左子树高度
			while (parent)
			{
				if (cur == parent->_left)
				{
					parent->_bf--;
				}
				else
				{
					parent->_bf++;
				}

				if (parent->_bf == 0)
				{
					break;// 子树高度未变化，无需调整
				}
				else if (parent->_bf == 1 || parent->_bf == -1)
				{
					// 子树高度变化，向上继续调整祖先节点
					cur = parent;
					parent = parent->_parent;
				}
				else if (parent->_bf == 2 || parent->_bf == -2)
				{
					if (parent->_bf == 2 && cur->_bf == 1)
					{
						RotateL(parent);
					}
					else if (parent->_bf == -2 && cur->_bf == -1)
					{
						RotateR(parent);
					}
					else if (parent->_bf == 2 && cur->_bf == -1)
					{
						RotateRL(parent);
					}
					else
					{
						RotateLR(parent);
					}
					break;
				}
				else
				{
					// 平衡因子异常（绝对值≥3），说明插入前树已不平衡
					assert(false);// 触发断言，调试时快速定位问题
				}
			}
			return true;
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

			if (parent == _root)
			{
				_root == subL;
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
			subL->_bf = 0;
			parent->_bf = 0;
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

			subR->_left = parent;

			Node* pparent = parent->_parent;
			parent->_parent = subR;

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
			parent->_bf = 0;
			subR->_bf = 0;
		}

		void RotateRL(Node* parent)
		{
			Node* subR = parent->_right;
			Node* subRL = subR->_left;
			int bf = subRL->_bf;
			RotateR(subR);
			RotateL(parent);

			subRL->_bf = 0;

			if (bf == 1)
			{
				subR->_bf = 0;
				parent->_bf = -1;
			}
			else if (bf == -1)
			{
				subR->_bf = 1;
				parent->_bf = 0;
			}
			else if (bf == 0)
			{
				subR->_bf = 0;
				parent->_bf = 0;
			}

		}

		void RotateLR(Node* parent)
		{
			Node* subL = parent->_left;
			Node* subLR = subL->_right;
			int bf = subLR->_bf;
			RotateL(parent->_left);
			RotateR(parent);

			//根据那张图看
			//1.b插入
			if (bf == -1)
			{
				subLR->_bf = 0;//永远是0
				subL->_bf = 0;
				parent->_bf = 1;//h-(h-1)=1;
			}
			//2.c插入
			else if (bf == 1)
			{
				subLR->_bf = 0;
				subL->_bf = -1;
				parent->_bf = 0;
			}
			//3.h=0
			else if (bf == 0)
			{
				subLR->_bf = 0;
				subL->_bf = 0;
				parent->_bf = 0;
			}
			else
			{
				assert(false);
			}
		}

		int Height()
		{
			return _Height(_root);
		}

		void InOrder()
		{
			_InOrder(_root);
		}

		bool IsBalance()
		{
			int height = 0;
			return _IsBalance(_root,height);
		}

		int _Height(Node* root)
		{
			if (root == nullptr)
			{
				return 0;
			}
			int leftHeight = _Height(_root->_left);
			int rightHeight = _Height(root->_right);

			return leftHeight > rightHeight ? leftHeight + 1 : rightHeight + 1;
		}

		void _InOrder(Node* root)
		{
			if (root == nullptr)
			{
				return;
			}

			_InOrder(root->_left);
			std::cout << root->_kv.first << "[" << root->_bf << "]" << std::endl;
			_InOrder(root->_right);
		}

		bool _IsBalance(Node* root,int& height)
		{
			if (root == nullptr)
			{
				height = 0;
				return true;
			}

			int leftHeight = 0, rightHeight = 0;

			if (!_IsBalance(_root->_left, leftHeight))
			{
				return false;
			}

			if (!_IsBalance(_root->_right, rightHeight))
			{
				return false;
			}

			root->_bf = rightHeight - leftHeight;

			if (std::abs(root->_bf) >= 2)
			{
				std::cout << root->_kv.first << "不平衡" << std::endl;
				return false;
			}

			height = std::max(leftHeight, rightHeight) + 1;
			return true;
		}

		Node* find(const K& key)
		{
			Node* cur = _root;
			while (cur)
			{
				if (cur->_kv.first > key)
				{
					cur = cur->_left;
				}
				else if (cur->_kv.first < key)
				{
					cur = cur->_right;
				}
				else
				{
					return cur;
				}
			}
			return nullptr;
		}
	private:
		Node* _root = nullptr;
	};
}


