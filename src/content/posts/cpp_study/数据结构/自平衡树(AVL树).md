---
title: 自平衡树(AVL树)
date: 2026-09-01
tags: []
draft: false
---

## AVL树的概念

二叉搜索树虽可以缩短查找的效率，但如果数据有序或接近有序二叉搜索树将退化为单支树，查找元素相当于在顺序表中搜索元素，效率低下。因此，两位俄罗斯的数学家G.M.Adelson-Velskii和E.M.Landis在1962年发明了一种解决上述问题的方法：当向二叉搜索树中插入新结点后，如果能保证每个结点的左右子树高度之差的绝对值不超过1(需要对树中的结点进行调整)，即可降低树的高度，从而减少平均搜索长度。

一棵AVL树或者是空树，或者是具有以下性质的二叉搜索树：
- 它的左右子树都是AVL树
- 左右子树高度之差(简称平衡因子)的绝对值不超过1(-1/0/1)
![](../../_Pasted_Image/Pasted%20image%2020260901143550.png)

AVL 树（Adelson-Velsky and Landis Tree）也叫做**平衡二叉树**（平衡二叉排序树）是一种**自平衡的**二叉搜索树（BST），它通过严格的平衡因子控制树的高度，从而在插入、删除和查找操作中保持 `O(log n)` 的时间复杂度。

- **平衡因子（bf）**：结点的左子树的深度减去右子树的深度。
- **定义**：AVL 树是一种**高度平衡的二叉搜索树**，其每个节点的**平衡因子**满足：  
    平衡因子 = 左子树高度 - 右子树高度，范围为 `-1, 0, 1`。
- **自平衡的核心思想**：通过旋转操作（单旋转和双旋转），在插入或删除节点后重新调整树的结构，使其维持平衡。
![](../../_Pasted_Image/Pasted%20image%2020260901143701.png)

![](../../_Pasted_Image/Pasted%20image%2020260901143705.png)

由 AVL树的定义可知，下面的两个图都不是 AVL树（平衡二叉树） 。
![](../../_Pasted_Image/Pasted%20image%2020260901143717.png)

**AVL树的作用：**
我们都了解，对于常见的二叉搜索树（Binary Search Tree）来说，当它是一棵平衡树时，期望高度为 log₂n ，而这个高度同时也决定了该树各个操作的时间复杂度为 O(log₂n)。 不过，在某些极端状况下，例如插入的序列呈现有序状态时，二叉搜索树就会退化为近似链或者链的形态。在这种情况下，其操作的时间复杂度会从 O(log₂n) 退化到线性的 O(n)。 为了尽可能避免出现上述情况，我们可以采用随机化的方式来构建二叉搜索树。但在经过多次操作之后，问题又会出现。因为在执行删除操作时，我们通常会选择用待删除节点的后继节点来替代它。这样做会导致右侧节点数量持续减少，使得树逐渐向左倾斜。而树的平衡性一旦被破坏，其操作的时间复杂度也会随之提高。

例如：我们按顺序将一组数据 ` 1,2,3,4,5,6` 分别插入到一颗空二叉查找树和AVL树中，插入的结果如下图：
![](../../_Pasted_Image/Pasted%20image%2020260901143733.png)
由上图可知，同样的结点，由于插入方式不同导致树的高度也有所不同。特别是在带插入结点个数很多且正序的情况下，会导致二叉树 的高度是O(N)，而AVL树就不会出现这种情况，树的高度始终是O(lgN).高度越小，对树的一些基本操作的时间复杂度就会越小。这也就是我们引入AVL树的原因。

下图也是一个例子。
![](../../_Pasted_Image/Pasted%20image%2020260901143743.png)

## AVL树节点的定义
```cpp
template<class K,class V>//用来声明这个结构体是一个模板类
struct AVLTreeNode
{
    //三叉链
	AVLTreeNode<K, V>* _left;
	AVLTreeNode<K, V>* _right;
	AVLTreeNode<K, V>* _parent;
	int _bf;//平衡因子
	//键值对
    pair<K, V> _kv; 

    //构造函数
	AVLTreeNode(const pair<K, V>& kv)
		:_left(nullptr)
		, _right(nullptr)
		, _parent(nullptr)
		,_bf(0)
		,_kv(kv)
	{}
};

```
## AVL树的插入

AVL树就是在二叉搜索树的基础上引入了平衡因子，因此AVL树也可以看成是二叉搜索树。那么  
AVL树的插入过程可以分为三步：
1. 按照二叉搜索树的插入方法，找到待插入位置。
2. 找到待插入位置后，将待插入结点插入到树中。
3. **更新平衡因子，如果出现不平衡，则需要进行旋转。**

当我们插入一个新节点时，这个节点的插入会影响从新节点到根节点路径上所有祖先节点的平衡因子。![](../../_Pasted_Image/Pasted%20image%2020260901143823.png)

关键点如下：

- **更新平衡因子的规则：**
    - 在插入节点时，首先通过常规的二叉查找树插入操作把节点插入树中。
	-  然后从**插入节点的父节点**开始，**逐层**更新祖先节点的平衡因子。每个祖先节点的平衡因子会根据其左右子树的高度变化而发生改变。
- **具体的更新过程：**
    - 假设我们插入的节点是`c`，它的父节点是`p`，`p`的父节点是`g`，以此类推。这些节点的平衡因子都会受到影响。
    - 在更新平衡因子时，根据以下原则：

这个操作会一直向上更新，直到：

```
 这里的bf是按右子树减左子树算的
```

- **如果**`c`**是**`p`**的左孩子**，那么`p`的`BF`会减1 (`p->bf--`)；
    
- **如果**`c`**是**`p`**的右孩子**，那么`p`的`BF`会加1 (`p->bf++`)。


1. p的平衡因子变为0：说明p的左右子树在更新后平衡了，p的高度没有变化，更新结束，不需要进一步的旋转。

	`只有-1/1经过+ + /− −操作后会变成0，说明新结点插入到了parent左右子树当中高度较矮的一棵子树，插入后使得parent左右子树的高度相等了，此操作并没有改变以parent为根结点的子树的高度，从而不会影响parent的父结点的平衡因子，因此无需继续往上更新平衡因子。`

![](../../_Pasted_Image/Pasted%20image%2020260901143940.png)

2. p的平衡因子变为±1：说明p的子树高度变了，但树还是平衡的，不需要旋转，继续向上更新。

	`只有0经过− −/+ +操作后会变成-1/1，说明新结点的插入使得parent的左子树或右子树增高了，即改变了以parent为根结点的子树的高度，从而会影响parent的父结点的平衡因子，因此需要继续往上更新平衡因子。`
