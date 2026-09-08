---
title: Rasterization
date: 2026-09-05
tags: []
draft: false
---
# GAMES101 Lecture 5：Rasterization 1（光栅化 1：三角形）

> 依据 GAMES101 Lecture 5 的讲解顺序整理。  
> 目标：跟着课程理解 Rasterization（光栅化）的基本思路。
  
**Rasterization（光栅化）总知识路线**
```
MVP Transformation
        ↓
Canonical Cube（标准立方体）
        ↓
Viewport Transformation（视口变换）
        ↓
Screen Space（屏幕空间）
        ↓
Pixel（像素）
        ↓
Triangle Primitive（三角形图元）
        ↓
Rasterization（光栅化）
        ↓
Sampling（采样）
        ↓
Point-in-Triangle Test（三角形内点测试）
        ↓
得到离散像素
        ↓
Aliasing（走样）
        ↓
Frequency（频率）
        ↓
Filtering（滤波）
        ↓
Antialiasing（抗走样）
        ↓
Supersampling / MSAA
        ↓
Visibility / Occlusion（可见性 / 遮挡）
        ↓
Z-Buffering（深度缓冲）
```
---

## 1. 上一节内容回顾

上一节主要学习了 Viewing Transformation（观测变换）：

- View / Camera Transformation（视图 / 相机变换）
- Projection Transformation（投影变换）
- Orthographic Projection（正交投影）
- Perspective Projection（透视投影）

这一节会先把上一节的内容收尾，然后正式进入 Rasterization（光栅化）。

---

## 2. Perspective Projection（透视投影）中的两个常见参数

### 2.1 Field of View, FOV（视场角）

课程中使用的是：

**vertical field-of-view, fovY（垂直视场角）**

可以理解为：

> 相机在竖直方向上能够看到多大的范围。

fovY 越大：

- 看到的范围越广。

fovY 越小：

- 看到的范围越窄。

---

### 2.2 Aspect Ratio（宽高比）

定义：
![](../../_Pasted_Image/Pasted%20image%2020260905144027.png)
例如屏幕分辨率为：

```text
1920 × 1080
```

则：
![](../../_Pasted_Image/Pasted%20image%2020260905144034.png)
也就是常见的：

```text
16 : 9
```

课程这里的意思是：

> 如果知道 `fovY` 和 `aspect`，就可以反推出透视投影近平面的 `l、r、b、t`透视投影里近平面（near plane）矩形的四个边界坐标。

这一部分主要还是上一节 Projection（投影）的收尾内容。

---

# 3. MVP 之后是什么？

课程第 7 页提出：

> What's after MVP?  
> MVP 之后是什么？

前面我们做了：

```text
Model Transformation（模型变换）
        ↓
View Transformation（观察变换）
        ↓
Projection Transformation（投影变换）
```

Projection（投影）之后，我们会得到一个：

**Canonical Cube（标准立方体）**

范围为：
![](../../_Pasted_Image/Pasted%20image%2020260905144259.png)
也就是：

```text
x ∈ [-1,1]
y ∈ [-1,1]
z ∈ [-1,1]
```

那么下一步的问题就是：

> Canonical Cube（标准立方体）如何变到 Screen（屏幕）？

这就引出了接下来的 Viewport Transformation（视口变换）。

---

# 4. 什么是 Screen（屏幕）

课程将屏幕定义为：

> An array of pixels  
> 一个由 Pixel（像素）组成的二维数组。

例如：

```text
1920 × 1080
```

本质上就是：

```text
1920 列 × 1080 行
```

的 Pixel（像素）。

---

## 4.1 Resolution（分辨率）

屏幕二维像素数组的大小称为：

**Resolution（分辨率）**

例如：

```text
1920 × 1080
2560 × 1440
3840 × 2160
```

---

## 4.2 Raster（光栅）

课程中提到：

> Raster == screen in German

这里可以将 Raster（光栅）理解为：

> 一种规则排列的二维网格。

因此：

**Rasterize（光栅化）**

在这一阶段可以先直观理解成：

> 把图形绘制到由像素构成的屏幕上。

---

# 5. Pixel（像素）是什么

Pixel 是：

**Picture Element（图像元素）**

的简称。

GAMES101 目前采用一个简化模型：

> A pixel is a little square with uniform color.  
> 一个像素可以暂时看成一个颜色均匀的小正方形。

颜色由：

- R：Red（红）
- G：Green（绿）
- B：Blue（蓝）

共同组成。

真实显示器中的 Pixel（像素）结构实际上更加复杂。

