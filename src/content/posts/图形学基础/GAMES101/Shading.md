---
title: Shading
date: 2026-09-06
tags: []
draft: false
---

# 📝 GAMES101 第7课笔记：着色 1（Shading 1）

## 📌 一、 深度测试与可见性（Visibility / Occlusion）

在计算机图形学中，我们需要解决“哪些物体在前面、哪些被挡住了”的问题（即遮挡/可见性问题）。


### 1. 画家算法（Painter's Algorithm）


- **原理**：模仿画家作画，**从远到近**依次绘制物体，远的会被近的覆盖。
- **缺点**：
    - **计算复杂度高**：需要对所有三角形按深度排序，算法复杂度为 $O(n \log n)$。
        
    - **无法解决循环遮挡**：例如三个三角形互相重叠覆盖（A挡B、B挡C、C挡A），此时无法按顺序排序。

### 2. Z-Buffer / Depth Buffer（深度缓存算法）

- **地位**：当前图形学中最主流、被 GPU 硬件广泛支持的深度算法。
- **基本概念**：
    - **Frame Buffer（帧缓存）**：用于存储每个像素的颜色信息。
    - **Depth Buffer / Z-Buffer（深度缓存）**：用于存储每个像素（采样点）当前最小的深度值 $z$。
        
    - 注：这里假设 $z$ 值恒为正数，**$z$ 值越小代表距离相机越近**，越大代表越远。
        
- **算法逻辑**：
    
      
    1. 将深度缓存中的所有像素初始化为无穷大（$\infty$）。
    2. 在光栅化过程中，遍历每个三角形及其包含的像素/采样点 $(x, y)$：
        - 如果当前点的 $z < \text{zbuffer}[x,y]$（说明当前点更靠前）：
            - 更新颜色：$\text{framebuffer}[x,y] = \text{rgb}$
            - 更新深度：$\text{zbuffer}[x,y] = z$
        - 否则：什么都不做（说明被挡住了）。
            
- **复杂度与特点**：
    - 时间复杂度为 $O(n)$（$n$ 为三角形数量）。
    - 与绘制三角形的顺序无关。
  

  

## 📌 二、 着色基础（Shading）

### 1. 什么是着色（Shading）？

- **定义**：对物体应用材质（Material）的过程。
    
- **核心性质**：**着色是局部计算（Shading is Local）**。
    - 计算某个着色点（Shading point）的反射光时，只考虑它自身的位置、法线、光线和视角。
        
    - **着色不等同于阴影（Shading $\neq$ Shadow）**，光靠着色本身**不会生成阴影**（阴影需要单独计算遮挡）。
  
### 2. 着色点（Shading Point）的基础向量

计算某个点的着色时，需要用到以下单位向量：

- **$\mathbf{v}$（Viewer direction / 视角方向）**：从着色点指向摄像机/观察者。
    
- **$\mathbf{n}$（Surface normal / 表面法线）**：垂直于着色点表面的单位向量。
    
- **$\mathbf{l}$（Light direction / 光照方向）**：从着色点指向光源。
    
- **Surface parameters（表面材质参数）**：如颜色、光滑度等。

## 📌 三、 布林-冯反射模型：漫反射（Blinn-Phong Model: Diffuse）

Blinn-Phong 模型包含三部分：漫反射（Diffuse）、高光（Specular）和环境光（Ambient）。本节重点讨论**漫反射**。
### 1. 漫反射的定义（Diffuse Reflection）

- 光线照射到物体表面后，向**所有方向均匀散射**。
- 因此，**从任何角度观察，漫反射看到的颜色都是相同的**（与视角 $\mathbf{v}$ 无关）。

### 2. 朗伯余弦定律（Lambert's Cosine Law）

- **概念**：着色点单位面积接收到的光能，与**光线方向 $\mathbf{l}$ 和法线方向 $\mathbf{n}$ 的夹角 $\theta$ 的余弦值**成正比。
- **公式表示**：$\cos \theta = \mathbf{n} \cdot \mathbf{l}$
    - 当光线垂直照射表面时（$\theta = 0^\circ$），$\cos \theta = 1$，表面接收到的能量最多。
    - 当光线倾斜（如 $60^\circ$），接收到的能量减半。

### 3. 光的衰减（Light Falloff）