![](../../_Pasted_Image/Pasted%20image%2020260901144000.png)

3. p的平衡因子变为±2：表示p的子树不再平衡，需要进行**旋转**来恢复平衡。
` 此时parent结点的左右子树高度之差的绝对值已经超过1了，不满足AVL树的要求，因此需要进行旋转处理。`

**注意：** parent的平衡因子在更新前只可能是-1/0/1（AVL树中每个结点的左右子树高度之差的绝对值不超过1）。
![](../../_Pasted_Image/Pasted%20image%2020260901144025.png)
而在最坏情况下，我们更新平衡因子时会一路更新到根结点。例如下面这种情况：
![](../../_Pasted_Image/Pasted%20image%2020260901144035.png)
- `cur`：表示当前处理的节点（可能是插入节点，也可能是插入节点的父节点、祖父节点等）。
- `parent`：表示 `cur` 的父节点，用来在每一步更新平衡因子后往上继续追溯。

**第一次更新**：插入节点后，先更新其父节点 `parent` 的平衡因子。

**如果需要继续向上更新**：则会将 `cur` 设置为 `parent`，`parent` 设置为 `parent->_parent`，继续往上更新。
**关键点在于：当发现某个节点（parent）的平衡因子变为 ±2 时，需要进行旋转。但此时 cur 的平衡因子一定是 ±1，而不可能是 0。**

**1. 为什么 `parent` 的平衡因子为 ±2 时，`cur` 的平衡因子必定是 ±1??**
这里的 `cur` 指的是引起 `parent` 节点出现平衡因子 ±2 的那个子节点。

**2. 若 `cur` 的平衡因子是 0 会发生什么？**
- 如果 `cur` 的平衡因子是 0，意味着 `cur` 节点是**新增的节点**，而且它在插入后本身就没有导致子树高度差的变化（或者说 `cur` 自己还是一个高度为 1 的子树）。
- 但如果 `cur` 真的是“上一次更新平衡因子时的 `parent`”，它不可能是 0，因为**在插入节点导致向上更新的过程中，如果遇到一个节点的平衡因子是 0，就会停止向上更新**（因为这代表该子树高度没有再继续增加）。

**3. 新增节点插入时对父节点平衡因子的影响**  
插入节点到一棵子树时，其父节点 `parent` 的平衡因子可能从以下几种状态转变：
1. **父节点原本平衡因子为 0**，插入节点后变为 ±1。
2. **父节点原本平衡因子为 ±1**，插入到父节点的空子树后可能变为 0。
在这两种情况下都不可能直接出现 parent->_ bf 为 ±2，除非继续往上更新并发现祖父节点出现了更高的高度差。也就是说，如果某个节点 parent 的平衡因子直接跳到 ±2，那么它的子节点（即我们称的 cur）的平衡因子一定是 ±1，而不可能是 0。

换言之，只有在“子节点非 0”时，父节点才可能一下子出现 ±2 的高度差，否则插入一个新节点（平衡因子为 0）不会导致父节点直接出现 ±2。

根据以上结论，我们将旋转分为下面四类。

**旋转处理的四种情况**
当 parent->_ bf == -2 或 2 时，具体需要做哪种旋转，要看 cur->_ bf 的值：

```
1. parent->_bf == -2 且 cur->_bf == -1：右单旋（RR型失衡）
2. parent->_bf == -2 且 cur->_bf == 1：左右双旋（LR型失衡）
3. parent->_bf == 2 且 cur->_bf == -1：右左双旋（RL型失衡）
4. parent->_bf == 2 且 cur->_bf == 1：左单旋（LL型失衡）
```
这里，“单旋”或“双旋”指的是 AVL 树平衡处理中的旋转方式。**单旋**用于子节点和父节点在同一方向（LL 或 RR），**双旋**用于子节点和父节点方向相反（LR 或 RL）。

**为什么旋转后无需继续往上更新**：  
旋转会将树的高度恢复到插入前的高度，因此再往上的父节点的平衡因子并不会受影响，所以可以停止更新。

具体的旋转图解我们在下一步细致讲解，这里先给出代码
```cpp
bool Insert(const pair<K, V>& kv) {
	// 1. 二叉搜索树插入阶段
	if (_root == nullptr) {            // 空树直接创建根节点
		_root = new Node(kv);
		return true;
	}

	Node* parent = nullptr;            // 跟踪插入位置的父节点
	Node* cur = _root;
	// 查找插入位置（遵循二叉搜索树规则）
	while (cur) {
		if (cur->_kv.first > kv.first) {  // 目标键小于当前节点，向左子树搜索
			parent = cur;
			cur = cur->_left;
		}
		else if (cur->_kv.first < kv.first) { // 目标键大于当前节点，向右子树搜索
			parent = cur;
			cur = cur->_right;
		}
		else {                        // 键值已存在，插入失败
			return false;
		}
	}

	// 创建新节点并插入到正确位置
	cur = new Node(kv);                // 新节点初始化（bf=0，parent=nullptr）
	if (parent->_kv.first < kv.first) { // 插入到父节点的右子树
		parent->_right = cur;
	}
	else {                            // 插入到父节点的左子树
		parent->_left = cur;
	}
	cur->_parent = parent;             // 设置新节点的父指针

	// 2. 平衡因子调整及旋转处理阶段
	while (parent) {                   // 从插入点向上回溯调整平衡因子
		// 更新父节点平衡因子
		if (cur == parent->_left) {    // 插入在左子树，父节点bf减1
			parent->_bf--;
		}
		else {                       // 插入在右子树，父节点bf加1
			parent->_bf++;
		}

		// 根据平衡因子决定后续操作
		if (parent->_bf == 0) {        // 子树高度未变化，无需调整
			break;
		}
		else if (parent->_bf == 1 || parent->_bf == -1) {
			// 子树高度变化，向上继续调整祖先节点
			cur = parent;
			parent = parent->_parent;
		}
		else if (parent->_bf == 2 || parent->_bf == -2) {
			// 平衡因子绝对值超过1，需旋转调整
			if (parent->_bf == 2 && cur->_bf == 1) {       // 右子树过深，左单旋
				RotateL(parent);
			}
			else if (parent->_bf == -2 && cur->_bf == -1) { // 左子树过深，右单旋
				RotateR(parent);
			}
			else if (parent->_bf == -2 && cur->_bf == 1) {  // 左子树的右子树过深，先左旋后右旋（LR型）
				RotateLR(parent);
			}
			else {                                        // 右子树的左子树过深，先右旋后左旋（RL型）
				RotateRL(parent);
			}
			break;  // 旋转后子树高度恢复，无需继续向上调整
		}
		else {
			// 平衡因子异常（绝对值≥3），说明插入前树已不平衡
			assert(false);  // 触发断言，调试时快速定位问题
		}
	}
	return true;
}
```
## AVL树的旋转