---

# 6. Screen Space（屏幕空间）

GAMES101 中将屏幕坐标定义为：
![](../../_Pasted_Image/Pasted%20image%2020260905144451.png)

即：
- 左下角为 `(0,0)`
- X 轴向右
- Y 轴向上

不同图形 API 或图片格式可能采用不同的坐标方向，但当前按照 GAMES101 的定义理解即可。

---

# 7. 像素编号和像素中心不是一回事

像素编号：

```text
Pixel (x, y)
```

其中：
```text
x
y
```
都是整数。

因为它表示的是：
> 第几列、第几行像素。

例如：
```text
(0,0)
(1,0)
(2,0)
```

---

## 7.1 Pixel Center（像素中心）

Pixel `(0,0)` 所占据的区域可以理解为：
![](../../_Pasted_Image/Pasted%20image%2020260905144722.png)

因此它的中心为：
![](../../_Pasted_Image/Pasted%20image%2020260905144732.png)
同理：

Pixel `(1,0)` 的中心：

(1.5,0.5)

Pixel `(3,4)` 的中心：

(3.5,4.5)

因此一般写成：
![](../../_Pasted_Image/Pasted%20image%2020260905144810.png)

这个公式在后面的 Sampling（采样）中非常重要。

---

# 8. 屏幕坐标范围

假设：
```text
width = 800
height = 600
```

像素下标范围为：
```text
x：0 ~ 799
y：0 ~ 599
```

即：
![](../../_Pasted_Image/Pasted%20image%2020260905144839.png)
但是整个屏幕覆盖的几何坐标范围为：
![](../../_Pasted_Image/Pasted%20image%2020260905144846.png)
也就是：

```text
左下角：(0,0)
右上角：(800,600)
```

所以：

```text
最大像素下标：(799,599)

屏幕右上角：(800,600)
```

这两个概念不要混淆。

---

# 9. Canonical Cube（标准立方体）如何变到屏幕？

标准立方体中：
![](../../_Pasted_Image/Pasted%20image%2020260905145005.png)

课程强调：

> Irrelevant to z  
> 这里暂时与 z 无关。

因为当前主要是在处理 x、y 的屏幕映射。

---

# 10. x 坐标的变换

原范围：

```text
-1 ── 1
```

目标范围：

```text
0 ── width
```
![](../../_Pasted_Image/Pasted%20image%2020260905145129.png)
先缩放：
![](../../_Pasted_Image/Pasted%20image%2020260905145152.png)
则范围由：[-1,1]
变成：[-width/2,width/2]
![](../../_Pasted_Image/Pasted%20image%2020260905145248.png)

---

# 11. y 坐标的变换

同理：
![](../../_Pasted_Image/Pasted%20image%2020260905145326.png)



---

# 12. Viewport Transformation（视口变换）

将前面的 x、y 变换写成矩阵：
![](../../_Pasted_Image/Pasted%20image%2020260905145352.png)

可以理解为：
```text
x：
缩放 width/2
+
平移 width/2

y：
缩放 height/2
+
平移 height/2

z：
暂时不改变
```

---

# 13. MVP 到屏幕的整体流程

现在可以把整个过程串起来：

```text
模型自己的坐标
        ↓
Model Transformation（模型变换）
        ↓
世界坐标
        ↓
View Transformation（观察变换）
        ↓
相机坐标
        ↓
Projection Transformation（投影变换）
        ↓
Canonical Cube（标准立方体）
        ↓
Viewport Transformation（视口变换）
        ↓
Screen Space（屏幕空间）
```

此时，一个三角形的顶点可能变成：

```text
(2.2, 1.3)
(4.4, 11.0)
(15.3, 8.6)
```

这些屏幕坐标完全可以是小数，不需要是整数。

---

# 14. 不同的 Drawing Machines（绘制设备）

课程中展示了多种不同的绘制和显示设备，包括：

- CNC Drawing Machine（数控绘图机）
- Laser Cutters（激光切割机）
- Oscilloscope（示波器）
- Cathode Ray Tube, CRT（阴极射线管）
- LCD（液晶显示器）
- LED Array Display（LED 阵列显示）
- Electronic Ink Display（电子墨水显示器）

> “绘制”图像在物理世界中可以有很多种不同实现方式。

但当前计算机图形学主要关心的是：
**Raster Display（光栅显示器）**
也就是通过规则排列的像素显示图像。

---

# 15. Raster Scan（光栅扫描）

老式 CRT（阴极射线管）显示器可以通过：

**Raster Scan（光栅扫描）**

