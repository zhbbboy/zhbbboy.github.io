---
title: RayTracing
date: 2026-09-09
tags: []
draft: false
---
笔记来自于[华为云](https://bbs.huaweicloud.com/blogs/351709) 
## 13 光线追踪基本概念

### 1.1 为什么需要光线追踪？

这里常用之对比的是光栅化。光栅化是把场景中的物体独立出来，并且再将物体独立成一个个的三角形面片进行渲染，通常 GPU 绘制三角形面片是并行的，所以对于物体来说，它是不知道自己周围的物体，同样的，对于三角形面片来说，它也是不知道自己周围的三角形。所以光栅化无法处理一类需要全局信息进行渲染的效果，如：`软阴影`、`Glossy 反射`以及`间接照明`：
![](../../_Pasted_Image/Pasted%20image%2020260909185940.png)

相比较光栅化，光线追踪是从相机出发，计算相机发出的光线（视线）与场景产生的结果，这里的光线可能会与场景中的物体碰撞后生成另外一些光线，然后生成的光线也需要继续做计算。所以对于光线追踪来说，相机产生的光线是知道整个场景信息的。

光栅化与光线追踪的对比：
1. 光栅化快，实时，是一种近似算法
2. 光纤追踪慢，离线，是一种准确的算法

### 1.2 光线
光线追踪需要光线的概念，课程以三个假设引入了光线的概念：
1. 光线沿着直线传播
2. 光线之间不会碰撞
3. 光线的可逆性。光线在场景中传播，产生反弹，假设反弹至相机，那么从相机视角来看，一定可以沿着传播的路径找到光源位置

### 1.3 光线投射（Ray Casting）
光线投射的过程如下：
1. 从相机位置，向物体投射一条光线用于成像
2. 从像素中对应物体的每个位置投射回光线，检查是否在阴影中（类似 Shadow Mapping）
![](../../_Pasted_Image/Pasted%20image%2020260909185946.png)

### 1.4 常规的着色过程
从视点发射一条光线，通过成像平面（某个像素）投射至物体，然后从物体发出光线至光源检查其是否在阴影中，如果不在阴影中，就可以通过某些着色方案对这个像素进行着色，如 Blinn Phong 等。  
![](../../_Pasted_Image/Pasted%20image%2020260909185951.png)

不同于常规的着色过程，下面介绍了一种递归的光线追踪方法`Recursive (Whitted-Style) Ray Tracing`。

## 二、Recursive (Whitted-Style) Ray Tracing

该方法有几点不同于常规的着色方案：

1. 光源：光线不同，这里假设光线在碰撞到物体后还会产生`折射`和`反射`。
2. 着色过程：每发生一次折射或者反射（弹射点）都计算一次着色，前提是该点不在阴影内。

如图所示，该算法的过程如下：

1. 从视点从成像平面发出光线，检测是否与物体碰撞
2. 碰撞后生成折射和反射部分
3. 递归计算生成的光线
4. 所有弹射点都与光源计算一次着色，前提是该弹射点能被光源看见
5. 将所有着色通过某种加权叠加起来，得到最终成像平面上的像素的颜色
![](../../_Pasted_Image/Pasted%20image%2020260909185958.png)

为了后续说明方便，课程定义了一些概念：

- 从视角出发第一次打到物体的光线叫做：`primary ray`
- 一次弹射之后的光线叫做：`secondary rays`
- 判断可见性的光线叫做：`shadow rays`

### 2.1 光线-表面相交

Recursive (Whitted-Style) Ray Tracing涉及到光线与物体的碰撞问题，所以这里介绍一下如何判断光线-表面相交问题。

#### 2.1.1 光线定义

光线可以一个点o和方向向量d定义：  
![](../../_Pasted_Image/Pasted%20image%2020260909190042.png)

![](../../_Pasted_Image/Pasted%20image%2020260909190007.png)

#### 2.1.2 光线与球求交(隐式表面求法)

已知光线方程：![](../../_Pasted_Image/Pasted%20image%2020260909190051.png)
和球的方程：![](../../_Pasted_Image/Pasted%20image%2020260909190116.png)

其中p表示球中某点， c表示球中心点， R表示球的半径。

那么直接将光线方程带入球的方程求解即可：  
![](../../_Pasted_Image/Pasted%20image%2020260909190147.png)
![](../../_Pasted_Image/Pasted%20image%2020260909190151.png)

对于所有隐式表示的曲面，都可以通过该方式求解（需要注意的是光线其实是射线，所以得到的解需要是实数且为正）：  
需满足：①是实数 ② 是正的  
![](../../_Pasted_Image/Pasted%20image%2020260909190204.png)

#### 2.1.3 光线与三角形求交（显示表面求法）
![](../../_Pasted_Image/Pasted%20image%2020260909190209.png)

判断是否在物体内：一个点在封闭图形内，在任意方向打一束光线出去

- 奇数个交点：内
- 偶数个交点：外  
    //推广到3D仍然适用

计算步骤：先求光线和平面的交点 → 再判断点是否在三角形内

光线与三角形相交最简单的便是将光线方程带入平面方程，求解：
![](../../_Pasted_Image/Pasted%20image%2020260909190216.png)

##### Moller Trumbore Alogrithm（MT算法）

一种快速计算光线是否在三角形内以及算法与平面相交的算法  
平面方程的一般式表达为：Ax+By+Cz+D=0Ax+By+Cz+D=0  
如果光线在三角形内，那么这个点一定可以用重心坐标进行表示，那么进行求解便可得到答案：

![](../../_Pasted_Image/Pasted%20image%2020260909190221.png)

具体步骤：  
光线上的点，用重心坐标表示  
→求解- t，b1，b2（三个式子，三个未知数——求解方法：克莱姆法则）  
→解出来之后，看是否合理：①沿着这个方向（t非负）②在三角形内（b1，b2非负）

### 2.2 光线-表面相交加速
加速原理：不再判断光线与每个三角形的相交情况，而是判断光线与包围物体表面的盒子来粗略判定光线是否与物体相交  
它等同于用更简单的模型来替代原有复杂的模型，代替原有模型求解与光线的相交

#### 2.2.1 Bounding Volumes(包围盒）
包围盒：一个物体可以用简单的形状包围起来，保证这个物体一定在这个简单的形状中。  
一个重要逻辑：如果光线连包围盒都碰不到，就更不可能碰到物体。

这里主要介绍了 AABB 包围盒（轴对称包围盒），它是轴对齐的。通常在判断物体相交情况的时候，可以先检测物体是否与 AABB 包围盒是否相交，如果相交再进一步做判断（更精细的相交判断）。  
![](../../_Pasted_Image/Pasted%20image%2020260909190229.png)

#### 2.2.2 如何判断光线与 AABB 包围盒相交？

这里给了一个 2D 的例子进行解释：  
![](../../_Pasted_Image/Pasted%20image%2020260909190233.png)

需要记住的关键概念是：
1. 光进入了所有对面，才算进入Box
2. 光离开任意一对面，就算离开Box
3. 什么时候有交点：t进入<t离开时有交点
4. 考虑负的情况：t离开<0 说明 Box在光背后
5. t离开≥0 ＆ t进入< 0 说明光线起点在Box内 → 有交点
6. 对于三维包围盒来说，有如下公式成立：![](../../_Pasted_Image/Pasted%20image%2020260909153441.png)

**总之：当且仅当 t进入 < t离开 ＆＆ t离开≥0 时，有交点**

这里还需要注意一些问题：为什么选包围盒比较好？
因为包围盒本质上是可以分为垂直于 x、y、z 的分量的，所以求解光线与包围盒的相交也可以分解成光线中某个轴的分量与包围盒某个轴的分量的相交：  
![](../../_Pasted_Image/Pasted%20image%2020260909190240.png)


## 14光线追踪的加速

上一节课说了计算光线-物体的求交通常可以通过计算光线与物体表面包围盒的求交来进行加速。这只是从物体出发的一个角度，另外一种方式是通过对场景进行划分来达到加速的效果。常用的对场景进行划分的方法有 `Uniform grids` 和 `Spatial partitions`。

### 1.1 Uniform grids

`Uniform grids` 简单来说就是将场景划分成一个个规整的格子，通过格子来判断光线与物体的相交情况。

构建Uniform grids 的步骤如下：
1. 找到包围盒
2. 创建格子
3. 存储每个对象至格子中  
    那么此时判定光线与物体相交就转换成光线与格子相交问题，当光线与格子相交后，再去考虑它与格子内物体的相交情况：
![](../../_Pasted_Image/Pasted%20image%2020260909190305.png)
这种方式有一些问题：
1. 格子的分辨率问题，如果格子的分辨率设置太小就失去了划分的意义，如果分辨率太大也不好。
2. 格子通常只适用于规整的场景，相反，物体位置较为稀疏的场景就不适用这种划分方案，因为此时很多格子都不存储物体，大量时间被浪费在了光线与格子的求交上：

### 1.2 Spatial partitions 空间划分

Spatial partitions 通常有 Oct-Tree（八叉树） 、KD-Tree 以及 BSP-Tree 三种划分方案：
![](../../_Pasted_Image/Pasted%20image%2020260909190317.png)

下面将在二维对三种方案进行讨论：

Oct-Tree：每次迭代都将区域重新切分为均匀四块，按一定规则停止切分（如切分得到的四块区域中，三块都没有物体；或四块区域都还有物体，但是此时的区域已经较小）

KD-Tree：总是沿某个轴进行切分，每次划分总会在原来的区域上生成两块新的区域（这里沿轴的次序是由交替进行的，如二维中总是按着 x/y 或 y/x 的次序进行交替切分）

BSP-Tree：每次都是沿着一定方向进行切分（非水平或竖直）

### 1.3 KD-Tree 预处理※※※※※※※

KD-Tree 每次划分都会生成新的两块区域，然后再在两块区域内进行划分，直至满足递归截至条件。

下图显示了一个[三维k-d树](https://zh.wikipedia.org/wiki/K-d%E6%A0%91)。第一次划分（红色）把根节点（白色）划分成两个节点，然后它们分别再次被划分（绿色）为两个子节点。最后这四个子节点的每一个都被划分（蓝色）为两个子节点：
![](../../_Pasted_Image/Pasted%20image%2020260909190329.png)

假设空间 A 被划分为如图所示 KD-Tree，那么它的结构等同于其中树结构所所描述：  
![](../../_Pasted_Image/Pasted%20image%2020260909190337.png)

#### 1.3.1 数据结构

中间节点（A，B，C，D）存储了：
1. 划分后的轴
2. 划分后的切分平面
3. 子节点的指针，叶子节点（1，2，3，4，5）存储了：区域中包含的物体的列表（obj只会存在于叶子节点）

#### 1.3.2 遍历

1. 发射光线从根节点出发，分别判断光线与左右节点是否相交，若相交则进入2；否则，则与节点不相交
2. 递归判断相交直至叶子节点，若与叶子节点相交，进入3
3. 挨个判断叶子节点存储物体与光线的相交情况

下列图分别显示了光线遍历至 B 节点的情况：

光线穿过A节点：  
![](../../_Pasted_Image/Pasted%20image%2020260909190342.png)

假设这里1号区域为叶子节点，因此要遍历A中的所有物体判断是否与光线相交  
![](../../_Pasted_Image/Pasted%20image%2020260909190346.png)

光线穿过B节点：  
![](../../_Pasted_Image/Pasted%20image%2020260909190353.png)

假设这里2号区域为叶子节点，因此要遍历2中的所有物体判断是否与光线相交  
![](../../_Pasted_Image/Pasted%20image%2020260909190356.png)

光线穿过C节点：  
![](../../_Pasted_Image/Pasted%20image%2020260909190401.png)

假设这里3号区域为叶子节点，因此要遍历3中的所有物体判断是否与光线相交  
![](../../_Pasted_Image/Pasted%20image%2020260909190405.png)

#### 1.3.3 KD-Tree 存在问题

1. KD-Tree 建立过程中需要考虑三角形与包围盒的求交问题
2. 物体可能与多个包围盒相交

![](../../_Pasted_Image/Pasted%20image%2020260909190411.png)

### 1.4 Object Partitions（基于物体的划分） & Bounding Volume Hierarchy (BVH)

为了解决上述问题，人们提出了另外一种划分方案，即在场景中对**物体**进行划分，此时就不用考虑三角形与包围盒的求交问题

它本质是将一个场景用一个包围盒包住，然后按照一定划分方案将盒子划分成不同的子区域，不同子区域都需要包含三角形，最终划分到叶子节点时，每个叶子节点就包含了一些三角形，即包含了对应的一些物体：

![](../../_Pasted_Image/Pasted%20image%2020260909190416.png)

#### 1.4.1 划分流程

下面的图片参考链接: https://www.haroldserrano.com/blog/visualizing-the-boundary-volume-hierarchy-collision-algorithm

1. 创建一个根节点
2. 创建一个AABB box 将所有物体包围在该场景中
3. 设置AABB box为根节点  
![](../../_Pasted_Image/Pasted%20image%2020260909190422.png)
4. 找到AABB 最长轴，并且沿着该方向将所有物体排序  
![](../../_Pasted_Image/Pasted%20image%2020260909190426.png)

5. 找到一个中点(基于某个轴）切分bbox  
![](../../_Pasted_Image/Pasted%20image%2020260909190435.png)

6. 使用分割索引，将场景分成左右两侧。
7. 对于每一侧，创建一个包含其各自对象的 AABB box。
8. 在二叉树中创建一个左右节点，并附加其对应的AABB box。  
![](../../_Pasted_Image/Pasted%20image%2020260909190442.png)

9. 在每个节点上重复4-8步 直到每个节点最多只包含两个物体。  
eg: 左边重复4-6步得到的结果：  
![](../../_Pasted_Image/Pasted%20image%2020260909190447.png)


eg:左边重复6-8步得到的结果：  
![](../../_Pasted_Image/Pasted%20image%2020260909190451.png)

整体得到的最终结果：  
![](../../_Pasted_Image/Pasted%20image%2020260909190456.png)
#### 1.4.2 如何划分节点
1. 按轴的次序进行划分
2. 按最长轴继续进行划分
3. 在物体三角形数目的最中间对物体进行划分（这里的最中间指的是划分后，两边的三角形数目基本一致）

#### 1.4.2 数据结构
- 中间节点（Internal nodes）：该节点对应的包围盒和子节点的指针
- 叶子节点（Leaf nodes）：该节点对应的包围盒和包围盒里面的物体列表

#### 1.4.3 BVH遍历流程
遍历过程就是递归的，类似于遍历一颗二叉树：  
![](../../_Pasted_Image/Pasted%20image%2020260909190501.png)

#### 1.5 Spatial vs Object Partitions   KD-tree与BVH的区别
![](../../_Pasted_Image/Pasted%20image%2020260909190508.png)

## 二、辐射度量学

### 2.1 为什么需要辐射度量学？
因为需要对物理世界的光做更加真实的描述，而辐射度量学是具有物理意义的，它相较于 blinn phong 等光照模型，能够更加精确、真实地描述光照 （在图形学中）

为了弄清出辐射度量学如何对光照进行更加精确的描述，这里需要辐射度量学的一些相关概念来帮助理解：Radiant flux, intensity, irradiance, radiance，它们的单位表示为下图：  
维基百科：[https://zh.wikipedia.org/wiki/辐射度量学](https://zh.wikipedia.org/wiki/%E8%BE%90%E5%B0%84%E5%BA%A6%E9%87%8F%E5%AD%A6)  
![](../../_Pasted_Image/Pasted%20image%2020260909190513.png)
### 2.2 一些基本概念理解

#### 2.2.1 Radiant Energy and Flux (Power)

`Radiant Energy`是电磁辐射的能量，用焦耳来表示其单位（它在图形学中较少使用）：![](../../_Pasted_Image/Pasted%20image%2020260909174908.png)

`Radiant Flux`是单位时间的能量（功率），它简单描述了一个发光体在单位时间所发出的能量/接收的能量：  
![](../../_Pasted_Image/Pasted%20image%2020260909174922.png)

在光学中经常以流明为单位，说明光的亮度,与功率Flux是一回事。  
![](../../_Pasted_Image/Pasted%20image%2020260909174932.png)

光在物理世界中涉及到能量的部分，通常可以用三个过程来进行概括：**发射**，**接收**以及**传播**，对应的是 光源发出的光，表面接收的光，光在传播过程中的光 三个概念：  
![](../../_Pasted_Image/Pasted%20image%2020260909190522.png)
#### 2.2.2 Radiant Intensity
由于光源是往不同方向发射光的，所以我们需要一种方法来描述在不同方向上，光源在单位时间发射的能量（即不同方向光源产生的功率），这里采用了`Radiant Intensity`来进行描述，如图所示（假设ω为球面上的某个方向向量）：  
![](../../_Pasted_Image/Pasted%20image%2020260909190529.png)

那么如何描述方向？这里采用 Angles and Solid Angles（ 角度和立体角）的来进行描述

##### Angles and Solid Angles
1. Angles（角度）  
    通常角度 `Angles` 可以表示为弧度制，也就是用 `弧长/ 半径` 来进行表示：
- θ = l/ r
- 圆有2π的 radians

![](../../_Pasted_Image/Pasted%20image%2020260909190538.png)

2. Solid Angles（立体角是角度在3维空间上的延申）  
![](../../_Pasted_Image/Pasted%20image%2020260909190541.png)

立体角表示的是球面上的**面积与半径平方之比**：![](../../_Pasted_Image/Pasted%20image%2020260909175933.png)

球总共有4π的 steradians
定义球面上的方向向量为ω：  
![](../../_Pasted_Image/Pasted%20image%2020260909190545.png)

那么求解单位立体角，我们只需要得到单位立体角对应的球面面积，然后用单位面积除以半径的平方即可，求解如下：  
![](../../_Pasted_Image/Pasted%20image%2020260909175010.png)

当定义了单位立体角后，就可以通过单位辐射通量来描述光源在某个方向上发出的能量。即单位方向内，光源在**单位时间发出的能量**：  
![](../../_Pasted_Image/Pasted%20image%2020260909175020.png)​
换言之，已知辐射通量，我们就可以求出在整个球面上光源的 Radiant Intensity：  

![](../../_Pasted_Image/Pasted%20image%2020260909175015.png)

这里课程给了一个例子，假设一个灯泡的 Radiant flux (power) 为 815 lumens，那么它在整个球面所辐射的强度 Radiant Intensity 为：  
Intensity = 815 lumens / 4pi sr = 65 candelas  
（这里的 lumens 也是 Radiant flux (power) 的单位）
![](../../_Pasted_Image/Pasted%20image%2020260909180559.png)




## 15 辐射度量学

上节课的回顾：

- `Radiant flux (power)` 描述了单位时间内的能量
- `Radiant intensity` 描述了光源在单位立体角，单位时间上发出的辐射能量（Radiant flux (power) ）
- `Solid Angle` 描述了球面面积与半径的平方之比

![](../../_Pasted_Image/Pasted%20image%2020260909190608.png)

既然定义了点光源发出的辐射功率，接下来就继续定义物体表面是如何接收辐射功率的，它是通过 irradiance 来进行描述的。

### 1.1 irradiance

irradiance 与 Radiant intensity 不同，它描述的是接收到的功率：即**单位区域面积、单位时间**内接收到的辐射能量：  
![](../../_Pasted_Image/Pasted%20image%2020260909191345.png)

![](../../_Pasted_Image/Pasted%20image%2020260909190612.png)

这里的面积指的是与光线所垂直的面积，即如果表面与光线存在夹角，需要对其进行投影,即表面的Irradiance和法线的余弦成正比：  
![](../../_Pasted_Image/Pasted%20image%2020260909190616.png)

相较于 Radiant intensity ，irradiance 在传播过程中是在衰减的，因为 Radiant intensity 只和角度有关。但是当光源离物体表面越远，角度是不变的，但辐射到的面积却是增大的，所以 irradiance 是逐渐衰减的。  
![](../../_Pasted_Image/Pasted%20image%2020260909190620.png)

### 1.2 radiance

`radiance` 是指**单位立体角、单位投影面积**所辐射出的能量。  
为什么需要 radiance？

1. 用于描述光线的各种属性  
![](../../_Pasted_Image/Pasted%20image%2020260909190626.png)
    
2. 是因为需要和 irradiance 进行联系，以描述场景中物体接收到的能量以及它向周围所辐射出去的能量。

radiance 的定义如下：per unit solidangle, per projected unit area  
![](../../_Pasted_Image/Pasted%20image%2020260909190631.png)

理解：确定一定的区域（dA），它会往特定的一个方向辐射能量（欧米伽方向）

#### 1.2.1 比较/联系Radiance、Irradiance、Intensity

- 总结：  
    Radiance 等于 Irradiance per 立体角  
    等于 Intensity per unit 面积
从上面式子可以看出，通过积分顺序可以定义出不同类型的`radiance`。

#### 1.2.2 Incident Radiance
当先积分dA 时，radiance 定义为 `Incident Radiance` （和入射有关）。表示光源通过单位立体角**到达**物体表面的能量。  
![](../../_Pasted_Image/Pasted%20image%2020260910100218.png)

#### 1.2.3 Exiting Radiance

当先积分dωdω 时， radiance 定义为 Exiting Radiance（和出射有关）。表示单位投影区域离开物体表面的能量:  
![](../../_Pasted_Image/Pasted%20image%2020260910100222.png)

#### 1.2.4 Irradiance VS Radiance

当定义了描述接收辐射能量的概念 irradiance ，和描述发出辐射能量的概念 radiance ，就可将两者联系起来（其实把 radiance 看出是发出辐射能量的概念并不贴切，它更像是描述了单位立体角、单位投影面积的辐射能量的密度），它们两者的联系如下：  
![](../../_Pasted_Image/Pasted%20image%2020260910100232.png)

从图中我们也可以看出 irradiance 实际上是描述了从所有潜在方向接收到的辐射能量。

简单来说：
- 区别：方向性
    - Irradiance：某个范围（dA）收到的power
    - Radiance：某个范围（dA）收到的一个方向的power
- 联系： Radiance →积分得→ Irradiance

### 1.3 BRDF(双向反射分布函数 Bidirectional Reflectance Distribution Function)

#### 1.3.1 试图用 `Radiance`→积分得→`Irradiance`来解释BRDF

(a)为什么有反射？  
理解1：光打到物体上，被弹走，改变方向  
理解2：光打到物体表面，被吸收，再被发出  
(b)用Irradiance和Radiance解释理解2  
![](../../_Pasted_Image/Pasted%20image%2020260910100242.png)

#### 1.3.2 BRDF的定义
前面定义了每单位表面如何接收能量的，以及该表面所反射能量的计算方式。那么当表面接收到能量后，它们又是如何将能量**从不同方向**反射出去的呢？这里就引入了 BRDF 来进行解决。由于不同方向反射的能量是不同的，所以反射出去的能量可以看成是一个分布，BRDF 解决的正是**从某个方向辐射到表面的能量，它所辐射到其他方向的能量分布问题**，它描述了反射方向上的能量分布 （这一切的前提都是在物体表面会把接收的能量辐射出去的前提下）

定义物体表面单位面积在**单位入射立体角ωi​、单位时间下通过光源入射所接收到的能量dEi(ωi)** ，假设它反射了能量，定义该单位表面沿某个单位出射立体角ωr反射的能量为dLr(ωr)，人们就将 `BRDF` 定义为它们两者的比值：
其实就是一个比例：对于任何一个出射方向，算出来他的Radiance，除以dA接受到的Irradiance，就是`BRDF`。

![](../../_Pasted_Image/Pasted%20image%2020260910100251.png)

### 1.4 反射方程

上述过程只是寻找某个单位入射立体角方向的反射能量分布，那么当所有方向都对物体表面辐射了能量，就需要将这些能量一并考虑进反射方程。所以对于某个单位表面来说，它沿某个出射立体角方向所反射的能量就是光沿所有方向辐射给该表面的能量，定义如下：  

![](../../_Pasted_Image/Pasted%20image%2020260910100254.png)

这也正是`反射方程`的定义。

由定义也可以看出，它可以理解为：在一个表面位置p和出射方向ωo​，不同入射方向ωi​的光源都对该位置辐射了能量，使得该表面会沿出射方向反射能量Li。而不同入射方向会使得表面产生不同的反射能量，这时就需要一个描述反射能量分布的函数fr(ωi→ωr) 来描述表面在不同方向的`反射能量分布（BRDF）`。求出所有反射能量后，将它们求和即得到沿该出射方向的总反射能量。

具体来说：从某个方向（camera）看某个着色点（dA）就是一个积分。  
这个积分考虑的就是任何一个不同的方向（dωi​），然后考虑它的Radiance（Li(p,ωi)cosθidωi），Radiance进来之后，经过BRDF（fr(p, ωi→ωr)）会变成出射的Radiance。  
![](../../_Pasted_Image/Pasted%20image%2020260910100259.png)

challenge：这个点来的Radiance，不只是有光源部分的，还有其它的（eg：其它点出射的Radiance）

### 1.4 渲染方程

渲染方程简单理解就是：物体自反光 + 物体反射光

上一节已经计算出了物体沿各个方向通过入射光计算得到的物体反射光，而物体自发光可以通过人为规定的材质来进行定义，所以渲染方程就可以定义如下：  
![](../../_Pasted_Image/Pasted%20image%2020260910100306.png)
注意：  
1.所有方向认为朝外  
2.认为下半球贡献为0（定义积分忽略下半球）  
3.n·ωi=cosθi

#### 1.4.1 渲染方程的由来

![](../../_Pasted_Image/Pasted%20image%2020260910100311.png)

1. 假设有多个点光源-> 加起来  
![](../../_Pasted_Image/Pasted%20image%2020260910100320.png)

2. 面光源怎么办？ -> 积分起来（点光源集合）  
![](../../_Pasted_Image/Pasted%20image%2020260910100334.png)

3. 有其它物体反射过来的Radiance -> 把其它来源也当成 光源处理  
![](../../_Pasted_Image/Pasted%20image%2020260910100340.png)
##### 把问题理解为递归方式

a. 数学简写  
![](../../_Pasted_Image/Pasted%20image%2020260910100345.png)

b. 再简写(为了解出L)  
![](../../_Pasted_Image/Pasted%20image%2020260910100351.png)

c. 怎么解  
数学变换：L可以写成如下:  
![](../../_Pasted_Image/Pasted%20image%2020260910100429.png)  
这里主要运用了算子同样具有类似泰勒展开的性质

d. 最后可以理解为能量分解->全局光照  
![](../../_Pasted_Image/Pasted%20image%2020260910100434.png)

- E：直接看到的光源（直接光照）
- kE：辐射出的能量经过1次反射，会看到（间接光照）
- k2E：经过2次反射  

### 1.5 全局光照概念
全局光照=直接光照+间接光照

#### 1.5.1 光栅化的局限
仅可以做：L=E+kE（只能做0次和1次）  
![](../../_Pasted_Image/Pasted%20image%2020260910100440.png)

这里也推出了Ray-Tracing的意义：可以做后续的弹射
一个直观的例子
- 直接光照（光栅化结果）  
![](../../_Pasted_Image/Pasted%20image%2020260910100451.png)
- 加入间接光照（弹2次，光追）  
![](../../_Pasted_Image/Pasted%20image%2020260910100500.png)

## 二、概率论基本知识

用于后续求解渲染方程
### 2.1 随机变量与概率密度函数 (PDF)

`随机变量`：以一定概率出现的变量值  
`概率密度函数`:在一定区间进行采样，用来描述随机变量的出现概率的分布函数  
![](../../_Pasted_Image/Pasted%20image%2020260910100507.png)

`概率`(probabilities)：  
![](../../_Pasted_Image/Pasted%20image%2020260910100512.png)

### 2.3 期望

离散的数学期望就是把所有随机变量与对应的概率的乘积求和，它反应的是随机变量平均出现的概率：  
![](../../_Pasted_Image/Pasted%20image%2020260910100517.png)

### 2.4 连续情况下描述变量和分布

pdf：概率密度函数要满足：  
a. 线下面积为1  
b. ∫x·概率密度=E（X）

![](../../_Pasted_Image/Pasted%20image%2020260910100524.png)

对本身是函数的随机变量求期望，就等于直接把该函数看成是输入：  
![](../../_Pasted_Image/Pasted%20image%2020260910100533.png)




## 16 Monte Carlo Integration蒙特卡洛积分

定积分本质上是求解曲线在区间内与坐标轴所围成的面积。Monte Carlo Integration 是一种近似求解积分的方法，它在所求定积分的函数中随机取样多次，分别对取样得到的 f(x) 值和定积分上下限的矩形求面积，然后将这些面积平均，以近似求解定积分。

1.Why Monte Carlo Integration  
为了解决定积分

2.What ＆How  
①蒙特卡洛积分：  
不同于黎曼积分（分为多个微小的部分），蒙特卡洛积分为：用一个随机的采样的方法。随意取一个点，对应f(x)为高，a→b为宽，（假设为长方形）来近似线下的面积，重复多次采样，最后平均起来长方形的面积。  
②定义：  
![](../../_Pasted_Image/Pasted%20image%2020260910104109.png)

当然，采样次数越多，该方法求解的定积分结果就越准确。

eg：如果在a→b均匀的采样，pdf=1/b-a  
![](../../_Pasted_Image/Pasted%20image%2020260910104104.png)

用蒙特卡洛来算  
![](../../_Pasted_Image/Pasted%20image%2020260910104101.png)

一个特殊情况：随机变量的采样是均匀的  
一个结论：任何一个积分的蒙特卡洛形式：  
![](../../_Pasted_Image/Pasted%20image%2020260910104053.png)

注意：定义在x域上，采样也要在x域上（定义和采样要在一个域）

## 二、路径追踪

为了解决 Whitted-Style Ray Tracing 中的一些问题，让渲染更加真实。

### 2.1 Whitted-style光线追踪的两个问题

#### 2.1.1 问题1：Whitted-Style Ray Tracing 在处理光照反射时总是沿着镜面反射方向进行，这适用于镜面材质，但却不适用于 Glossy 之类的材质

![](../../_Pasted_Image/Pasted%20image%2020260910104048.png)

#### 2.1.2 问题2：Whitted-Style Ray Tracing 不考虑物体表面漫反射会进一步反射，但实际上漫反射物体仍会继续反射光线

![](../../_Pasted_Image/Pasted%20image%2020260910104043.png)  
以及 color bleed现象。

### 2.2 渲染方程解决Whiteed-style问题

相较于这些问题，渲染方程的结果是对的，所以我们只需要按照渲染方程来求解物体表面的着色问题即可：  
![](../../_Pasted_Image/Pasted%20image%2020260910104039.png)

### 2.3 蒙特卡洛方法具体过程

先考虑简单情况（直接光照情况，且不考虑自发光项）  
![](../../_Pasted_Image/Pasted%20image%2020260910104034.png)  
![](../../_Pasted_Image/Pasted%20image%2020260910104030.png)

为了求解这个在球面上的积分，这里就需要 Monte Carlo 方法来近似求解：  
![](../../_Pasted_Image/Pasted%20image%2020260910104025.png)  
![](../../_Pasted_Image/Pasted%20image%2020260910104020.png)

进一步看如果来自某一物体的反射的间接光源时应如何处理：  
![](../../_Pasted_Image/Pasted%20image%2020260910104015.png)

![](../../_Pasted_Image/Pasted%20image%2020260910104011.png)

通过上述公式就可以简单描述出求解着色的步骤：
1. 随机采样 N 次入射方向
2. 根据采样的入射方向，针对能够反射到光源的部分，计算Li(p,ωi)fr(p,ωi,ωo)(n⋅ωi)Li​(p,ωi​)fr​(p,ωi​,ωo​)(n⋅ωi​)
3. 累加该次求解结果
4. 平均，完成着色

同时，还需要考虑反射如果打到物体表面，物体会吸收光源进行二次反射着色过程。

但是这里还存在几个问题：

#### 问题1：光线数量会爆炸
物体表面在进行反射的过程中，反射的光线数目应该为多少？  
![](../../_Pasted_Image/Pasted%20image%2020260910104006.png)

如果反射的光线数目为 N，反射 次数为 M，那么对于一个像素上的射线的计算量都有可能为：NM ，这个计算量是相当大且呈指数增长的。为了减少计算量，人们将反射的光线数目设为1，而在每个像素处打入多条射线来解决这个问题（即`路径追踪`的概念）：  
如下图所示 红色、黑色、蓝色都表示穿过同一像素的路径  
![](../../_Pasted_Image/Pasted%20image%2020260910104001.png)

这一步可以在一个循环里面做，即对于一个像素选取不同的位置打入射线以求解着色：  
![](../../_Pasted_Image/Pasted%20image%2020260910103957.png)

#### 问题2：递归无法停止问题

着色何时停止，即当光线反射多少次后，不再进行反射？这里采用了 `Russian Roulette (RR)` 来解决这个问题。

当满足概率为 P 时，返回除以该概率的着色结果：Lo/PLo​/P，当概率为 (1-P) 时，就停止反射光线，通过这种方式，我们仍然能够得到正确的着色结果。

![](../../_Pasted_Image/Pasted%20image%2020260910103952.png)

转换为伪代码就是：  
![](../../_Pasted_Image/Pasted%20image%2020260910103947.png)

此时已经是一个正确的路径追踪算法实现了，但效率不高。

![](../../_Pasted_Image/Pasted%20image%2020260910103942.png)

我们希望通过低采样率也能获得高采样率的效果。

#### 问题3：效率低下->不浪费算法推导

光线在反射的过程中打到光源是存在一定概率的，它往往与光源自身大小相关，如果光源过小，会使得大量光线无法打到光源，以致出现光线浪费现象。  
![](../../_Pasted_Image/Pasted%20image%2020260910103937.png)

原始蒙特卡洛方程是在着色点上进行积分的，这里需要考虑在光源上进行积分。  
![](../../_Pasted_Image/Pasted%20image%2020260910103931.png)

因此要找到光源出dA和立体角dw之间的关系  
![](../../_Pasted_Image/Pasted%20image%2020260910103924.png)

即先将光源投影到立体角方向dAcosθ，再计算其余x点的距离平方，即可得到立体角的关系。

由此，就可以重写改变积分阈的渲染方程：  
![](../../_Pasted_Image/Pasted%20image%2020260910103917.png)

这里的做法就是将物体表面接收到的 radiance 分解为两部分：来自光源和来自其他反射物体。

- 对于来自光源的部分，直接在光源上进行采样即可，采样过程就是换积分域的过程
- 对于来自其他反射的光源，仍然按照RR算法求解  
![](../../_Pasted_Image/Pasted%20image%2020260910103912.png)
伪代码如下：  
![](../../_Pasted_Image/Pasted%20image%2020260910103902.png)

#### 问题4：判断半球到光源之间有没有被挡:

解决方法：点到光源取一条连线，打一条光线，看中间有没有打到其他的物体  
![](../../_Pasted_Image/Pasted%20image%2020260910103856.png)

一些注解：  
①Path Tracing很难实现
②还是入门吗？ 算进阶  
③Path Tracing 几乎100%正确  
![](../../_Pasted_Image/Pasted%20image%2020260910103851.png)

## 三、Ray Tracing：过去的光追VS现在的概念

1. 以前：基本都是指Whitted-style 的光线追踪
2. Now（老师自己的定义）：所有光线传播方法的大集合
    - (Unidirectional & bidirectional) path tracing
    - Photon mapping
    - Metropolis light transport
    - VCM / UPBP…

## 四、我们没有说的话题

1. Uniformly sampling the hemisphere  
    怎么均匀采样？怎么采样？
2. Monte Carlo integration allows arbitrary pdfs， What’s the best choice?  
    importance sampling 重要性采样理论
3. Do random numbers matter? 随机数重要吗？  
    Yes! (low discrepancy sequences)比如蓝噪音
4. I can sample the hemisphere and the light， Can I combine them?  
    Yes! (multiple imp. sampling) 可以结合半球和光源采样吗？
5. The radiance of a pixel is the average of radiance on all paths passing through it  
    不同Path的Radiance平均起来，为什么平均起来就是pixel的Radiance？  
    Why? (pixel reconstruction filter)
6. Is the radiance of a pixel the color of a pixel? Pixel的Radiance就是它的颜色吗？  
    No. (gamma correction（radiance到color的对应关系）, curves(HDR), color space)