在AVL树中，如果在插入操作之后，某个节点的平衡因子变为`±2`，则需要进行旋转来恢复平衡。旋转操作有四种类型，取决于插入节点的相对位置（左或右）：

在讲旋转之前，大家一定要弄清楚命名规则：

**LL、RR、LR、RL 描述的是什么？**
- **LL 失衡**：表示 **父节点**(记为 `P`) 的平衡因子为 +2（左子树过高），并且 **子节点**(记为 `C`) 的平衡因子为 +1（左子树也过高）。
	- “L” 指的就是 **左**，第一个 “L” 表示 `P` 的左子树高，第二个 “L” 表示 `C` 的左子树高。
	-  LL 型失衡需要 **右单旋** 来修复。
- **RR 失衡**：表示 `P` 的平衡因子为 -2（右子树过高），并且 `C` 的平衡因子为 -1（右子树也过高）。
	- “R” 指的是 **右**，第一个 “R” 表示 `P` 的右子树高，第二个 “R`表示`C` 的右子树高。
	- RR 型失衡需要 **左单旋** 来修复。
- **LR 失衡**：表示 `P` 的平衡因子为 +2（左子树过高），但 `C` 的平衡因子为 -1（右子树高）。
    - 这是一个 “左-右” 失衡，需要先 **左旋** 子树，再 **右旋** 父节点，称为 **左右双旋**。
- **RL 失衡**：表示 `P` 的平衡因子为 -2（右子树过高），但 `C` 的平衡因子为 +1（左子树高）。
    - 这是一个 “右-左” 失衡，需要先 **右旋** 子树，再 **左旋** 父节点，称为 **右左双旋**。

**关键**：**LL**、**RR**、**LR**、**RL** 这些命名是根据“哪边子树过高”来命名的，并不是根据旋转方向命名。

#### 左单旋-向左旋转（RR）

**左旋（Left Rotation）：**  
发生在右子树比左子树高的情况下，即平衡因子为`-2`。在右子树的右侧插入节点时，需要进行左旋。
![](../../_Pasted_Image/Pasted%20image%2020260901144517.png)
![](../../_Pasted_Image/Pasted%20image%2020260901144521.png)
![](../../_Pasted_Image/Pasted%20image%2020260901144527.png)

**左单旋的步骤如下：**
1. 让subR的左子树作为parent的右子树。
2. 让parent作为subR的左子树。
3. 让subR作为整个子树的根。
4. 更新平衡因子。

**代码如下：**
```cpp
void RotateL(Node* parent)
{
    Node* subR = parent->_right;
    Node* subRL = subR->_left;
    // `subR` 是 `parent` 的右子节点，旋转后的新根
    // `subRL` 是 `subR` 的左子节点，在旋转后将成为 `parent` 的右子节点

    // 1. 将 `parent` 的右子树更新为 `subR` 的左子树
    parent->_right = subRL;

    // 2. 更新 `subRL` 的父节点为 `parent`
    if (subRL)
        subRL->_parent = parent;

    // 3. 将 `subR` 的左子树更新为 `parent`
    subR->_left = parent;

    // 4. 更新 `parent` 的父节点指向 `subR`
    Node* ppnode = parent->_parent;
    parent->_parent = subR;

    // 5. 如果 `parent` 是根节点，更新 `_root`
    if (parent == _root)
    {
        _root = subR;
        subR->_parent = nullptr;  // `subR` 成为新的根节点
    }
    else
    {
        // 如果 `parent` 不是根节点，则需要调整父节点指向
        if (ppnode->_left == parent)
        {
            ppnode->_left = subR;
        }
        else
        {
            ppnode->_right = subR;
        }
        subR->_parent = ppnode;
    }

    // 6. 更新平衡因子，旋转后 `parent` 和 `subR` 平衡因子都恢复为 0
    parent->_bf = 0;
    subR->_bf = 0;
}
```

#### 右单旋-向右旋转(LL)

发生在左子树比右子树高的情况下，即平衡因子为`+2`。在左子树的左侧插入节点时，需要进行右旋。![](../../_Pasted_Image/Pasted%20image%2020260901144556.png)
![](../../_Pasted_Image/Pasted%20image%2020260901144601.png)
![](../../_Pasted_Image/Pasted%20image%2020260901144604.png)

**右单旋的步骤如下：**
1. 让subL的右子树作为parent的左子树。
2. 让parent作为subL的右子树。
3. 让subL作为整个子树的根。
4. 更新平衡因子。

**代码如下：**
```cpp
	void RotateR(Node* parent)
	{
		Node* subL = parent->_left;
		Node* subLR = subL->_right;
		// `subL` 是 `parent` 的左子节点，即旋转后的新根
		// `subLR` 是 `subL` 的右子节点，在旋转后将成为 `parent` 的左子节点

		// 1. 将 `parent` 的左子树更新为 `subL` 的右子树
		parent->_left = subLR;

		// 2. 更新 `subLR` 的父节点为 `parent`
		if (subLR)
			subLR->_parent = parent;

		// 3. 将 `subL` 的右子树更新为 `parent`
		subL->_right = parent;

		// 4. 更新 `parent` 的父节点指向 `subL`
		Node* ppnode = parent->_parent;
		parent->_parent = subL;

		// 5. 如果 `parent` 是根节点，更新 `_root`
		if (parent == _root)
		{
			_root = subL;
			subL->_parent = nullptr;  // `subL` 成为新的根节点
		}
		else
		{
			// 如果 `parent` 不是根节点，则需要调整父节点指向
			if (ppnode->_left == parent)
			{
				ppnode->_left = subL;
			}
			else
			{
				ppnode->_right = subL;
			}
			subL->_parent = ppnode;
		}

		// 6. 更新平衡因子，旋转后 `parent` 和 `subL` 平衡因子都恢复为 0
		subL->_bf = 0;
		parent->_bf = 0;
	}