逐行扫描屏幕。

可以简单理解为：

```text
第一行  →→→→→→
第二行  →→→→→→
第三行  →→→→→→
第四行  →→→→→→
```

通过扫描不同位置并改变亮度或颜色，从而形成图像。

---

# 16. Frame Buffer（帧缓冲）

课程提出：

> Frame Buffer: Memory for a Raster Display  
> 帧缓冲：用于光栅显示器的一块内存。

可以简单理解为：

> Frame Buffer（帧缓冲）是一块用来保存屏幕各像素颜色数据的内存。

课程中有一句很重要的话：

> Image = 2D array of colors

即：
 图像可以看作一个二维颜色数组。> ![](../../_Pasted_Image/Pasted%20image%2020260905150114.png)

---

# 17. 正式进入 Rasterization（光栅化）

课程第 26 页：

> Rasterization: Drawing to Raster Displays  
> 光栅化：在光栅显示器上绘制。

接着介绍：
- Polygon Meshes（多边形网格）
- Triangle Meshes（三角形网格）

也就是说：

> 接下来我们主要研究如何把 Triangle（三角形）绘制到像素屏幕上。

---

# 18. 为什么使用 Triangle（三角形）

课程将三角形称为：

**Fundamental Shape Primitive（基本图形图元）**

Primitive（图元）可以理解为：

> 图形学中最基本的绘制几何单位。

课程给出的原因包括：

---

## 18.1 Most Basic Polygon（最基本的多边形）

三角形只有三个顶点，是最基本的多边形。

---

## 18.2 Break Up Other Polygons（其他多边形可以拆成三角形）

例如一个四边形：

```text
A────B
│   /│
│  / │
│ /  │
D────C
```

可以拆成：

```text
△ABC
+
△ACD
```

所以复杂模型最终可以由很多 Triangle（三角形）组成。

---

## 18.3 Guaranteed to be Planar（一定共面）

三个不共线的点一定可以确定一个平面。

因此三角形天然是平面的。

---

## 18.4 Well-defined Interior（内部定义明确）

对于一个三角形，可以明确判断：

> 某个点到底是在三角形内部还是外部。

这个性质马上就会用于 Rasterization（光栅化）。

---

## 18.5 Barycentric Interpolation（重心坐标插值）

三角形可以利用：

**Barycentric Interpolation（重心坐标插值）**

在三个顶点之间插值各种属性。

这一点会在后续课程中详细学习。

---

# 19. 光栅化真正的问题

课程提出：

> What Pixel Values Approximate a Triangle?  
> 应该用哪些像素值去近似一个三角形？

输入：

> 三角形三个顶点投影后的屏幕坐标。

输出：

> 一组离散的像素值，用这些像素近似原本连续的三角形。

注意这里非常重要的词：

**Approximate（近似）**

因为：

```text
数学中的三角形
```
是连续的。

而：

```text
屏幕上的像素
```
是离散的。

所以 Rasterization（光栅化）本质上需要：

> 用有限的离散像素去近似连续的几何图形。

---

# 20. A Simple Approach: Sampling（一个简单的方法：采样）

课程给出的办法是：
### Sampling（采样）



---

# 21. Sampling（采样）是什么

课程定义：

> Evaluating a function at a point is sampling.  
> 在某一个点上求一个函数的值，就叫采样。

例如：
![](../../_Pasted_Image/Pasted%20image%2020260905150517.png)
如果只计算：
```text
f(0)
f(1)
f(2)
f(3)
```
就相当于只在：
```text
x = 0,1,2,3
```

这些位置进行 Sampling（采样）。

得到：
```text
0
1
4
9
```

---

课程接着说：

> We can discretize a function by sampling.  
> 可以通过采样把一个连续函数离散化。

这正是 Rasterization（光栅化）的核心思想。

---

# 22. Rasterization As 2D Sampling（把光栅化看成二维采样）

为什么是二维？

因为屏幕是：(x,y)
构成的二维平面。
现在我们定义一个函数：inside(tri,x,y)

---

# 23. inside(tri, x, y)

![](../../_Pasted_Image/Pasted%20image%2020260905150706.png)
即：

```text
点在三角形内部 → 1

点不在三角形内部 → 0
```

所以：
> 光栅化可以看成对这个二维函数进行采样。

---

# 24. 在哪里进行 Sampling（采样）

答案：

> 在每一个 Pixel Center（像素中心）进行采样。

Pixel `(x,y)` 的中心为：(x+0.5,y+0.5)

