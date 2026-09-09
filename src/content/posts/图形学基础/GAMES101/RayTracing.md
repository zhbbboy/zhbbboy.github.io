---
title: RayTracing
date: 2026-09-09
tags: []
draft: false
---
笔记来自于[华为云](https://bbs.huaweicloud.com/blogs/351709) 
## 一、光线追踪基本概念

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




## 一、辐射度量学

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
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153233.35214883317772659091131089022850:20260909124125:2415:02F21170977C075023428BC70E8A7C23C78AD7DD581F72B72FAA092B70EAC7C5.png)

#### 1.2.3 Exiting Radiance

当先积分dωdω 时， radiance 定义为 Exiting Radiance（和出射有关）。表示单位投影区域离开物体表面的能量:  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153233.09180826888331647352485869585213:20260909124125:2415:EAC86C9C388EE6BAC236A0F695B18D8DAADDCEE552A99A04D47633BCC2B241DC.png)

#### 1.2.4 Irradiance VS Radiance

当定义了描述接收辐射能量的概念 irradiance ，和描述发出辐射能量的概念 radiance ，就可将两者联系起来（其实把 radiance 看出是发出辐射能量的概念并不贴切，它更像是描述了单位立体角、单位投影面积的辐射能量的密度），它们两者的联系如下：  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153234.73534969963043684531450972216453:20260909124125:2415:B1B151E282566FD22C82FE8CEC70D4446F27AA28C08090D28838700485D3EFEC.png)

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
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153234.35193574694974108691877906383357:20260909124125:2415:3900399688DC8AFA7BC9CF998AA73EC09FD5F3B85E119BFD0B8CDF1F3BEDB8DD.png)

#### 1.3.2 BRDF的定义

前面定义了每单位表面如何接收能量的，以及该表面所反射能量的计算方式。那么当表面接收到能量后，它们又是如何将能量**从不同方向**反射出去的呢？这里就引入了 BRDF 来进行解决。由于不同方向反射的能量是不同的，所以反射出去的能量可以看成是一个分布，BRDF 解决的正是**从某个方向辐射到表面的能量，它所辐射到其他方向的能量分布问题**，它描述了反射方向上的能量分布 （这一切的前提都是在物体表面会把接收的能量辐射出去的前提下）

定义物体表面单位面积在**单位入射立体角ωiωi​、单位时间下通过光源入射所接收到的能量dEi(ωi)dEi​(ωi​)**，假设它反射了能量，定义该单位表面沿某个单位出射立体角ωrωr​反射的能量为dLr(ωr)dLr​(ωr​)，人们就将 `BRDF` 定义为它们两者的比值：

其实就是一个比例：对于任何一个出射方向，算出来他的Radiance，除以dA接受到的Irradiance，就是`BRDF`。

![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153234.16904861319126153278216492685981:20260909124125:2415:F55F6B828687E129D87A2D2F1E691FB198E21CA7416E491678BBD7E8CAE05388.png)

### 1.4 反射方程

上述过程只是寻找某个单位入射立体角方向的反射能量分布，那么当所有方向都对物体表面辐射了能量，就需要将这些能量一并考虑进反射方程。所以对于某个单位表面来说，它沿某个出射立体角方向所反射的能量就是光沿所有方向辐射给该表面的能量，定义如下：  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153234.90200014794637031658784740718305:20260909124125:2415:FB16D519666A51289A9605D5A25EEBE9E9B8576D3B50E2E924724FF3880DBD4A.png)

这也正是`反射方程`的定义。

由定义也可以看出，它可以理解为：在一个表面位置p和出射方向ωoωo​，不同入射方向ωiωi​的光源都对该位置辐射了能量，使得该表面会沿出射方向反射能量LiLi​。而不同入射方向会使得表面产生不同的反射能量，这时就需要一个描述反射能量分布的函数fr(ωi→ωr)fr​(ωi​→ωr​) 来描述表面在不同方向的`反射能量分布（BRDF）`。求出所有反射能量后，将它们求和即得到沿该出射方向的总反射能量。