```

#### 左右双旋-LR

- **左-右旋（Left-Right Rotation）：**  
    发生在**左**子树的**右**侧插入节点时。首先对左子树进行左旋，然后对根节点进行右旋。
![](../../_Pasted_Image/Pasted%20image%2020260901144634.png)由于插入新结点后，可能并不会立即进行旋转操作，而可能是在更新其祖先结点的过程中出现了不平衡而进行的旋转操作，因此用长方形表示下面的子树。

动图中，在b子树当中新增结点，**或是在c子树当中新增结点**，均会引发左右双旋，动图中以在b子树当中新增结点为例。

**动图当中的旋转示意图如下：**

1、插入新结点。
![](../../_Pasted_Image/Pasted%20image%2020260901144653.png)
2、以30为旋转点进行左单旋。
![](../../_Pasted_Image/Pasted%20image%2020260901144700.png)
3、以90为旋转点进行右单旋。
![](../../_Pasted_Image/Pasted%20image%2020260901144707.png)

**左右双旋的步骤如下：**

以subL为旋转点进行左单旋。

以parent为旋转点进行右单旋。

更新平衡因子。

**左右双旋后，平衡因子的更新随着subLR原始平衡因子的不同分为以下三种情况：**  
1、当subLR原始平衡因子是-1时，左右双旋后parent、subL、subLR的平衡因子分别更新为1、0、0。
![](../../_Pasted_Image/Pasted%20image%2020260901144719.png)

2、当subLR原始平衡因子是1时，左右双旋后parent、subL、subLR的平衡因子分别更新为0、-1、0。
![](../../_Pasted_Image/Pasted%20image%2020260901144727.png)
3、当subLR原始平衡因子是0时，左右双旋后parent、subL、subLR的平衡因子分别更新为0、0、0。![](../../_Pasted_Image/Pasted%20image%2020260901144734.png)
```cpp
void  RotateLR(Node* parent)
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
```
#### 右左双旋-RL

- **右-左旋（Right-Left Rotation）：**  
    发生在**右**子树的**左**侧插入节点时。首先对右子树进行右旋，然后对根节点进行左旋。

![](../../_Pasted_Image/Pasted%20image%2020260901144750.png)
动图中，在b子树当中新增结点，**或是**在c子树当中新增结点，均会引发右左双旋，动图中以在c子树当中新增结点为例。

**动图当中的旋转示意图如下：**  
1、插入新结点。
![](../../_Pasted_Image/Pasted%20image%2020260901144759.png)

2、以90为旋转点进行右单旋。
![](../../_Pasted_Image/Pasted%20image%2020260901145040.png)

3、以30为旋转点进行左单旋。
![](../../_Pasted_Image/Pasted%20image%2020260901145048.png)
**右左双旋的步骤如下：**
1. 以subR为旋转点进行右单旋。
2. 以parent为旋转点进行左单旋。
3. 更新平衡因子。

**右左双旋后，平衡因子的更新随着subLR原始平衡因子的不同分为以下三种情况：**  
1、当subRL原始平衡因子是1时，左右双旋后parent、subR、subRL的平衡因子分别更新为-1、0、0。
![](../../_Pasted_Image/Pasted%20image%2020260901145102.png)

2、当subRL原始平衡因子是-1时，左右双旋后parent、subR、subRL的平衡因子分别更新为0、1、0。![](../../_Pasted_Image/Pasted%20image%2020260901145111.png)

3、当subRL原始平衡因子是0时，左右双旋后parent、subR、subRL的平衡因子分别更新为0、0、0。![](../../_Pasted_Image/Pasted%20image%2020260901145119.png)

代码如下：
```cpp
void RotateRL(Node* parent)
{
    // 取出 parent 的右子节点，记为 subR
    Node* subR = parent->_right;
    // 取出 subR 的左子节点，记为 subRL
    Node* subRL = subR->_left;
    // 记录 subRL 的平衡因子，后续用于调整旋转后各节点的平衡因子
    int bf = subRL->_bf;

    // 第一步：对 subR 进行右旋操作
    // 将 parent 的右子节点更新为 subRL，即让 subRL 替代 subR 在 parent 右子树的位置
    parent->_right = subRL;
    // 将 subR 的左子节点更新为 subRL 的右子节点
    subR->_left = subRL->_right;
    // 如果 subRL 的右子节点不为空，更新其父亲节点为 subR
    if (subRL->_right)
        subRL->_right->_parent = subR;
    // 将 subRL 的右子树设置为 subR
    subRL->_right = subR;
    // 更新 subR 的父亲节点为 subRL
    subR->_parent = subRL;

    // 第二步：对 parent 进行左旋操作
    // 保存 parent 的父亲节点，用于后续更新树的连接关系
    Node* ppnode = parent->_parent;
    // 将 parent 的右子节点更新为 subRL 的左子节点
    parent->_right = subRL->_left;
    // 如果 subRL 的左子节点不为空，更新其父亲节点为 parent
    if (subRL->_left)
        subRL->_left->_parent = parent;
    // 将 subRL 的左子节点更新为 parent
    subRL->_left = parent;
    // 更新 parent 的父亲节点为 subRL
    parent->_parent = subRL;

    // 第三步：处理父节点 ppnode 与 subRL 的连接关系
    if (ppnode == nullptr)
    {
        // 如果 parent 是根节点，旋转后 subRL 成为新的根节点
        _root = subRL;
        // 新根节点的父亲节点为空
        subRL->_parent = nullptr;
    }
    else
    {
        // 如果 parent 不是根节点，判断 parent 是 ppnode 的左子节点还是右子节点
        if (ppnode->_left == parent)
        {
            // 如果 parent 是 ppnode 的左子节点，将 ppnode 的左子节点更新为 subRL
            ppnode->_left = subRL;
        }
        else
        {
            // 如果 parent 是 ppnode 的右子节点，将 ppnode 的右子节点更新为 subRL
            ppnode->_right = subRL;
        }
        // 更新 subRL 的父亲节点为 ppnode
        subRL->_parent = ppnode;
    }

    // 第四步：根据 subRL 原来的平衡因子 bf 来更新旋转后各节点的平衡因子
    // 旋转后 subRL 成为新的子树根节点，其平衡因子必然为 0
    subRL->_bf = 0;
    if (bf == 1)
    {
        // 如果原来 subRL 的平衡因子为 1，说明插入操作发生在 subRL 的右子树
        // 旋转后 subR 的平衡因子变为 0
        subR->_bf = 0;
        // 旋转后 parent 的平衡因子变为 -1
        parent->_bf = -1;
    }
    else if (bf == -1)
    {
        // 如果原来 subRL 的平衡因子为 -1，说明插入操作发生在 subRL 的左子树
        // 旋转后 subR 的平衡因子变为 1
        subR->_bf = 1;
        // 旋转后 parent 的平衡因子变为 0
        parent->_bf = 0;
    }
    else if (bf == 0)
    {
        // 如果原来 subRL 的平衡因子为 0，说明插入操作导致刚好平衡
        // 旋转后 subR 的平衡因子变为 0
        subR->_bf = 0;
        // 旋转后 parent 的平衡因子变为 0
        parent->_bf = 0;
    }
}
```

## AVL树的验证

1. **验证是否为二叉搜索树**

首先，AVL 树本质上是**一棵平衡的二叉搜索树**。所以要验证它是不是 AVL 树，**第一步**要先确保它是一棵**二叉搜索树**（BST）。

- **方法**：进行**中序遍历**，如果遍历结果是一个递增的有序序列，就说明这棵树满足 BST 的性质。
- **示例代码**：
```cpp
void _InOrder(Node* root)
{
    if (root == nullptr)
        return;
    _InOrder(root->_left);
    cout << root->_kv.first << "[" << root->_bf << "]" << endl;
    _InOrder(root->_right);
}