所以课程中的代码为：
```cpp
for (int x = 0; x < xmax; ++x)
{
    for (int y = 0; y < ymax; ++y)
    {
        image[x][y] =inside(tri,x + 0.5,y + 0.5);
    }
}
```

例如像素：

```text
(3,2)
```

就测试：
inside(tri,3.5,2.5)
如果返回：
```text
1
```
认为：

> 这个像素被三角形覆盖。

如果返回：
```text
0
```
认为：

> 这个像素不属于三角形。

---

# 25. 课程图中的红点是什么意思

课程第 35～36 页中：

- 每一个小圆点代表一个 Pixel Center（像素中心）
- 红点表示该采样点位于三角形内部
- 空心点表示该采样点位于三角形外部

因此整个过程为：

```text
连续三角形
        ↓
在规则像素中心进行 Sampling（采样）
        ↓
判断每一个采样点是否在三角形内部
        ↓
保留内部采样点
```

这就是当前最基础的 Rasterization（光栅化）。

---

# 26. 如何实现 inside(tri, x, y)

课程接下来提出：

> 如何判断一个点到底在三角形内部？

给出的提示是：

> Recall: Three Cross Products!  
> 回忆：三个叉积。

假设三角形顶点为：

```text
P0
P1
P2
```

测试点为：

```text
Q
```

分别检查 Q 相对于三条边：

```text
P0 → P1
P1 → P2
P2 → P0
```

的位置。

使用：

**Cross Product（叉积）**

如果 Q 对三条有向边始终处于同一侧，则 Q 位于三角形内部。

这一部分可以单独继续深入学习。

---

# 27. Edge Cases（边界情况）

如果一个采样点恰好位于两个三角形的共享边上：

```text
Triangle 1
Triangle 2
```

那么需要规定：

> 这个采样点到底属于三角形 1、三角形 2，还是两个都属于？

这是 Rasterization（光栅化）中的边界处理问题。

GAMES101 这一页主要是提出问题，并没有在这一页继续深入具体规则。

---

# 28. 是否需要检查整个屏幕？

如果每一个 Triangle（三角形）都遍历整个屏幕：

```text
1920 × 1080
```

显然会浪费大量计算。

所以课程提出：
![](../../_Pasted_Image/Pasted%20image%2020260905151122.png)
# Bounding Box（包围盒）

只检查能够包住三角形的矩形区域。

矩形之外的像素：

> 一定不可能位于三角形内部，因此不用检查。

---

# 29. Incremental Triangle Traversal（增量式三角形遍历）

课程还介绍了一种遍历思想：

**Incremental Triangle Traversal（增量式三角形遍历）**

它不一定检查整个 Bounding Box（包围盒），而是更加贴近三角形实际覆盖的区域进行遍历。

课程说明它：

> suitable for thin and rotated triangles  
> 比较适合细长或旋转角度较大的三角形。

这一部分目前理解思想即可。

---

# 30. 真实显示器的 Pixel（像素）

前面一直假设：

```text
一个 Pixel
=
一个颜色均匀的小正方形
```

但课程后面展示了真实 LCD 显示器的像素结构。

可以看到其中存在不同排列方式的：

- R
- G
- B

Subpixel（子像素）

真实 Pixel（像素）并不是一个理想的均匀彩色正方形。

不过 GAMES101 明确说明：

> 在这门课中，仍然把每个像素近似成一个完整的彩色正方形。


---

# 31. 将采样结果真正显示出来

Sampling（采样）后可能得到：

```text
○ ○ ○ ● ○
○ ○ ● ● ○
○ ● ● ● ○
● ● ● ● ○
```

其中红点表示：

> 该 Pixel Center（像素中心）在三角形内部。

显示器真正显示时，对应像素会整个显示成一个颜色方块。

于是连续三角形就被表示成一个由离散像素组成的形状。

---

# 32. Jaggies（锯齿）

原本连续的三角形边：

![](../../_Pasted_Image/Pasted%20image%2020260905151252.png)

在离散像素屏幕上可能变成：
![](../../_Pasted_Image/Pasted%20image%2020260905151247.png)


于是斜边看起来像阶梯一样。

这种现象称为：**Jaggies（锯齿）**

---

# 33. Aliasing（走样）

课程进一步将这种现象称为：
![](../../_Pasted_Image/Pasted%20image%2020260905151324.png)
# Aliasing（走样）

目前可以先理解为：

> 由于使用离散采样去表示连续图形，产生了不正确或不理想的视觉结果。

例如：

- 三角形边缘的锯齿

可以用**Antialiasing（抗锯齿）**  来缓解或解决该问题