- 点光源向四周辐射能量，光强（Intensity）随距离 $r$ 的平方成反比衰减：
    $$\text{距离 } r \text{ 处的实际光强} = \frac{I}{r^2}$$
### 4. 漫反射计算公式（Lambertian Shading）

$$L_d = k_d \left( \frac{I}{r^2} \right) \max(0, \mathbf{n} \cdot \mathbf{l})$$

- **参数解析**：
    - $L_d$：漫反射反射出的光能（Diffuse reflected light）。
        
    - $k_d$：漫反射系数 / 材质颜色（Diffuse coefficient）。如果 $k_d = 1$ 代表完全反射（白色），$k_d = 0$ 代表吸收所有光（黑色）。
        
    - $\frac{I}{r^2}$：到达着色点时的实际光强。
        
    - $\max(0, \mathbf{n} \cdot \mathbf{l})$：着色点接收到的能量比例。
        - 用 $\max(0, \cdot)$ 是因为当光从物体背面照过来时（$\mathbf{n} \cdot \mathbf{l} < 0$），漫反射能量没有物理意义，直接取 $0$。

![](../../_Pasted_Image/Pasted%20image%2020260906121138.png)![](../../_Pasted_Image/Pasted%20image%2020260906121341.png)

# Shading 2 (着色、管线与纹理映射)

## 1. Blinn-Phong 反射模型 (Blinn-Phong Reflectance Model)

Blinn-Phong 模型将点光源作用下的光线反射分为三部分：**漫反射 (Diffuse)**、**高光 (Specular)** 和 **环境光 (Ambient)**。

### 1.1 高光项 (Specular Term)
当观察方向 $\mathbf{v}$ 接近镜面反射方向 $\mathbf{R}$ 时，会产生高光。Blinn-Phong 模型通过计算**法线向量 $\mathbf{n}$** 与 **半程向量 (Half Vector) $\mathbf{h}$** 的接近程度来进行简化计算。

* **半程向量定义**：
  $$ \mathbf{h} = \frac{\mathbf{v} + \mathbf{l}}{\|\mathbf{v} + \mathbf{l}\|} $$
* **高光计算公式**：
  $$ L_s = k_s \left( \frac{I}{r^2} \right) \max(0, \mathbf{n} \cdot \mathbf{h})^p $$
  * $k_s$：高光系数 (Specular Coefficient)。
  * $p$：高光指数 (Cosine Power)，用于控制高光斑点的大小。$p$ 越大，高光范围越小、越集中；通常 $p$ 取值在 100~200 左右。
![](../../_Pasted_Image/Pasted%20image%2020260906121711.png)
![](../../_Pasted_Image/Pasted%20image%2020260906121728.png)
#### 为什么法线 $\mathbf{n}$ 与半程向量 $\mathbf{h}$ 的点积可以来评估观察方向与反射方向的接近程度
我们可以把视线向量 $\mathbf{v}$、反射向量 $\mathbf{R}$、入射光线向量 $\mathbf{l}$ 和法线向量 $\mathbf{n}$ 放在同一个平面上来观察：
- **完全重合时**：根据反射定律，如果观察方向 $\mathbf{v}$ **正好**沿着反射方向 $\mathbf{R}$，那么入射光线 $\mathbf{l}$ 与视线 $\mathbf{v}$ 的角平分线（即半程向量 $\mathbf{h}$）就会**恰好**与法线 $\mathbf{n}$ 完全重合！此时 $\mathbf{n} \cdot \mathbf{h} = 1$。
- **发生偏离时**：当视线 $\mathbf{v}$ 逐渐偏离反射方向 $\mathbf{R}$ 时，半程向量 $\mathbf{h}$ 也会随之偏离法线 $\mathbf{n}$。偏离的角度越大，$\mathbf{n}$ 和 $\mathbf{h}$ 之间的夹角就越大，它们的点积（余弦值）就会变小。
因此，$\mathbf{n} \cdot \mathbf{h}$ 的大小直接反映了 $\mathbf{v}$ 与 $\mathbf{R}$ 的接近程度。

#### 💡 为什么 Blinn-Phong 选择用 $\mathbf{n} \cdot \mathbf{h}$ 替代传统 Phong 模型中的 $\mathbf{R} \cdot \mathbf{v}$？

