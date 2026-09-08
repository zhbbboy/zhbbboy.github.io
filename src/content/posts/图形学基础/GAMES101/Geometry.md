---
title: Geometry
date: 2026-09-07
tags: []
draft: false
---
## 1. Representation

### 1.1 Implicit

包括

- **algebraic surface**: 用表示一个平面。判断内外容易，得到表面上的点困难。
    
- **constructive solid geometry (CSG)**：用交并差等运算表示
    
- **distance function**：空间任何一点到物体表面距离的最小值（可以是带符号的）
    
- **level set**（水平集）：store a grid of values approximating function; surface is found where interpolated values equal zero。譬如用密度做体渲染，找密度为一个定值的表面
    
- **fractal（分型）**
    

隐式表示的优劣

**Pros**

- compact description (e.g., a function)
    
- certain queries easy (inside object, distance to surface)
    
- good for ray-to-surface intersection (more later)
    
- for simple shapes, exact description / no sampling error
    
- easy to handle changes in topology (e.g., fluid)
    

**Cons**

- difficult to model complex shape
    

### 1.2 Explicit

- 。得到表面上的点容易，判断点在表面内外困难。
    
- **point cloud（点云）**
    
- **mesh**，使用`.obj`文件描述mesh
    

## 2. Curve

这里着重介绍**贝塞尔曲线（Bézier Curves）**。下图有三个**控制点（control point）**，取所有[0,1]之间的t，便可以画出该曲线。当然四个甚至更多的控制点都是可行的。特别的四个控制点的称为**cubic Bézier curve**

![](https://iewug.github.io/book/img/11curve.png)

代数形式：

![](https://iewug.github.io/book/img/11algebraic.png)

更一般的，n+1个控制点下，曲线可以表示为：

贝塞尔曲线性质（n个控制点）：

- 端点切线斜率：
    
- 仿射变换后做贝塞尔曲线，与先做贝塞尔曲线再仿射变换，结果一致
    
- 凸包性质：贝塞尔曲线一定在凸包（最小围住控制点的多边形）内
    

**分段贝塞尔曲线（Piecewise Bézier Curve）**

也就是一根曲线由多个贝塞尔曲线组成。连接点的连续性可以定义：

- C0 continuity：第一段终点和第二段起点相同，
    
- C1 continuity：还要求切线斜率一致，
    

## 3. Surface

### 3.1 Bézier Surface

考虑的控制点阵，先用每列四个控制点画出四条贝塞尔曲线（图中灰色，参数u控制），再利用这四条曲线在相同参数u上的点，画出一条贝塞尔曲线（图中蓝色，参数v控制）。诸多这样蓝色的曲线就可以组成贝塞尔曲面。

![](https://iewug.github.io/book/img/11surface.png)

### 3.2 Mesh

这里介绍对网格的**细分（subdivision）**、**简化（simplification）**和**正则（regularization）**操作。

![](https://iewug.github.io/book/img/12meshOp.png)

#### 3.2.1 Subdivision

网格细分的思路是：

1. 创建更多的三角形（顶点）
    
2. 调整所有顶点的位置
    

下面介绍两种细分方法。

**1）Loop Subdivision**（注意到Loop是人名）

首先，将每个三角形分成四份：

![](https://iewug.github.io/book/img/12Loop1.png)

然后，调整顶点位置：

- 新顶点
    

![](https://iewug.github.io/book/img/12Loop2.png)

- 旧顶点，可以看到如果度越大，相邻顶点对原始顶点的影响越小
    

![](https://iewug.github.io/book/img/12Loop3.png)

**2）Catmull-Clark Subdivision**

Loop细分只能解决三角形网格的细分问题，对于一般的情况可以采用Catmull-Clark细分。

**基本概念**

- **非四边形面（Non-quad face）**：顾名思义。
    
- **奇异点（Extraordinary vertex）**：度数不为4的点。
    

首先，添加新点

- 取每个面的一个点（比如重心），取每个边的中点，将所有新添加的点做连接
    
- 经过一次细分，所有非四边形面会消失，变成一个奇异点
    

![](https://iewug.github.io/book/img/12catmull.png)

然后，调整位置

![](https://iewug.github.io/book/img/12catmull2.png)

#### 3.2.2 Simplification

我们采取**边坍缩（edge collapsing）**的方法来简化曲面。

![](https://iewug.github.io/book/img/12meshSim.png)

先要介绍**二次误差（quadric error）**，即新的点应该使与它相关联的面的平方和达到最小。

![](https://iewug.github.io/book/img/12quadricError.png)

于是简化的步骤如下：（贪心算法）

1. 假设坍缩每一条边，然后分别得到它们的二次误差。 做为各自的分数
    
2. 每次都坍缩分数最小的边，然后更新被影响边的分数
    
3. 重复步骤1
    

## 另. Shadow Mapping

_不属于几何这一部分，但是somehow就在lecture12介绍了，应该还是属于光栅这一块的_

这里提供一个生成物体影子的办法，其核心想法是：不在影子中的点必须同时被光源和照相机看到。

![](https://iewug.github.io/book/img/12shadowMapping.png)

1. 首先记录光源能看到点的深度信息
    
2. 从眼睛看向物体，将所看到的点，投射回记录深度信息的那张图。在对应像素位置，找到当时的深度信息，与现在该点到光源的距离进行比较。如果当前距离比之前的深度大，说明中间有物体遮挡，即在阴影之中（如点2）；如果相等，则不在阴影中（如点1）。
    

**Shadow Mapping的优点**：快

**Shadow Mapping 的问题**

- 理论上只能硬阴影（适用于点光源）
    
- 依赖贴图的分辨率，走样问题
    
- 浮点精度误差，浮点数判断相等
    

所谓的硬阴影和软阴影，其实可以理解为**本影（umbra）**和**半影（penumbra）**。

CS3310还补充了**阴影体（shadow volume）**和**平面阴影（planar shadow）**来计算阴影的方法。

**planar shadow**

思想很简单，根据光源和投影面位置推导一个投影矩阵，通过这个矩阵能把模型上所有顶点投射到投影面（比如地面）上，也就是将渲染物体压扁到一个平面上。

**shadow volume**

![](https://iewug.github.io/book/img/shadowVolume.png)

上图中红色点是进入阴影体，紫色点是出阴影体。只需判断观察点进出阴影体数目是否相同（可以使用stencil buffer来计数）。相同的话就不在阴影体中。该算法性能不好，且理论上也只能生成硬阴影，不过不会因采样和分辨率而产生的问题。