---

# 34. Lecture 5 的完整主线

整个 Lecture 5 可以压缩成：

```text
MVP 完成
    ↓
Canonical Cube（标准立方体）
    ↓
Viewport Transformation（视口变换）
    ↓
Screen Space（屏幕空间）
    ↓
Pixel（像素）
    ↓
Triangle（三角形）
    ↓
如何把连续三角形变成离散像素？
    ↓
Sampling（采样）
    ↓
在每个像素中心测试 inside()
    ↓
Cross Product（叉积）判断是否在三角形内部
    ↓
得到离散像素
    ↓
出现 Jaggies（锯齿）
    ↓
Aliasing（走样）
```


# 36. 一句话总结 Rasterization（光栅化）

> Rasterization（光栅化）就是把屏幕空间中的连续几何图形，通过在 Pixel Center（像素中心）进行 Sampling（采样），转换成离散 Pixel（像素）表示的过程。

---



## Jaggies（锯齿）

而这种由于 Sampling（采样）造成的错误，更正式地称为：

## Aliasing（走样）

所以 Lecture 6 的第一个核心问题就是：

> **为什么采样会产生 Aliasing（走样）？**

第二个问题：

> **我们怎么避免它？**

整节课的主线其实是：
```
Sampling（采样）
        ↓
为什么会 Sampling Error（采样错误）
        ↓
Frequency（频率）
        ↓
Undersampling（采样不足）
        ↓
Aliasing（走样）
        ↓
Filtering（滤波）
        ↓
先低通滤波，再采样
        ↓
Antialiasing（抗锯齿）
        ↓
Supersampling（超采样）
        ↓
MSAA（多重采样抗锯齿）
```

## Rasterization（光栅化）：采样二维空间
![](../../_Pasted_Image/Pasted%20image%2020260905153234.png)
## Photograph（照片）：采样成像平面
![](../../_Pasted_Image/Pasted%20image%2020260905153227.png)
## Video（视频）：采样时间
![](../../_Pasted_Image/Pasted%20image%2020260905153134.png)


### Jaggies（锯齿）
![](../../_Pasted_Image/Pasted%20image%2020260905153258.png)
### Moiré Pattern（摩尔纹）![](../../_Pasted_Image/Pasted%20image%2020260905153303.png)

#### 然后采样分辨率不够时，就可能出现一些原图根本没有的：

```
波纹
干涉纹
大块纹理
```
它们也是 Aliasing（走样）。

## Wagon Wheel Illusion（车轮效应）

这个特别适合帮助你理解 Aliasing（走样）。
![](../../_Pasted_Image/Pasted%20image%2020260905153542.png)
现实中的轮子其实：
```
不断向前转
```
但是电影中有时会看到：
```
轮子好像倒着转
```

为什么？

因为摄像机不是连续观察，而是在：
```
t0
t1
t2
t3
```
这些离散时刻 Sampling（采样）。

例如一根轮辐：
```
第 1 帧：|
```

转了一大圈之后：
```
第 2 帧：/
```

但你并不知道它真实发生了：
```
| → / → — → \ → | → /
```

还是仅仅：
```
| ← /
```
因此看起来可能像反方向运动。

这就是：

> **时间上的采样频率不够。**


## Aliasing（走样）背后的原因是： 

> **信号变化得太快，而我们的采样速度太慢。**

**Frequency（频率）**

所以以后你不要仅仅记：
```
锯齿 = 像素太大
```

更本质的理解应该是：
> **原始信号里面包含了变化非常快的内容，但当前 Sampling Rate（采样率）不足以正确记录这些变化。**

这就引出了后面的 Frequency Domain（频域）。

## Antialiasing（抗锯齿）的基本思想是先进行 Blurring / Pre-Filtering（模糊 / 预滤波）再采样

![](../../_Pasted_Image/Pasted%20image%2020260905153739.png)

### 为什么先模糊反而能抗锯齿？
原始三角形：

```
        /\
       /  \
      /    \
     /______\
```
边缘处是：
```
突然从红色
↓
变成白色
```
比如：
```
1 1 1 1 1 | 0 0 0 0
```
这个 `1 → 0` 的变化非常突然。

也就是说：
> 信号变化非常快。

后面我们会把它称为：
> **High Frequency（高频）信息。**

如果先稍微 Blur（模糊）：
```
1 1 1 0.8 0.6 0.4 0.2 0 0
```
就不再是：
```
1 → 0
```
突然跳变。