具体来说：从某个方向（camera）看某个着色点（dA）就是一个积分。  
这个积分考虑的就是任何一个不同的方向（dωidωi​），然后考虑它的Radiance（Li(p,ωi)cosθidωiLi​(p,ωi​)cosθi​dωi​），Radiance进来之后，经过BRDF（fr(p, ωi→ωr)）会变成出射的Radiance。  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153235.85246741986685942988161997189466:20260909124125:2415:17F5550F915E5C7D255C6F2DA3DCE18434899F4A680B036B77EC42B5563BA423.png)

challenge：这个点来的Radiance，不只是有光源部分的，还有其它的（eg：其它点出射的Radiance）

### 1.4 渲染方程

渲染方程简单理解就是：物体自反光 + 物体反射光

上一节已经计算出了物体沿各个方向通过入射光计算得到的物体反射光，而物体自发光可以通过人为规定的材质来进行定义，所以渲染方程就可以定义如下：  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153235.29599400841023299711149959602269:20260909124125:2415:BB1D94A84C92D68F9E0364042909C91DAD96135B9374A07B68099A2336D1258D.png)  
注意：  
1.所有方向认为朝外  
2.认为下半球贡献为0（定义积分忽略下半球）  
3.n·ωi=cosθi

#### 1.4.1 渲染方程的由来

![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153235.53583193774041312598870519232749:20260909124125:2415:4B8DDB366B4C1AE48A696D7A6B80E9C718F20E591BF74FC32D42B7D025E61336.png)

1. 假设有多个点光源-> 加起来  
    ![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153236.12639298874032046054090205568535:20260909124125:2415:42CD89648137766858F9910E8A6266561662E1BD73131F4B62653843F0B99A70.png)
    
2. 面光源怎么办？ -> 积分起来（点光源集合）  
    ![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153236.46789463395964257118990653805508:20260909124125:2415:EDB267B5C80F2F8AB1FF21D321C5F279CA174079F29D405C5DD6BCF49E4E3510.png)
    
3. 有其它物体反射过来的Radiance -> 把其它来源也当成 光源处理  
    ![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153236.70621814748516990721406499659655:20260909124125:2415:0BACB775066168EF90DFF497BD73021B0D71FF5ED1AB3EC048319E8C52C42FCA.png)
    

##### 把问题理解为递归方式

a. 数学简写  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153236.83536593414430896160880240618839:20260909124125:2415:270DE6B200C72FABA8A39D50294BADDF840CD0E5474C59C39C44049842625FA0.png)

b. 再简写(为了解出L)  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153237.76100413385249680492380185436589:20260909124125:2415:6B660192563B8B7862E8EFDDB7870583275865B8ED49D4C4D868F26A6DF572D2.png)

c. 怎么解  
数学变换：L可以写成如下:  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153237.74363326831839713875731962318830:20260909124125:2415:516DADD65A8B32CE50FA5192E2A3D5D0788814BE59613CCB19A861BF1E2813E0.png)  
这里主要运用了算子同样具有类似泰勒展开的性质

d. 最后可以理解为能量分解->全局光照  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20251216153237.56848210009418908322369799211381:20260909124125:2415:8A455E5EF0776749C72D09E781EDF0C2264902CE7541B32CE3B8464DCEBC49B3.png)

- E：直接看到的光源（直接光照）
- kE：辐射出的能量经过1次反射，会看到（间接光照）
- k22E：经过2次反射  
    …

### 1.5 全局光照概念

全局光照=直接光照+间接光照

#### 1.5.1 光栅化的局限

仅可以做：L=E+kE（只能做0次和1次）  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20250424154851.01725401960581608346511352114177:20260909124125:2415:2955780E437846A894E3EEB340A26B0529C008739A30840DFA1B5EE744594980.png)