void InOrder()
{
//return _InOrder(root);
//_InOrder 是一个 void 类型函数，即它没有返回值。因此调用 _InOrder(root); 后，它不返回任何内容
//用return是多余的
    _InOrder(_root);
}
```
在这个中序遍历过程中，如果输出的键值（`_kv.first`）是按从小到大的顺序排布，说明它满足二叉搜索树的特性。

---

2. **验证平衡性**

仅仅是 BST 还不够，AVL 树额外要求**任意节点的左右子树高度差的绝对值不超过 1**。因此，**第二步**要验证平衡性。这里可以分为两种方法：
1. **低效方法：`IsBalance1`**

`IsBalance1` 的逻辑如下：
1. 对每个节点，计算它的左右子树高度（调用 `Height` 函数）。
2. 判断该节点左右子树高度差是否小于 2；如果不是，说明不平衡。
3. 同时，还检查该节点的 `_bf`（平衡因子）是否等于 `rightHeight - leftHeight`。如果不一致，说明平衡因子更新有错误。
4. 递归检查左右子树，直到整棵树都检查完毕。

**低效** 的原因在于：对每个节点都要重新调用 `Height` 函数来计算左右子树高度，导致重复计算，整体复杂度较高。

```cpp
bool _IsBalance1(Node* root)
{
    if (root == nullptr)
        return true;

    int leftheight = Height(root->_left);
    int rightheight = Height(root->_right);

    // 高度差绝对值≥2，说明不平衡
    if (abs(rightheight - leftheight) >= 2)
    {
        cout << root->_kv.first << "不平衡" << endl;
        return false;
    }
    // 检查平衡因子是否正确
    if (rightheight - leftheight != root->_bf)
    {
        cout << root->_kv.first << "平衡因子异常" << endl;
        return false;
    }

    // 递归检查左右子树
    return _IsBalance1(root->_left) && _IsBalance1(root->_right);
}

bool IsBalance1()
{
    return _IsBalance1(_root);
}
```
2. **更高效的方法：`IsBalance`**

`IsBalance` 采用了**后序遍历**的思路，一边计算子树高度，一边判断是否平衡，避免重复计算高度。它在一次递归中既获取左右子树的高度，又判断平衡性：
1. 如果节点是空，返回高度 0，并标记它是平衡的。
2. 先递归检查左子树是否平衡，并得到左子树高度。
3. 再递归检查右子树是否平衡，并得到右子树高度。
4. 计算当前节点的平衡因子 `_bf = rightHeight - leftHeight`，若绝对值≥2，则不平衡。
5. 计算当前节点的高度 `height = max(leftHeight, rightHeight) + 1` 并返回给上一层。

这样只需要一次后序遍历，就能在获取高度的同时检查平衡性。效率更高。
```cpp
bool _IsBalance(Node* root, int& height)
{
    if (root == nullptr)
    {
        height = 0;
        return true;
    }

    int leftHeight = 0, rightHeight = 0;
    if (!_IsBalance(root->_left, leftHeight))  // 检查左子树
        return false;
    if (!_IsBalance(root->_right, rightHeight)) // 检查右子树
        return false;

    // 计算当前节点平衡因子
    root->_bf = rightHeight - leftHeight;

    // 检查平衡因子是否符合AVL的要求
    if (abs(root->_bf) >= 2)
    {
        cout << root->_kv.first << "不平衡" << endl;
        return false;
    }

    // 计算当前节点的高度
    height = max(leftHeight, rightHeight) + 1;
    return true;
}

bool IsBalance()
{
    int height = 0;
    return _IsBalance(_root, height);
}
```
![](../../_Pasted_Image/Pasted%20image%2020260901145316.png)
## AVL树的删除（了解）
因为AVL树也是二叉搜索树，可按照二叉搜索树的方式将节点删除，然后再更新平衡因子，只不
错与删除不同的时，删除节点后的平衡因子更新，最差情况下一直要调整到根节点的位置。
具体实现学生们可参考《算法导论》或《数据结构-用面向对象方法与C++描述》殷人昆版。

## AVL树的性能

AVL 树是一种高度平衡的二叉搜索树，它在**查询**方面表现非常出色，但在**插入**和**删除**等更新操作上可能会带来额外的开销。下面从几个方面来分析它的性能和适用场景：
1. 查询性能

- **绝对平衡**：AVL 树要求每个节点左右子树的高度差（平衡因子 `_bf`）的绝对值不超过 1，这使得树在最坏情况下也能保持近似平衡。
- **时间复杂度**：由于树的高度始终保持在 `O(log N)`，AVL 树的查找操作可以在 `O(log N)` 时间内完成。对于需要频繁查询的场景，AVL 树可以提供较高的查询效率。

2. 插入和删除的开销
- **插入操作**：
	1. 在二叉搜索树的基础上进行插入，插入后需要从插入位置向上更新平衡因子。
	2. 如果某个节点出现了平衡因子 ±2，就需要做旋转（可能是单旋或双旋）来恢复平衡。
	3. 由于 AVL 树追求“每个节点高度差 ≤ 1”，在插入时可能需要做**多次旋转**，从而维持树的绝对平衡。
- **删除操作**：
	1. 删除节点后也需要从删除位置向上更新平衡因子。
	2. 更糟糕的是，在删除节点的场景下，可能会持续地出现高度不平衡，一直向上回溯到根节点，导致需要多次旋转才能恢复平衡。
	3. 因此，删除操作在最坏情况下会产生较大的调整开销。
3. 适用场景

- **静态或查询为主的场景**：如果数据基本不变（或很少修改），而对查询速度有较高要求，AVL 树非常适合，因为它能在 `O(log N)` 时间内进行快速查找，并且不会因为更新而经常触发旋转。
- **频繁更新的场景**：如果插入和删除操作非常频繁，AVL 树的维护成本较高，旋转次数多且可能反复向上回溯更新平衡因子。此时，可能需要考虑其他数据结构（如红黑树）来在更新和查询之间做平衡。

## 完整代码
```cpp
#pragma once
#include<assert.h>
#include<vector>
#include<iostream>
using namespace std;