在传统的 **Phong 反射模型**中，计算高光直接使用的是反射向量与视线的点积 $\mathbf{R} \cdot \mathbf{v}$。但计算反射向量 $\mathbf{R}$ 的公式相对复杂：
$$\mathbf{R} = 2(\mathbf{n} \cdot \mathbf{l})\mathbf{n} - \mathbf{l}$$

而在 **Blinn-Phong 模型**中，计算半程向量 $\mathbf{h}$ 只需要简单的向量加法和归一化：

$$\mathbf{h} = \frac{\mathbf{v} + \mathbf{l}}{\Vert{}\mathbf{v} + \mathbf{l}\Vert{}}$$

在大规模并行计算的 GPU 中，计算 $\mathbf{h}$ 比计算 $\mathbf{R}$ **省时得多**，而且两者的效果在视觉上非常接近！
### 1.2 环境光项 (Ambient Term)
用于模拟全局光照（多重漫反射），避免处于阴影中的物体完全变黑。
* **环境光公式**：
  $$ L_a = k_a I_a $$
  * $k_a$：环境光反射系数。
  * $I_a$：环境光强度。

### 1.3 完整 Blinn-Phong 模型公式
$$ L = L_a + L_d + L_s = k_a I_a + k_d \left( \frac{I}{r^2} \right) \max(0, \mathbf{n} \cdot \mathbf{l}) + k_s \left( \frac{I}{r^2} \right) \max(0, \mathbf{n} \cdot \mathbf{h})^p $$

![](../../_Pasted_Image/Pasted%20image%2020260906122724.png)


---

## 2. 着色频率 (Shading Frequencies)

根据在何处计算光照，着色频率主要分为以下三种：

| 着色类型 | 计算单位 | 计算方法 | 特点 / 适用场景 |
| :--- | :--- | :--- | :--- |
| **Flat Shading (平面着色)** | 三角形面 | 每个三角形计算一次法线和光照，整个面颜色一致。 | 简单快速，但对于平滑表面效果较差，会有明显的面片感。 |
| **Gouraud Shading (顶点着色)** | 顶点 | 计算每个顶点的法线与光照，在三角形内部**插值颜色**。 | 计算量适中，但在高光处理上可能会有失真或丢失高光的情况。 |
| **Phong Shading (像素着色)** | 像素/片段 | 在三角形内部**插值法线**，对每个像素单独计算光照。 | 效果最平滑逼真，但计算开销最大。（注意：区别于 Blinn-Phong 反射模型） |
![](../../_Pasted_Image/Pasted%20image%2020260906123009.png)
### 2.1 顶点法线计算 (Per-Vertex Normals)
若没有几何体的底层法线，通常采用**相邻三角形面法线的加权平均**来计算顶点法线：
$$ \mathbf{N}_v = \frac{\sum_{i} \mathbf{N}_i}{\|\sum_{i} \mathbf{N}_i\|} $$
![](../../_Pasted_Image/Pasted%20image%2020260906123116.png)
---

## 3. 实时渲染管线 (Graphics Pipeline)

实时渲染管线是将 3D 场景转化为 2D 屏幕图像的完整硬件/软件流程：

1. **Vertex Processing (顶点处理)**：执行 MVP 变换（Model, View, Projection），将顶点坐标变换到屏幕空间。
2. **Triangle Processing (三角形处理)**：将顶点组装成三角形网格。
3. **Rasterization (光栅化)**：确定哪些像素/采样点被三角形覆盖，生成片段流 (Fragments)。
4. **Fragment Processing (片段处理)**：执行 Z-Buffer 深度测试，计算着色（Shading）、纹理采样（Texture Mapping）等。
5. **Framebuffer Operations (帧缓冲区操作)**：输出最终图像到屏幕显示。

> **Shader (着色器)**：运行在 GPU 上的自定义可编程程序，分为顶点着色器 (Vertex Shader) 和片段/像素着色器 (Fragment/Pixel Shader)，用于控制顶点变换和像素颜色计算。
![](../../_Pasted_Image/Pasted%20image%2020260906123200.png)
---

## 4. 纹理映射 (Texture Mapping)