而变成：
```
1
↓
0.8
↓
0.6
↓
0.4
↓
0.2
↓
0
```
变化更加平滑。

这样再采样：
> 就不容易“漏掉”剧烈变化。

所以边缘像素会出现：
```
100% 红
75% 红
50% 红
25% 红
0% 红
```
而不是只有：
```
红
或者
白
```
视觉上自然就平滑了。
![](../../_Pasted_Image/Pasted%20image%2020260905154055.png)

![](../../_Pasted_Image/Pasted%20image%2020260905154106.png)
![](../../_Pasted_Image/Pasted%20image%2020260905154117.png)

#### 错误：
**先采样，已经出现走样，再模糊的结果只是：**
> **Blurred Aliasing（模糊掉的走样）**
![](../../_Pasted_Image/Pasted%20image%2020260905154413.png)

#### 正确
原始连续信号，Filtering（滤波），去掉采样无法表示的部分，Sampling（采样）
这才叫：**Antialiasing（抗走样）**


## Frequency（频率）

**单位范围内变化多少个周期。**

从最简单的：
![](../../_Pasted_Image/Pasted%20image%2020260905155141.png)
开始。
这是周期性变化的函数。
一般形式：
![](../../_Pasted_Image/Pasted%20image%2020260905155155.png)
这里 f 就是频率
![](../../_Pasted_Image/Pasted%20image%2020260905155255.png)

![](../../_Pasted_Image/Pasted%20image%2020260905155319.png)第二个函数变化速度更快。

所以： **频率越高，信号变化得越快。**

## Fourier Transform（傅里叶变换）

一个复杂信号可以被拆成很多不同 Frequency（频率）的正弦和余弦信号。

也就是：
```
复杂函数=低频成分+中频成分+高频成分+……
```

比如你看到一个非常复杂的图像：
```
人物照片
```
我们可以有两种看它的方法。

### Spatial Domain（空间域）

就是： 哪个位置是什么颜色。
也就是普通图片：f(x,y)

### Frequency Domain（频域）

换一种角度问：

这张图片里面有多少：
- 低频变化
- 中频变化
- 高频变化

于是得到：F(w)

![](../../_Pasted_Image/Pasted%20image%2020260905155623.png)
```
Spatial Domain（空间域）
        
Fourier Transform（傅里叶变换）
        
Frequency Domain（频域）

Inverse Fourier Transform（逆傅里叶变换）
```
![](../../_Pasted_Image/Pasted%20image%2020260905162822.png)
## 图像里什么叫低频、高频？

#### 1.整块颜色几乎不变。
这是：**Low Frequency（低频）**

因为：
> 空间位置发生变化时，颜色变化得非常慢。

---
#### 2.颜色疯狂变化。

这是：**High Frequency（高频）**

因为：
> 只移动一点点，颜色就发生剧烈变化。

---

所以图像中的：**大块平坦区域**
通常属于：
> 低频。

**边缘、细节、纹理**
通常拥有很多：
> 高频成分。


这一条 Edge（边缘）：
颜色发生突然变化。
所以包含大量高频内容。

这也解释了：
> 为什么三角形的边最容易出现 Aliasing（走样）。


## 越高频的信号，需要越高的采样速度。
、
假设低频：
```
      /\
     /  \
____/    \____
```

我们采样：
```
•       •       •       •
```
可能还能比较准确知道原函数是什么。

---

但如果原函数高频：
```
/\/\/\/\/\/\/\/\/\/\
```

你仍然只采：
```
•       •       •       •
```

中间发生了什么：
> 你完全不知道。

这就是采样不足。

## Aliasing 名字真正的来源


![](../../_Pasted_Image/Pasted%20image%2020260905160126.png)
图中有：
- 一条高频蓝色曲线
- 一条低频黑色曲线

但是在所有采样点上，它们经过的是完全一样的采样值。因此你只看采样结果，根本分不出来原来到底是哪一个信号。

也就是说：
```
高频信号 A
低频信号 B
```
经过当前采样以后：
```
看起来一模一样
```
这种：
> 在当前 Sampling Rate（采样率）下无法区分的两个频率

称为： **Aliases（混叠频率）**

这就是：Aliasing（走样）

这个词真正的含义。所以 Aliasing 不仅仅是“锯齿”。锯齿只是 Aliasing 的一种表现。

## Filtering（滤波）

Filtering（滤波）就是去掉某些 Frequency（频率）成分。

#### High-Pass Filter（高通滤波）

> 去掉低频，只保留高频。