template<class K,class V>//用来声明这个结构体是一个模板类
struct AVLTreeNode
{
	AVLTreeNode<K, V>* _left;
	AVLTreeNode<K, V>* _right;
	AVLTreeNode<K, V>* _parent;
	int _bf;//平衡因子
	pair<K, V> _kv; 

	AVLTreeNode(const pair<K, V>& kv)
		:_left(nullptr)
		, _right(nullptr)
		, _parent(nullptr)
		,_bf(0)
		,_kv(kv)
	{}
};

template<class K, class V>
//这里模板的作用是应用于整个 AVLTree 类，而不仅仅是 AVLTreeNode 结构体。因此，模板的声明需要在类定义前再次出现：
class AVLTree
{
	typedef AVLTreeNode<K, V> Node;
public:
	bool Insert(const pair<K, V>& kv) {
		// 1. 二叉搜索树插入阶段
		if (_root == nullptr) {            // 空树直接创建根节点
			_root = new Node(kv);
			return true;
		}

		Node* parent = nullptr;            // 跟踪插入位置的父节点
		Node* cur = _root;
		// 查找插入位置（遵循二叉搜索树规则）
		while (cur) {
			if (cur->_kv.first > kv.first) {  // 目标键小于当前节点，向左子树搜索
				parent = cur;
				cur = cur->_left;
			}
			else if (cur->_kv.first < kv.first) { // 目标键大于当前节点，向右子树搜索
				parent = cur;
				cur = cur->_right;
			}
			else {                        // 键值已存在，插入失败
				return false;
			}
		}

		// 创建新节点并插入到正确位置
		cur = new Node(kv);                // 新节点初始化（bf=0，parent=nullptr）
		if (parent->_kv.first < kv.first) { // 插入到父节点的右子树
			parent->_right = cur;
		}
		else {                            // 插入到父节点的左子树
			parent->_left = cur;
		}
		cur->_parent = parent;             // 设置新节点的父指针

		// 2. 平衡因子调整及旋转处理阶段
		while (parent) {                   // 从插入点向上回溯调整平衡因子
			// 更新父节点平衡因子
			if (cur == parent->_left) {    // 插入在左子树，父节点bf减1
				parent->_bf--;
			}
			else {                       // 插入在右子树，父节点bf加1
				parent->_bf++;
			}

			// 根据平衡因子决定后续操作
			if (parent->_bf == 0) {        // 子树高度未变化，无需调整
				break;
			}
			else if (parent->_bf == 1 || parent->_bf == -1) {
				// 子树高度变化，向上继续调整祖先节点
				cur = parent;
				parent = parent->_parent;
			}
			else if (parent->_bf == 2 || parent->_bf == -2) {
				// 平衡因子绝对值超过1，需旋转调整
				if (parent->_bf == 2 && cur->_bf == 1) {       // 右子树过深，左单旋
					RotateL(parent);
				}
				else if (parent->_bf == -2 && cur->_bf == -1) { // 左子树过深，右单旋
					RotateR(parent);
				}
				else if (parent->_bf == -2 && cur->_bf == 1) {  // 左子树的右子树过深，先左旋后右旋（LR型）
					RotateLR(parent);
				}
				else {                                        // 右子树的左子树过深，先右旋后左旋（RL型）
					RotateRL(parent);
				}
				break;  // 旋转后子树高度恢复，无需继续向上调整
			}
			else {
				// 平衡因子异常（绝对值≥3），说明插入前树已不平衡
				assert(false);  // 触发断言，调试时快速定位问题
			}
		}
		return true;
	}


	void RotateR(Node* parent)
	{
		Node* subL = parent->_left;
		Node* subLR = subL->_right;
		// `subL` 是 `parent` 的左子节点，即旋转后的新根
		// `subLR` 是 `subL` 的右子节点，在旋转后将成为 `parent` 的左子节点

		// 1. 将 `parent` 的左子树更新为 `subL` 的右子树
		parent->_left = subLR;

		// 2. 更新 `subLR` 的父节点为 `parent`
		if (subLR)
			subLR->_parent = parent;

		// 3. 将 `subL` 的右子树更新为 `parent`
		subL->_right = parent;

		// 4. 更新 `parent` 的父节点指向 `subL`
		Node* ppnode = parent->_parent;
		parent->_parent = subL;

		// 5. 如果 `parent` 是根节点，更新 `_root`
		if (parent == _root)
		{
			_root = subL;
			subL->_parent = nullptr;  // `subL` 成为新的根节点
		}
		else
		{
			// 如果 `parent` 不是根节点，则需要调整父节点指向
			if (ppnode->_left == parent)
			{
				ppnode->_left = subL;
			}
			else
			{
				ppnode->_right = subL;
			}
			subL->_parent = ppnode;
		}

		// 6. 更新平衡因子，旋转后 `parent` 和 `subL` 平衡因子都恢复为 0
		subL->_bf = 0;
		parent->_bf = 0;
	}



	void RotateL(Node* parent)
	{
		Node* subR = parent->_right;
		Node* subRL = subR->_left;
		// `subR` 是 `parent` 的右子节点，旋转后的新根
		// `subRL` 是 `subR` 的左子节点，在旋转后将成为 `parent` 的右子节点

		// 1. 将 `parent` 的右子树更新为 `subR` 的左子树
		parent->_right = subRL;

		// 2. 更新 `subRL` 的父节点为 `parent`
		if (subRL)
			subRL->_parent = parent;

		// 3. 将 `subR` 的左子树更新为 `parent`
		subR->_left = parent;

		// 4. 更新 `parent` 的父节点指向 `subR`
		Node* ppnode = parent->_parent;
		parent->_parent = subR;

		// 5. 如果 `parent` 是根节点，更新 `_root`
		if (parent == _root)
		{
			_root = subR;
			subR->_parent = nullptr;  // `subR` 成为新的根节点
		}
		else
		{
			// 如果 `parent` 不是根节点，则需要调整父节点指向
			if (ppnode->_left == parent)
			{
				ppnode->_left = subR;
			}
			else
			{
				ppnode->_right = subR;
			}
			subR->_parent = ppnode;
		}

		// 6. 更新平衡因子，旋转后 `parent` 和 `subR` 平衡因子都恢复为 0
		parent->_bf = 0;
		subR->_bf = 0;
	}






//ChatGPT的