* **概念**：将 2D 的纹理图像“粘贴”到 3D 物体表面，以定义物体表面不同位置的属性（如颜色 $k_d$、粗糙度等）。
* **纹理坐标 $(u, v)$**：定义在 2D 纹理空间上的坐标，取值范围通常为 $[0, 1] \times [0, 1]$。
* **映射过程**：三角形的每个顶点都绑定有一个 $(u, v)$ 坐标，在渲染像素时，通过**重心坐标插值 (Barycentric Interpolation)** 计算出像素对应的纹理坐标，从而查找到对应的纹理颜色。

> 纹理映射：着色模型确定了吸收的能量，但不同像素点有不同的属性（比如漫反射系数）
![](../../_Pasted_Image/Pasted%20image%2020260906123950.png)

- Surfaces are 2D：纹理就是一张图，蒙在3D物体表面
![](../../_Pasted_Image/Pasted%20image%2020260906124010.png)

- Texture Applied to Surface（如何对应：美工设计 | 自动化参数化映射）
![](../../_Pasted_Image/Pasted%20image%2020260906124033.png)



- 纹理坐标系：$uv$都是$[0,1]$
![](../../_Pasted_Image/Pasted%20image%2020260906124103.png)

- 不同地方可以使用相同的纹理（下图重复同一纹理），如何设计使纹理可以无缝链接呢？
![](../../_Pasted_Image/Pasted%20image%2020260906124117.png)


# Shading 3 (纹理映射续与范围查询)

## 📚 目录
1. 三角形内部插值：重心坐标 (Barycentric Coordinates)
2. 纹理放大与缩小问题 (Texture Magnification & Minification)
3. 纹理范围查询与 Mipmap 机制
4. 各向异性过滤 (Anisotropic Filtering)

---

## 1. 三角形内部插值：重心坐标 (Barycentric Coordinates)

### 1.1 为什么需要插值？
在光照与着色过程中，我们通常只知道三角形三个顶点上的属性（如 3D 坐标、法线、颜色、纹理坐标 $(u,v)$、深度等）。为了在三角形内部呈现平滑过渡的视觉效果，需要将顶点属性平滑插值到三角形内部的任意一点[cite: 3]。

### 1.2 重心坐标的定义
在三角形 $ABC$ 所在的平面内，任意一点 $(x, y)$ 都可以表示为三个顶点坐标的线性组合[cite: 3]：

$$ (x, y) = \alpha A + \beta B + \gamma C $$

其中满足归一化条件：
$$ \alpha + \beta + \gamma = 1 $$

* **内部点判定**：当且仅当 $\alpha \ge 0, \beta \ge 0, \gamma \ge 0$ 时，点 $(x, y)$ 位于三角形内部[cite: 3]。
![](../../_Pasted_Image/Pasted%20image%2020260906130540.png)
### 1.3 几何意义：面积比法
重心坐标 $(\alpha, \beta, \gamma)$ 可以通过子三角形的面积占比来直观理解[cite: 3]：

* $\alpha = \frac{A_A}{A_A + A_B + A_C}$ （$\alpha$ 对应顶点 $A$ 相对的子三角形 $A_A$）[cite: 3]
* $\beta = \frac{A_B}{A_A + A_B + A_C}$[cite: 3]
* $\gamma = \frac{A_C}{A_A + A_B + A_C}$[cite: 3]

> 💡 **特例说明**：
> * 顶点 $A$ 本身的重心坐标为 $(1, 0, 0)$[cite: 3]。
> * 三角形重心 (Centroid) 的重心坐标为 $(\frac{1}{3}, \frac{1}{3}, \frac{1}{3})$[cite: 3]。
![](../../_Pasted_Image/Pasted%20image%2020260906130600.png)
### 1.4 常用计算公式
已知点 $(x, y)$ 及顶点 $A, B, C$ 的坐标，可通过下式直接求解 $\alpha, \beta, \gamma$[cite: 3]：

$$ \alpha = \frac{-(x-x_B)(y_C-y_B) + (y-y_B)(x_C-x_B)}{-(x_A-x_B)(y_C-y_B) + (y_A-y_B)(x_C-x_B)} $$

$$ \beta = \frac{-(x-x_C)(y_A-y_C) + (y-y_C)(x_A-x_C)}{-(x_B-x_C)(y_A-y_C) + (y_B-y_C)(x_A-x_C)} $$

$$ \gamma = 1 - \alpha - \beta $$