结果你会发现：
```
人物的大面积颜色没了
```
但是：
```
轮廓
衣服边缘
眼睛
头发边缘
```
还在。
**High-Pass Filter（高通滤波）,非常适合提取边缘。**
![](../../_Pasted_Image/Pasted%20image%2020260905162925.png)
#### Low-Pass Filter（低通滤波）
去掉高频，只留下低频。

结果是 **Blur（模糊）**

人物：
```
边缘
纹理
细节
```
都没了。
只剩下大体颜色变化。

这就是：Low-Pass Filter（低通滤波）

所以现在你终于可以理解前面的：
```
Blur=Low-Pass Filtering=去掉高频信息
```
![](../../_Pasted_Image/Pasted%20image%2020260905162950.png)
#### 十九、这和 Antialiasing 怎么连起来？

前面说：
```
Aliasing 原因：
信号高频太高
采样速度太低
```

那么自然有两个办法。

**方法一**

增加：**Sampling Rate（采样率）**

**方法二**
既然采样速度提不上去, 那就先把采样不了的高频删掉。

也就是：**Low-Pass Filtering（低通滤波）**
然后再采样。
于是：
![](../../_Pasted_Image/Pasted%20image%2020260905162658.png)
就是 Antialiasing（抗走样）的核心思想。


## Convolution（卷积）

并给了一个非常简单的理解：

> Filtering (滤波)= Convolution(卷积) = Averaging(平均)

在这一节可以先近似理解成：
> **卷积就是在一个滑动窗口里面，对附近的数据进行加权平均。**

例如原始信号：

```
1 3 5 3 7 1 3 8 6 4
```

Filter（滤波器）：
```
1/4   1/2   1/4
```
拿最开始三个：
```
1 3 5
```
计算：
![](../../_Pasted_Image/Pasted%20image%2020260905163149.png)
得到：3 

然后窗口向右：
```
3 5 3
```

计算：
![](../../_Pasted_Image/Pasted%20image%2020260905163208.png)

于是不断：
```
滑动
↓
加权平均
↓
滑动
↓
加权平均
```
这就是这里展示的 Convolution（卷积）。![](../../_Pasted_Image/Pasted%20image%2020260905163327.png)


## 为什么平均会导致模糊？

例如像素：
```
0 0 0 255 255 255
```
这里有一个特别锐利的边界。

平均以后可能变成：
```
0 0 64 191 255 255
```

原来的：
```
0 → 255
```

突然跳变，

变成：
```
0
↓
64
↓
191
↓
255
```
变化变平缓。

因此：
> 高频成分减少。

所以：
```
Average（平均）
≈
Blur（模糊）
≈
Low-Pass Filtering（低通滤波）
```

## Convolution Theorem（卷积定理）

记住：
> **空间域中的卷积 = 频域中的乘法。**

也就是说有两种做 Filter（滤波）的方法。

#### 方法 1
直接在 Spatial Domain（空间域）：
```
图像 * Filter Kernel（滤波核）
```
进行 Convolution（卷积）。

#### 方法 2
```
图像
↓
Fourier Transform（傅里叶变换）
↓
频域
↓
乘以 Filter 的频率响应
↓
Inverse Fourier Transform（逆傅里叶变换）
↓
结果
```
数学上两者对应。

## Box Filter（盒式滤波器）

例如：
![](../../_Pasted_Image/Pasted%20image%2020260905163607.png)
什么意思？

就是一个像素新的值：
> 取周围 `3 × 3 = 9` 个像素的平均。

例如：
```
10 20 30
20 30 40
30 40 50
```
![](../../_Pasted_Image/Pasted%20image%2020260905163700.png)

这就是一个非常典型的：**Box Filter（盒式滤波器）**
![](../../_Pasted_Image/Pasted%20image%2020260905163747.png)
**Box Function = Low Pass Filter**也就是说 Box Filter 的效果之一就是去除高频，让图像变平滑。


## 为什么 Sampling 会产生 Aliasing

 **Sampling = Repeating Frequency Contents**
翻译：
在 Frequency Domain（频域）里，采样会让原来的频谱不断重复。

原信号频谱：
```
        /\
_______/  \_______
```
Sampling（采样）以后，会变成：
```
 /\    /\   /\
/  \__/  \_/  \
```
也就是一份一份复制。

如果 Sampling Rate（采样率）很高：

这些复制出来的频谱：
```
 /\          /\          /\
/  \        /  \        /  \
```
彼此距离很远，不会碰到。
所以：
> 没有 Aliasing。


如果 Sampling Rate 太低：
```
   /\  /\
__/  \/  \__
```
复制出来的频谱：
> 发生重叠。