	//左-右旋转包括两步：

	//首先对不平衡节点的左子节点进行左旋。
		//然后对不平衡节点本身进行右旋。
	void RotateLR1(Node* parent)
	{
		Node* subL = parent->_left;       // `subL` 是 `parent` 的左子节点
		Node* subLR = subL->_right;       // `subLR` 是 `subL` 的右子节点，也是旋转的核心

		// 1. 对 `subL` 执行左旋
		parent->_left = subLR;            // 将 `parent` 的左子节点更新为 `subLR`
		subL->_right = subLR->_left;      // `subL` 的右子节点更新为 `subLR` 的左子节点
		if (subLR->_left)
			subLR->_left->_parent = subL; // 更新 `subLR` 左子节点的父指针
		subLR->_left = subL;              // `subLR` 的左子树设为 `subL`
		subL->_parent = subLR;            // 更新 `subL` 的父节点为 `subLR`

		// 2. 对 `parent` 执行右旋
		Node* ppnode = parent->_parent;   // 保存 `parent` 的父节点信息
		parent->_left = subLR->_right;    // 更新 `parent` 的左子节点为 `subLR` 的右子树
		if (subLR->_right)
			subLR->_right->_parent = parent;
		subLR->_right = parent;           // `subLR` 的右子节点更新为 `parent`
		parent->_parent = subLR;          // 更新 `parent` 的父指针

		// 3. 处理 `ppnode` 的子树关系
		if (ppnode == nullptr)            // `parent` 是根节点的情况
		{
			_root = subLR;
			subLR->_parent = nullptr;
		}
		else
		{
			if (ppnode->_left == parent)   // 判断 `parent` 是 `ppnode` 的左子节点还是右子节点
				ppnode->_left = subLR;
			else
				ppnode->_right = subLR;
			subLR->_parent = ppnode;
		}

		// 4. 更新平衡因子
		subL->_bf = 0;
		parent->_bf = 0;
		subLR->_bf = 0;                    // 在双旋后，平衡因子通常重置为0
	}



	void RotateRL1(Node* parent)
	{
		Node* subR = parent->_right;      // `subR` 是 `parent` 的右子节点
		Node* subRL = subR->_left;        // `subRL` 是 `subR` 的左子节点

		// 1. 对 `subR` 执行右旋
		parent->_right = subRL;           // 将 `parent` 的右子节点更新为 `subRL`
		subR->_left = subRL->_right;      // `subR` 的左子节点更新为 `subRL` 的右子节点
		if (subRL->_right)
			subRL->_right->_parent = subR;
		subRL->_right = subR;             // `subRL` 的右子树设为 `subR`
		subR->_parent = subRL;            // 更新 `subR` 的父节点为 `subRL`

		// 2. 对 `parent` 执行左旋
		Node* ppnode = parent->_parent;   // 保存 `parent` 的父节点信息
		parent->_right = subRL->_left;    // 更新 `parent` 的右子节点为 `subRL` 的左子树
		if (subRL->_left)
			subRL->_left->_parent = parent;
		subRL->_left = parent;            // `subRL` 的左子节点更新为 `parent`
		parent->_parent = subRL;          // 更新 `parent` 的父指针

		// 3. 处理 `ppnode` 的子树关系
		if (ppnode == nullptr)            // `parent` 是根节点的情况
		{
			_root = subRL;
			subRL->_parent = nullptr;
		}
		else
		{
			if (ppnode->_left == parent)  // 判断 `parent` 是 `ppnode` 的左子节点还是右子节点
				ppnode->_left = subRL;
			else
				ppnode->_right = subRL;
			subRL->_parent = ppnode;
		}

		// 4. 更新平衡因子
		subR->_bf = 0;
		parent->_bf = 0;
		subRL->_bf = 0;                    // 双旋后平衡因子通常重置为0
	}


	void  RotateLR(Node* parent)
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

	void RotateRL(Node* parent)
	{
		Node* subR = parent->_right;
		Node* subRL = subR->_left;
		int bf = subRL->_bf;
		RotateR(subR);
		RotateL(parent);

		subRL->_bf = 0;//调节后是根 左右平衡
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

	void InOrder()
	{
		//return _InOrder(root);
		//_InOrder 是一个 void 类型函数，即它没有返回值。因此调用 _InOrder(root); 后，它不返回任何内容
		//用return是多余的
		_InOrder(_root);
	}

	void _InOrder(Node* root)
	{
		if (root == nullptr)
			return;
		_InOrder(root->_left);
		cout << root->_kv.first << "[" << root->_bf << "]" << endl;
		_InOrder(root->_right);
	}
	

	int _Height(Node* root)
	{
		if (root == nullptr)
			return 0;
		int leftHeight = _Height(root->_left);
		int rightHeight = _Height(root->_right);
		// 返回左右子树中的最大高度，并加上当前节点的高度（1）
		return leftHeight > rightHeight ? leftHeight + 1 : rightHeight + 1;
	}
	int Height()
	{
		return _Height(_root);
	}

	//低效写法
	bool _IsBalance1(Node* root)
	{
		if (root == nullptr)
			return true;
		int leftheight = Height(root->_left);
		int rightheight = Height(root->_right);
		if (abs(rightheight - leftheight) >= 2)
		{
			cout << root->kv.first << "不平衡" << endl;
			return false;
		}
		if (rightheight - leftheight != root->_bf)
		{
			cout << root->_kv.first << "平衡因子异常" << endl;
			return false;
		}
		return _IsBalance1(root->_left) 
			&& _IsBalance1(root->_right);
	}
	bool IsBalance1()
	{
		int height = 0;
		return _IsBalance1(_root);
	}


	bool _IsBalance(Node* root, int& height)
	{
		// 空节点视为平衡，高度为 0
		if (root == nullptr)
		{
			height = 0;
			return true;
		}

		int leftHeight = 0, rightHeight = 0;

		// 检查左子树是否平衡，同时获得左子树的高度
		if (!_IsBalance(root->_left, leftHeight))
			return false;

		// 检查右子树是否平衡，同时获得右子树的高度
		if (!_IsBalance(root->_right, rightHeight))
			return false;

		// 计算当前节点的平衡因子
		root->_bf = rightHeight - leftHeight;

		// 检查当前节点是否平衡
		if (abs(root->_bf) >= 2)
		{
			cout << root->_kv.first << "不平衡" << endl;
			return false;
		}

		// 计算当前节点的高度
		height = max(leftHeight, rightHeight) + 1;
		return true;
	}
	bool IsBalance()
	{
		int height = 0;
		return _IsBalance(_root, height);
	}

	size_t Size()
	{
		return _Size(_root);
	}
	size_t _Size(Node* root)
	{
		if (root == NULL)
			return 0;
		return _Size(root->_left) + _Size(root->_right) + 1;
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
		return NULL;
	}

private:
	Node* _root = nullptr;
};
void TestAVLTree1()
{
	int a[] = { 4, 2, 6, 1, 3, 5, 15, 7, 16, 14 };
	//int a[] = { 16, 3, 7, 11, 9, 26, 18, 14, 15 };
	AVLTree<int, int > t;
	for (auto e : a)
	{
		if (e == 14)
		{
			int x = 0;
		}
		t.Insert(make_pair(e, e));
		// 1、先看是插入谁导致出现的问题
		// 2、打条件断点，画出插入前的树
		// 3、单步跟踪，对比图一一分析细节原因
		cout << e << "->" << t.IsBalance() << endl;
	}
	t.InOrder();
	cout << t.IsBalance() << endl;
}
```
## 补充

### 为什么有两处 template<class K, class V> ？

在代码中，`template<class K, class V>` 出现了两次，是因为模板的定义和模板的使用分别位于两个不同的地方：
1. **模板结构体 **`AVLTreeNode`** 的定义**  
    在定义 `AVLTreeNode` 结构体时，`template<class K, class V>` 用来声明这个结构体是一个模板类。具体来说：

```cpp
template<class K, class V>
struct AVLTreeNode
{
    AVLTreeNode<K, V>* _left;
    AVLTreeNode<K, V>* _right;
    AVLTreeNode<K, V>* _parent;
    int _bf; //平衡因子
    pair<K, V> _kv;