### 1.5 属性插值计算
设顶点 $A, B, C$ 上的某属性分别为 $V_A, V_B, V_C$（如颜色、法线、纹理坐标等），则内部点的属性 $V$ 为[cite: 3]：

$$ V = \alpha V_A + \beta V_B + \gamma V_C $$

⚠️ **重点注意**：**重心坐标在透视投影下不能保证重心坐标不变**[cite: 3]！因此在三维空间投影到二维屏幕后，深度值或纹理坐标的插值必须使用**透视矫正插值 (Perspective-Correct Interpolation)**，即先在三维空间中计算插值或对 $1/z$ 进行插值，不能直接在二维屏幕空间做简单的线性重心插值[cite: 3]。

---

## 2. 纹理查询与放大问题 (Texture Magnification)

在将纹理贴图应用到物体表面时，屏幕像素 (Pixel) 与纹理像素 (Texel, 纹素) 的分辨率往往无法一一对应[cite: 3]。

### 2.1 纹理太小（Texture Magnification - 纹理放大）
当纹理分辨率低于屏幕显示区域时，多个屏幕像素会映射到同一个纹素上，导致画面出现锯齿或块状感[cite: 3]。

常用解决办法[cite: 3]：
1. **Nearest (邻近采样)**：直接取最近的纹素颜色。速度最快，但块状锯齿明显[cite: 3]。
2. **Bilinear (双线性插值)**：取目标点四周最近的 4 个纹素点，先在水平方向做两次线性插值 (Lerp)，再在垂直方向做一次线性插值[cite: 3]。
   * 单维线性插值： $lerp(x, v_0, v_1) = v_0 + x(v_1 - v_0)$[cite: 3]
   * 结果更加平滑，是实时渲染中最常用的折中方案[cite: 3]。
3. **Bicubic (双三次插值)**：利用四周 $4 \times 4$ 共 16 个纹素进行三次插值，效果最好，但计算开销较高[cite: 3]。
![](../../_Pasted_Image/Pasted%20image%2020260906133224.png)
---

## 3. 纹理缩小与范围查询 (Texture Minification & Mipmap)

### 3.1 纹理太大（Texture Minification - 纹理缩小）
当纹理分辨率远高于屏幕显示区域（例如远处地面）时，一个屏幕像素在纹理空间中覆盖了很大一片区域（Footprint）[cite: 3]。

* **点采样的问题**：如果在这一大片区域内只做点采样，会导致严重的**摩尔纹 (Moiré pattern)** 和 **锯齿 (Jaggies)**[cite: 3]。
 ![](../../_Pasted_Image/Pasted%20image%2020260906133340.png)
* **超采样 (Supersampling)**：如 512x 超采样虽能解决问题，但开销极大[cite: 3]。
![](../../_Pasted_Image/Pasted%20image%2020260906133421.png)
* **核心思路转变**：从**点查询 (Point Query)** 转变为**范围平均值查询 (Range Query)**[cite: 3]。我们需要快速知道“纹理上某块区域的平均颜色”[cite: 3]。
![](../../_Pasted_Image/Pasted%20image%2020260906133524.png)
### 3.2 Mipmap 机制
Mipmap 是一种提供**快速、近似、正方形区域**平均值查询的技术[cite: 3]。

* **层级结构**：
  * Level 0：原始图像（高分辨率）[cite: 3]。
  * Level $d+1$：由 Level $d$ 宽高各缩小一半（分辨率变为 $1/4$）得到[cite: 3]。
* **额外内存开销**：由于等比数列求和 $\frac{1}{4} + \frac{1}{16} + \dots = \frac{1}{3}$，Mipmap 仅增加 **33.3% ($1/3$)** 的额外存储空间[cite: 3]。

### 3.3 Mipmap 层级 $D$ 的计算
如何确定某个屏幕像素应该查询 Mipmap 的第几层？

1. 取当前屏幕采样点 $(x, y)$ 及其邻近点 $(x+1, y)$、$(x, y+1)$，分别计算它们对应的纹理坐标 $(u, v)$[cite: 3]。
2. 计算在纹理空间中的变化率/微分（微分步长）[cite: 3]：
   $$ L = \max\left( \sqrt{\left(\frac{du}{dx}\right)^2 + \left(\frac{dv}{dx}\right)^2}, \sqrt{\left(\frac{du}{dy}\right)^2 + \left(\frac{dv}{dy}\right)^2} \right) $$