Mixed Frequency Contents（频率内容混在一起）
这就是：**Aliasing（走样）**

所以从频域角度：
![](../../_Pasted_Image/Pasted%20image%2020260905165127.png)

## 怎么降低 Aliasing？

#### Option 1：Increase Sampling Rate（提高采样率）

例如：
```
1920×1080
↓
3840×2160
```
采样点更多。于是频域里的副本离得更远。不容易重叠。

但缺点：
> 贵。

因为：
```
更多像素
更多计算
更多显存
更多带宽
```

---

#### Option 2：Antialiasing

不是让副本离远。

而是：
> 在复制之前，把原始频谱变窄。

怎么变窄？
**Filter Out High Frequencies（滤掉高频）**

所以：
```
原始信号
↓
Low-Pass Filter
↓
频谱变窄
↓
Sampling
↓
复制
↓
副本不再重叠
```

> **Antialiasing = Limiting, then repeating**

可以理解成：
> **先限制频率范围，再采样。**

![](../../_Pasted_Image/Pasted%20image%2020260905165422.png)

## 怎么把这个理论用于三角形光栅化？

理论说：
> 每次采样之前先进行 Low-Pass Filter（低通滤波）。

那三角形怎么办？
> 使用一个大约 **1 Pixel Width（一个像素宽）**的 Box Filter（盒式滤波器）。

也就是说：
> 不要只问“像素中心在不在三角形里面”。

而要问：
> **这个 Pixel（像素）到底有多少面积被 Triangle（三角形）覆盖？**

这是整个抗锯齿里特别重要的一次思想变化。![](../../_Pasted_Image/Pasted%20image%2020260905165601.png)


原来的方法：
```
一个 Pixel：

┌────────┐
│        │
│   ●    │
│        │
└────────┘
```
只看中心 `●`。

如果中心：
```
在三角形内
```
就：
```
100% 红色
```
如果中心：
```
在三角形外
```
就：
```
0% 红色
```
完全不考虑：
> 三角形实际覆盖这个像素多少面积。


**真正理想的答案**
例如：
三角形覆盖：
```
75%
```
那么这个 Pixel 的值应该大约：
```
75% 红
```
如果覆盖：
```
25%
```
则：
```
25% 红
```
 对 `inside(triangle,x,y)` 在一个 Pixel Area（像素区域）内求平均，其平均值就等于：
**三角形覆盖这个 Pixel 的面积比例。**

#### 但是精确求覆盖面积很麻烦
理想情况是：
> 对每个 Pixel 精确计算 Triangle 覆盖了多少面积。

但是这样比较贵。
于是引出一个实际方法：**Supersampling（超采样）**

这里把它和**MSAA（多重采样抗锯齿）** 联系起来。

#### Supersampling（超采样）

核心思想：
> 不再一个 Pixel 只采一个点。

而是：
> **在一个 Pixel 内采多个点。**

例如：
# 4×4 Supersampling

一个 Pixel 内：
```
• • • •
• • • •
• • • •
• • • •
```
一共：4 * 4=16个 Sample（采样点）。

#### 为什么多个采样点可以近似面积？

例如一个 Pixel：
```
┌─────────┐
│ ● ● ○ ○ │
│ ● ● ○ ○ │
│ ● ● ○ ○ │
│ ○ ○ ○ ○ │
└─────────┘
```

一共：\[ 16 \]个点。其中：\[ 6 \]个在三角形内。那么估计覆盖率：37.5%
于是这个 Pixel：
```
37.5% 红
```
我们虽然没有真正计算几何面积，

但是：
> 通过多个 Sample（采样点）的统计比例，近似覆盖面积。

**这就是 Supersampling 的思想。**
![](../../_Pasted_Image/Pasted%20image%2020260905170150.png)
![](../../_Pasted_Image/Pasted%20image%2020260905170202.png)
![](../../_Pasted_Image/Pasted%20image%2020260905170208.png)
![](../../_Pasted_Image/Pasted%20image%2020260905170217.png)

![](../../_Pasted_Image/Pasted%20image%2020260905170236.png)![](../../_Pasted_Image/Pasted%20image%2020260905170243.png)
### 效果对比
![](../../_Pasted_Image/Pasted%20image%2020260905170307.png)
![](../../_Pasted_Image/Pasted%20image%2020260905170324.png)

**但是这样会使计算成本也增加**

因此现实中又出现：
- FXAA（快速近似抗锯齿）
- TAA（时间抗锯齿）
- DLSS（深度学习超级采样）