    AVLTreeNode(const pair<K, V>& kv)
        : _left(nullptr)
        , _right(nullptr)
        , _parent(nullptr)
        , _bf(0)
        , _kv(kv)
    {}
};
```
这段代码定义了一个模板结构体 AVLTreeNode，它使用了模板类型 K 和 V。模板参数 K 和 V 是用于定义节点中的键值对  (pair<K, V>)，同时也用于定义指向左右子节点、父节点的指针类型（AVLTreeNode<K, V>*）。

2. **模板类 **`AVLTree`** 的定义**  
    这里模板的作用是应用于整个 `AVLTree` 类，而不仅仅是 `AVLTreeNode` 结构体。因此，模板的声明需要在类定义前再次出现：

```cpp
template<class K, class V>
class AVLTree
{
    typedef AVLTreeNode<K, V> Node;
public:
    // 这里可以使用 Node 类型
private:
    Node* _root = nullptr;
};
```
这段代码定义了一个模板类 AVLTree，它的模板参数同样是 K 和 V，表示这个 AVL 树可以存储任意类型的键值对。在类内部，你使用了 typedef 来定义 Node，即 AVLTreeNode<K, V>，这样你就可以在 AVLTree 类内部方便地使用 Node 来指代 AVLTreeNode<K, V> 类型了。

什么模板需要两次声明？
1. AVLTreeNode 是一个模板结构体，用于描述树的节点，它使用了模板类型 K 和 V 来表示键和值。
2. AVLTree 是一个模板类，用于表示整个 AVL 树，类内部使用了 AVLTreeNode<K, V> 来定义树的节点类型。因此，AVLTree 类本身也需要使用模板参数 K 和 V。

所以，模板 template<class K, class V> 必须在这两处都声明，分别告诉编译器 这两个类（结构体和类）是模板类，且使用了类型 K 和 V。编译器需要知道K 和 V 是在这两个地方使用的类型，以便正确地实例化模板。
### 调试小技巧
```cpp
void TestAVLTree1()
{
    //int a[] = { 8, 3, 1, 10, 6, 4, 7, 14, 13 };
    int a[] = { 4, 2, 6, 1, 3, 5, 15, 7, 16, 14 };
    AVLTree<int, int> t1;
    for (auto e : a)
    {
        /*if (e == 4)
		{
			int i = 0;
		}*/

        // 1、先看是插入谁导致出现的问题
        // 2、打条件断点，画出插入前的树
        // 3、单步跟踪，对比图一一分析细节原因
        t1.Insert({e,e});

        cout <<"Insert:"<< e<<"->"<< t1.IsBalance() << endl;
    }

    t1.InOrder();

    cout << t1.IsBalance() << endl;
}

```
**代码解释：**
```cpp
int a[] = { 4, 2, 6, 1, 3, 5, 15, 7, 16, 14 };
AVLTree<int, int> t;
for (auto e : a)
{
    t.Insert(make_pair(e, e));
    cout << e << "->" << t.IsBalance() << endl;
}

```
- 定义了一个数组 a[]，包含了多个整数，目的是依次将这些数字插入到 AVL 树中。
- 对于每一个元素 e，会调用 t.Insert(make_pair(e, e)) 将其插入到树中。
- t.IsBalance() 会检查当前树是否平衡，并输出检查结果。通常，AVL 树要求每个节点的左右子树高度差不超过 1，才能保持平衡。

1. **通过调试定位问题**
```cpp
if (e == 14)
{
    int x = 0;
}
```
- 这个条件 if (e == 14) 被用作调试标记，目的在于找到插入哪个节点时 AVL 树可能发生不平衡或出错。
- 当插入值为 14 时，程序会在 int x = 0; 这一行停下来。可以在这里设置一个断点，并通过调试工具单步执行，观察 AVL 树的结构变化，特别是插入 14 前后树的平衡状态。
- 通过这样的方式，可以帮助我们找到问题所在。例如，插入某个值后，树可能会失去平衡，需要进行旋转操作。

2. **打条件断点并分析树结构**
- 在插入过程中，通过 `t.IsBalance()` 可以观察每次插入后的树是否保持平衡。
- 在调试器中，打一个断点在 `int x = 0;`，运行程序时会暂停在该行，我们可以查看树的结构、节点高度等信息。通过树的变化图，我们能看到在 `14` 这个插入操作时，树是否失衡，以及失衡的具体情况（例如左子树或右子树的高度差过大）。
- 通过这种方法，能够精准地找出插入哪个元素导致了树的不平衡。

3. **最终检查平衡**
```cpp
t.InOrder();
cout << t.IsBalance() << endl;
```
- 在所有元素插入完成后，程序输出树的中序遍历（`InOrder()`）。可以查看最终的树结构，确认树是否有序，且平衡。
- `t.IsBalance()` 最后会输出整个树是否平衡，帮助我们确认最终的结果是否符合预期。