3. 计算对应的 Mipmap 层级 $D$[cite: 3]：
   $$ D = \log_2 L $$

### 3.4 三线性插值 (Trilinear Interpolation)
如果计算出的层级 $D$ 是连续的浮点数（如 $D = 1.8$），直接取整会导致不同层级交界处出现明显的断层缝隙[cite: 3]。

**三线性插值**的做法[cite: 3]：
1. 在 Level $\lfloor D1 \rfloor$ 层上做一次**双线性插值**，得到颜色 $C_1$[cite: 3]。
2. 在 Level $\lceil D2 \rceil$ 层上做一次**双线性插值**，得到颜色 $C_2$[cite: 3]。
3. 根据 $D$ 的小数部分，对 $C_1$ 和 $C_2$ 进行一次**线性插值**[cite: 3]。
* 最终在层与层之间实现了连续平滑的过渡[cite: 3]。

---

## 4. 各向异性过滤 (Anisotropic Filtering)

### 4.1 Mipmap 的局限性：过度模糊 (Overblur)
Mipmap 假设屏幕像素在纹理空间中的映射区域是**正方形**的[cite: 3]。但是在透视投影下（如倾斜看地面），屏幕像素映射到纹理空间往往是拉长的**矩形**或**不规则多边形**[cite: 3]。

如果强行用包含该矩形的大正方形去查找 Mipmap，会导致包含了过多无关区域的颜色，从而造成远处画面**过度模糊 (Overblur)**[cite: 3]。
![](../../_Pasted_Image/Pasted%20image%2020260906141858.png)
### 4.2 各向异性过滤 (Anisotropic Filtering / Ripmap)
* **核心原理**：不仅预计算正方形压缩图像，还预计算轴向拉伸（如长宽比为 2:1、4:1 等）的**矩形**区域图像[cite: 3]。
* **优点**：能够精准匹配轴向拉伸的矩形采样区域，大幅改善倾斜观察视角下的纹理清晰度[cite: 3]。
* **缺点/局限**：只能很好地处理水平或垂直拉伸的矩形，对于对角线方向（斜向）拉伸的区域依然力不从心[cite: 3]。

### 4.3 补充：EWA 过滤 (EWA Filtering)
* 使用多次查找与加权平均（Weighted average），将不规则区域拆解成多个圆形区域进行采样叠加[cite: 3]。
* 能够完美处理任意形状的复杂采样区域，但代价是多次纹理查询带来的计算开销增高[cite: 3]。
 ![](../../_Pasted_Image/Pasted%20image%2020260906141948.png)
## Applications of Textures
纹理不仅仅可以给物体表面贴个图，还有其他的应用。

### **Environment Map**
可以使用纹理来表示环境光，如图：
![](../../_Pasted_Image/Pasted%20image%2020260907145521.png)

### **Bump Mapping**
#### **凹凸贴图（bump mapping）** 
可以记录物体表面相对高度，从而影响物体表面法线，进而影响阴影，给人凹凸感觉。不过物体表面实际上还是光滑的，凹凸只是纹理带来的错觉。
简单理解：物体几何表面其实没有真的凹凸，只是骗光照系统：“这里的表面方向变了”，于是亮暗发生变化，人眼就误以为表面凹凸不平。
![](../../_Pasted_Image/Pasted%20image%2020260907145603.png)
图里真正的逻辑就是：
```
Height（高度）
    ↓
Normal（法线）
    ↓
Shading（着色）
    ↓
看起来有凹凸
```
##### 先看一个完全光滑的球
假设一个球表面本来非常光滑：
```
      ↑
   ↖  ↑  ↗
     球面
```
每个位置都有自己的 Normal（法线）。
而我们前面学过 Blinn-Phong 时，亮不亮和法线有关，比如 Diffuse（漫反射）里有：
![](../../_Pasted_Image/Pasted%20image%2020260907150519.png)
所以：
> **法线方向一变，光照结果就会变。**