这里也推出了Ray-Tracing的意义：可以做后续的弹射

一个直观的例子

- 直接光照（光栅化结果）  
    ![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20250424154851.60880067412891201027073764468012:20260909124125:2415:05A1E9E4513269DF0388D11E4556604D7301D404B5CFDD777969D50F7E6BE4CA.png)
    
- 加入间接光照（弹2次，光追）  
    ![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20250424154852.31704922164791817649076164046635:20260909124125:2415:BE49F0449AE44A8F4C2BE8EC52E08DA41F02ADAF8052A50A8D31D11B1D2993A2.png)
    

## 二、概率论基本知识

用于后续求解渲染方程

### 2.1 随机变量与概率密度函数 (PDF)

`随机变量`：以一定概率出现的变量值  
`概率密度函数`:在一定区间进行采样，用来描述随机变量的出现概率的分布函数  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20250424154852.12870750526554482629190318665449:20260909124125:2415:047BF88F0897AC2EFE323F3C31886A0E9B207EEC5AA1116FDECD5F69414A8575.png)

`概率`(probabilities)：  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20250424154852.99585190392462874468521256678361:20260909124125:2415:DAF766DF6BA75E057E04D57526EE7BD0CB2A90BBCACEA99BAA6B3D05A3BF3CE4.png)

### 2.3 期望

离散的数学期望就是把所有随机变量与对应的概率的乘积求和，它反应的是随机变量平均出现的概率：  
![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20250424154852.14607600006313992313441124260910:20260909124125:2415:1547C0F0A0BC3C2B33CF982CFD491CF3CD7B94416BA80532577A65C8269188EB.png)

### 2.4 连续情况下描述变量和分布

pdf：概率密度函数要满足：  
a. 线下面积为1  
b. ∫x·概率密度=E（X）