---
###### 现在我不改球，只偷偷改法线
原来某一小块表面其实是平的：
```
真实表面：

────────────
      ↑
      n
```
但 Bump Mapping 告诉 Shading：
```
你别用原来的 ↑

你用这个 ↖
```
于是光照系统会以为：
```
这里表面是斜的
```
然后旁边另一个位置可能使用：
```
↗
```
结果就变成：
```
↖  ↑  ↗  ↘  ↑  ↖
```
虽然真实几何还是：
```
──────────────
```
但是计算光照时，各个位置使用的法线乱七八糟地发生变化。

于是：
```
有的地方更亮
有的地方更暗
```
人眼看到这种连续的明暗变化，就会认为：
> “这里好像有坑、这里好像凸起来了。”

这就是所谓的：
**Fake the detailed geometry（伪造几何细节）**。

---
##### 那 Height Map（高度图）干什么？
你图左下角的纹理不是简单拿来当颜色贴到橘子上的。
它记录的是：
> **每个位置理论上应该有多高。**

比如可以想成：
```
白色 = 高
灰色 = 中
黑色 = 低
```
假设一排高度是：
```
1   1   2   4   3   1
```
它说明这里如果真的有凹凸，大概应该是：
```
        /\
       /  \
______/    \____
```
但是 Bump Mapping **不会真的把表面做成这样**。

它只是看：
```
高度有没有变化？
```

例如：
```
1 → 4
```
说明这里高度快速上升。

于是推断：
> 这里表面应该向上倾斜。

再根据这个斜率，算一个新的 Normal（法线）。
所以：
```
Height Map
    ↓
计算高度变化
    ↓
推断表面倾斜程度
    ↓
修改 Normal
    ↓
重新算光照
```

---
##### 用橘子最容易理解
真实模型可能只是一个很光滑的球：
```
Geometry：

       ______
     /        \
    |          |
     \________/
```
但你希望它像橘子皮一样：
```
• •  • • •
 • • •  •
•  • • •
```
方案一是：
> 真把这些坑坑洼洼全部建成几何。

那需要大量 Triangle（三角形）。
很贵。
Bump Mapping 的办法是：
> 球还是那个球，我只在每个位置修改法线。

于是：
```
实际几何：
光滑球

Shading 使用的法线：
↗ ↑ ↙ ↖ → ↘ ↑ ...
```
渲染出来以后，就变成你 PPT 右下角那个橘子：
> **看起来坑坑洼洼。**

但如果你真的从侧面特别仔细看它的 Silhouette（轮廓），会发现：
> 轮廓其实还是光滑的。

因为 Geometry 根本没有改变。

---
##### 这也是 Bump Mapping 最重要的局限

假设你做一个砖墙。
Bump Mapping 可以让正面看起来：
```
砖块凸起
砖缝凹下去
```
但从侧面看：
```
真实轮廓依然是：

────────────
```
而不是：
```
真实砖块轮廓：

__    __    __
  |__|  |__|
```
因为它只是：
> **修改 Normal，没修改 Vertex（顶点）。**

这就是为什么下一页 GAMES101 会马上讲：
**Displacement Mapping（位移贴图）**
它与 Bump Mapping 最大区别就是：
```
Bump Mapping
只改法线
Geometry 不变

Displacement Mapping
真的移动顶点
Geometry 真变了
```

---

> Relative height to the underlying surface

理解成：
> **这张纹理记录“假设表面真的有凹凸的话，每个位置相对于原始表面应该高多少或低多少”。**

然后：
```
Relative Height（相对高度）
        ↓
算出 Surface Slope（表面斜率）
        ↓
算新的 Normal（法线）
        ↓
带入光照公式
        ↓
不同地方亮暗不同
        ↓
人眼感觉有凹凸
```

最关键的一点是：
> **Bump Mapping 不是“用高度贴图把模型顶起来”，而是“利用高度变化推算一个假的法线，再用这个假的法线做光照”。**

你把这一句话理解了，这一页就基本通了。
#### **位移贴图（displacement mapping）** 
则真正移动了3Dmesh的顶点，从而有更真实的阴影效果。但是要求物体的3D mesh要足够细，才能支持这个操作。

#### **3D Procedural Noise**
似乎是引入噪声，来生成3维纹理（譬如大理石纹路之类的）。

#### **Provide Precomputed Shading**
就是在纹理上已经做好shading了

#### **3D Textures**
纹理可以不只是二维表面的，也可以是三维空间的。还可以做**体渲染（volume rendering）**。譬如CT扫描的结果。