![image.png](https://fileserver.developer.huaweicloud.com/FileServer/getFile/community/720/574/456/0001538023720574456.20250424154853.15668843201895810876002125572044:20260909124125:2415:7D3A475369121350B4A5FF85F241E287A34A60E7BE2E359B5918E3CE14823AD8.png)

对本身是函数的随机变量求期望，就等于直接把该函数看成是输入：  
E[Y]=E[f(X)]=∫f(x)p(x)dxE[Y]=E[f(X)]=∫f(x)p(x)dx




# 13~16 Ray Tracing
 [华为云](https://bbs.huaweicloud.com/blogs/351709) 上的整理看上去很不错，本章就拿来了，做了一些删减工作和一点点补充：)。由于遵循了课程顺序，本章分节有待商榷。

光栅化是把场景中的物体独立出来，并且再将物体独立成一个个的三角形面片进行渲染，通常 GPU 绘制三角形面片是并行的，所以对于物体来说，它是不知道自己周围的物体，同样的，对于三角形面片来说，它也是不知道自己周围的三角形。所以光栅化无法处理一类需要全局信息进行渲染的效果，如：软阴影、Glossy 反射以及间接照明。

相比较光栅化，光线追踪是从相机出发，计算相机发出的光线（视线）与场景产生的结果，这里的光线可能会与场景中的物体碰撞后生成另外一些光线，然后生成的光线也需要继续做计算。所以对于光线追踪来说，相机产生的光线是知道整个场景信息的。

光栅化与光线追踪的对比：
1. 光栅化快，实时，是一种近似算法
2. 光线追踪慢，离线，是一种准确的算法

## 1. Recursive (Whitted-Style) Ray Tracing

先来学习一种传统的光线追踪算法，称为Whitted风格。

该方法有几点不同于常规的着色方案：

1. 光源：光线不同，这里假设光线在碰撞到物体后还会产生折射和反射。
    
2. 着色过程：每发生一次折射或者反射（弹射点）都计算一次着色，前提是该点不在阴影内。
    

如图所示，该算法的过程如下：

1. 从视点从成像平面发出光线，检测是否与物体碰撞
    
2. 碰撞后生成折射和反射部分
    
3. 递归计算生成的光线
    
4. 所有弹射点都与光源计算一次着色，前提是该弹射点能被光源看见
    
5. 将所有着色通过某种加权叠加起来，得到最终成像平面上的像素的颜色
    

![](https://iewug.github.io/book/img/13whitted.png)

为了后续说明方便，课程定义了一些概念：

- 从视角出发第一次打到物体的光线叫做：`primary ray`
    
- 弹射之后的光线叫做：`secondary rays`
    
- 判断可见性的光线叫做：`shadow rays`
    

### 1.1 Ray-Surface Intersection

Recursive (Whitted-Style) Ray Tracing涉及到光线与物体的碰撞问题，所以这里介绍一下如何判断光线-表面相交问题。

#### 1.1.1 Ray Equation

光线可以一个点o和方向向量d定义：

#### 1.1.2 Ray Intersection With Implicit Surface

如图解t即可：

![](https://iewug.github.io/book/img/13intersection.png)

#### 1.1.3 Ray Intersection With Triangle

计算步骤：先求光线和平面的交点 → 再判断点是否在三角形内

光线与平面交点求解：

![](https://iewug.github.io/book/img/13intersection2.png)

**Moller Trumbore Alogrithm（MT算法）**

一种快速计算光线是否在三角形内以及与平面交点的算法，其核心出发点是用重心坐标表示平面。

![](https://iewug.github.io/book/img/13MT.png)

具体步骤：

1. 求解t，b1，b2（三个式子，三个未知数——求解方法：克莱姆法则）
    
2. 解出来之后，看是否合理：①沿着这个方向（t非负）②在三角形内（b1，b2非负）
    

### 1.2 Accelerating Ray-Surface Intersection

加速原理：不再判断光线与每个三角形的相交情况，而是判断光线与包围物体表面的盒子来粗略判定光线是否与物体相交。如果与**包围盒（bounding volume）**相交，再判断光线是否与物体相交。

#### 1.2.1 Bounding Volume

这里主要介绍了 **AABB 包围盒（Axis-Aligned Bounding Box）**，它是轴对齐的，也就是其平面与xOy、yOz或zOx平面平行。通常在判断物体相交情况的时候，可以先检测物体是否与 AABB 包围盒是否相交，如果相交再进一步做判断（更精细的相交判断）。

#### 1.2.2 Ray Intersection with Axis-Aligned Box

这里给了一个 2D 的例子进行解释：

![](https://iewug.github.io/book/img/13AABB.png)

需要记住的关键概念是：

1. 光进入了所有对面（slab，一对平行的平面），才算进入Box
    
2. 光离开任意一对面，就算离开了包围盒
    
3. 什么时候有交点：时有交点
    
4. 考虑负的情况： 说明Box在光背后
    
5. 说明光线起点在Box内 → 有交点
    
6. 对于三维包围盒来说，有如下公式成立：
    

**总之，当且仅当时，光线与AABB相交**。（该算法背后是**Liang-Barsky算法**）

## 2. Using AABBs to Accelerate Ray Tracing

lecture14继续介绍加速光线追踪的内容。是1.2节的延续。

本节我们将介绍Uniform grids，Spatial partitions和Object partitions三种场景划分方式。

### 2.1 Uniform Grid

Uniform grids 简单来说就是将场景划分成一个个规整的格子，通过格子来判断光线与物体的相交情况。

构建Uniform grids 的步骤如下：

1. 找到包围盒
    
2. 创建格子
    
3. 存储每个对象至格子中
    

那么此时判定光线与物体相交就转换成光线与格子相交问题，当光线与格子相交后，再去考虑它与格子内物体的相交情况：

![](https://iewug.github.io/book/img/14uniformGrid.png)

这种方式有一些问题：

1. 格子的分辨率问题，如果格子的分辨率设置太小就失去了划分的意义；如果分辨率太大就要做好多次和格子求交，效率不高。
    
2. 格子通常只适用于规整的场景，相反，物体位置较为稀疏的场景就不适用这种划分方案，因为此时很多格子都不存储物体，大量时间被浪费在了光线与格子的求交上
    

### 2.2 Spatial Partition

通常有 Oct-Tree（八叉树） 、KD-Tree 以及 BSP-Tree 三种划分方案：

![](https://iewug.github.io/book/img/14spatialPartition.png)

- **Oct-Tree**：每次迭代都将区域重新切分为均匀四块，按一定规则停止切分（如切分得到的四块区域中，三块都没有物体；或四块区域都还有物体，但是此时的区域已经较小）
    
- **KD-Tree**：总是沿某个轴进行切分，每次划分总会在原来的区域上生成两块新的区域（这里沿轴的次序是由交替进行的，如二维中总是按着x y的次序进行交替切分）
    
- **BSP-Tree**：每次都是沿着一定方向进行切分（非水平或竖直）
    

下面着重介绍KD-tree。

**1）Pre-Processing**

假设空间 A 被划分为如图所示 KD-Tree，那么它的结构等同于其中树结构所所描述：

![](https://iewug.github.io/book/img/14kdtree.png)

**2）Data Structure for KD-Trees**

中间节点（A，B，C，D）存储了：

1. 划分后的轴
    
2. 划分后的切分平面
    
3. 子节点的指针
    

叶子节点（1，2，3，4，5）存储了：区域中包含的物体的列表（obj只会存在于叶子节点）

**3）Traverse**

1. 发射光线从根节点出发，分别判断光线与左右节点是否相交，若相交则进入2；否则，则与节点不相交
    
2. 递归判断相交直至叶子节点，若与叶子节点相交，进入3
    
3. 挨个判断叶子节点存储物体与光线的相交情况
    

![](https://iewug.github.io/book/img/14kdtree2.png)

**4）Problem**

1. 三角形mesh与盒子的交点不好判断
    
2. 物体可能与多个包围盒相交
    

### 2.3 Object Partition & Bounding Volume Hierarchy (BVH)

为了解决上述问题，人们提出了另外一种划分方案，即在场景中对物体进行划分，此时就不用考虑三角形与包围盒的求交问题

它本质是将一个场景用一个包围盒包住，然后按照一定划分方案将盒子划分成不同的子区域，不同子区域都需要包含三角形，最终划分到叶子节点时，每个叶子节点就包含了一些三角形，即包含了对应的一些物体：

![](https://iewug.github.io/book/img/14objectPartition.png)

**1）Building BVHs**

三种想法

- 按轴的次序进行划分
    
- 按最长轴继续进行划分
    
- 在物体三角形数目的最中间对物体进行划分（这里的最中间指的是划分后，两边的三角形数目基本一致）
    

[Visualizing the Boundary Volume Hierarchy algorithm — Harold Serrano - Game Engine Developer](https://www.haroldserrano.com/blog/visualizing-the-boundary-volume-hierarchy-collision-algorithm)提供了第三种的具体画法（事实上也包含了第二种）。

何时停止？当包围盒中物体个数小于一个定值时停止。

**2）Data Structure**

- 中间节点（Internal nodes）：该节点对应的包围盒和子节点的指针
    
- 叶子节点（Leaf nodes）：该节点对应的包围盒和包围盒里面的物体列表
    

**3）BVH Traversal**

![](https://iewug.github.io/book/img/14bvhTraversal.png)

**4）Spatial vs Object Partitions**

![](https://iewug.github.io/book/img/14spatialVSobject.png)

## 3. Radiometry

为了对物理世界的光做更加真实的描述，我们需要介绍**辐射度量学（radiometry）**。

- **辐射通量（radiant flux）**：. 单位时间的辐射能量，亦作辐射功率
    
- **辐射强度（radiant intensity）**：. 单位立体角的辐射通量
    
- **辐照度（irradiance）**：. 单位表面的辐射通量
    
- **辐射率（radiance）**：单位立体角和垂直此方向的单位表面的辐射通量。如图
    

![](https://iewug.github.io/book/img/15radiance.png)

## 4. Bidirectional Reflectance Distribution Function (BRDF)

BRDF 解决了**从某个方向辐射到表面的能量，它所辐射到其他方向的能量分布问题**。BRDF公式如下：

![](https://iewug.github.io/book/img/15brdf.png)

对做积分，得到**反射方程（reflection equation）**：（图中H2表示上半球面）

![](https://iewug.github.io/book/img/15reflectionEquation.png)

**渲染方程（rendering equation）**就是物体自发光加上反射方程：（图中表示上半球面，p是着色点）

![](https://iewug.github.io/book/img/15renderingEquation.png)

接下来我们换一个角度来理解上述公式，来看看L到底包含了什么。下面的ppt包含了积分方程、矩阵泰勒展开等等数学知识。最后导出了漂亮的结果，可以看到物体发出来的辐射率包含了自发光、一次反射、二次反射等等。

![](https://iewug.github.io/book/img/15equation1.png)

![](https://iewug.github.io/book/img/15equation2.png)

![](https://iewug.github.io/book/img/15equation3.png)

![](https://iewug.github.io/book/img/15equation4.png)

## 5. Monte Carlo Integration

求定积分的一种估计方法

## 6. Path Tracing

### 6.1 Whitted-Style Ray Tracing‘s Problem

- 在处理光照反射时总是沿着镜面反射方向进行，这适用于镜面材质，但不适用于 Glossy 之类的材质
    
- 不考虑物体表面漫反射会进一步反射，但实际上漫反射物体仍会继续反射光线
    

### 6.2 A Monte Carlo Solution

相较于这些问题，第四节介绍的渲染方程仍然是对的，所以我们只需要按照渲染方程来求解物体表面的着色问题即可。由蒙特卡洛方法有：（式中pdf是概率密度函数（probability density function））

![](https://iewug.github.io/book/img/16_1.png)

算法：

![](https://iewug.github.io/book/img/16_2.png)

上述算法的问题：

- 光线数量会爆炸，因为一个着色点会反射多个光线（选时）。我们可以将反射的光线数目设为1（即**路径追踪path tracing**的概念），于是shade函数变为：
    
    ![](https://iewug.github.io/book/img/16_4.png)
    
    但是这样噪声很大，于是我们在每个像素处打入多条光线来解决这个问题。
    
    ![](https://iewug.github.io/book/img/16_3.png)
    
    ![](https://iewug.github.io/book/img/16_5.png)
    
- 递归无法停止。使用**Russian Roulette（俄罗斯轮盘赌）**解决。
    
    ![](https://iewug.github.io/book/img/16_6.png)
    

至此一个正确的路径追踪算法实现了，但效率不高。要求有高采样率，即一个像素要发出很多条光线进行采样。我们希望通过低采样率也能获得高采样率的效果。

![](https://iewug.github.io/book/img/16_7.png)

这里的做法就是将物体表面接收到的 radiance 分解为两部分：来自光源和来自其他反射物体。

- 对于来自光源的部分，直接在光源上进行采样即可，采样过程就是换积分域的过程
    
- 对于来自其他反射的光源，仍然按照RR算法求解
    

换积分域如图：

![](https://iewug.github.io/book/img/16_8.png)

![](https://iewug.github.io/book/img/16_9.png)

最终的伪代码如下：

![](https://iewug.github.io/book/img/16_10.png)

上述路径追踪算法效果相当相当的好。当然辐射率并不是颜色，还需要γ校正等操作。



