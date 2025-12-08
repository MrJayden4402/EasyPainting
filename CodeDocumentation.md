# EasyPainting 10.0 代码文档

## 1. 库概述

 
该库是一个基于DirectX的图形库，提供了一些简化接口，可以用于简单的绘图/渲染。

声明包含在 EasyPainting.h 中。\
实现包含在 EasyPainting.cpp 中。

实现中包含一些以__Easy_开头的函数，这些函数是库的内部函数，除非你知道你在做什么，否则你不应该直接调用它们。

使用这个库需要链接一系列lib，具体为加入以下参数：

- -ld2d1
- -ld3d11
- -ldwrite
- -lole32
- -lwindowscodecs
- -lshlwapi
- -luuid

还需要加入-mwindows编译选项。


## 2. 库函数/类

### 2.1 EasyPaintingStart 函数

函数头部:
```cpp
void EasyPaintingStart(HWND window, int WindowWidth, int WindowHeight, EasyPixel BackColor);
```
用于初始化EasyPainting库。\
**HWND window** 窗口句柄。代表需要渲染到哪个窗口上\
**int WindowWidth** 渲染缓冲区宽度\
**int WindowHeight** 渲染缓冲区高度\
**EasyPixel BackColor** 背景颜色，每帧渲染都会以这个颜色做底色

渲染缓冲区:\
EasyPainting内部有一个渲染缓冲区，大小是固定的，你的渲染将在这个缓冲区上进行，最后输出到窗口的时候会进行拉伸，以适应窗口大小。

可以全部都传入NULL，表示无需渲染窗口，此时EasyPainting库将不会创建渲染缓冲区，但会进行一些初始化工作。

### 2.2 DrawStart/DrawEnd 函数

函数头部:
```cpp
void DrawStart();
void DrawEnd(bool enableVSync = true);
```
用于开始/结束绘制。\
调用DrawStart后，可以调用一系列绘图函数，调用DrawEnd后，EasyPainting会将渲染缓冲区的内容全部输出到窗口上。

其中DrawEnd的**enableVSync**参数用于开启/关闭垂直同步，默认开启。

它们必须成对出现，否则会出现未定义行为。

### 2.3 EasySurface(旧名:SURFACE) 类

该类用于表示一个表面(位图)，可以将它渲染到缓冲区中。

#### 2.3.1 构造

构造方式有三种：\
第一种，使用构造函数

头部
```cpp
SURFACE::SURFACE(string filename, int width, int height, EasyPixel MatteColor, ID2D1DeviceContext **pRenderTarget)
```

参数\
**string filename** 加载位图的路径。
**int width** 位图的宽度，如果为0，则自动获取位图的宽度\
**int height** 位图的高度，如果为0，则自动获取位图的高度\
**EasyPixel MatteColor** 透明色，如果位图中有这个颜色，则会被渲染为透明\
**ID2D1DeviceContext \*\*pRenderTarget** 渲染目标，可以指定EasyBuffer，这个稍后会讲到，默认为主缓冲区。

第二种，使用Create成员函数\
它的参数和构造函数完全相同，不介绍。

第三种，使用CreateFromMemory成员函数\
这种构造请务必在EasyPaintingStart之后调用。
头部如下：
```cpp
void CreateFromMemory(vector<vector<EasyPixel>> &vec);
```
将vector的内容作为位图渲染。


注意在EasyPaintingStart之前构造的，它们会在EasyPaintingStart时全部统一加载。

#### 2.3.2 渲染

方式一:\
调用DrawItEx成员函数。\
这个的功能是最完全的。

头部
```cpp
void DrawItEx(int x, int y, int width, int height, int imagex, int imagey, int imagewidth, int imageheight, float rotation = 0, int midpointdx = 0, int midpointdy = 0, int reverse = 0);
```
**int x** 渲染的x坐标\
**int y** 渲染的y坐标\
**int width** 渲染的宽度\
**int height** 渲染的高度\
**int imagex** 位图裁剪的x坐标\
**int imagey** 位图裁剪的y坐标\
**int imagewidth** 位图裁剪的宽度\
**int imageheight** 位图裁剪的高度\
**float rotation** 旋转角度\
**int midpointdx** 旋转中心x偏移\
**int midpointdy** 旋转中心y偏移\
**int reverse** 0为正常\
传入EASY_TURNLR表示左右翻转，\
传入EASY_TURNUD表示上下翻转。\
传入EASY_TURNLR|EASY_TURNUD表示左右上下都翻转。

这个函数会从原图裁剪出指定区域，进行旋转，$x$，$y$偏移量代表与图像中心的偏移量，再进行镜像翻转，最后以指定坐标，拉伸到指定宽高进行渲染。



方式二:\
调用DrawIt成员函数，它的易用性相对DrawItEx比较强。

头部
```cpp
void DrawIt(int x, int y, int width, int height, float scaling = 1, float rotation = 0, int columns = 1, int frames = 0, int reverse = 0);
```

其中功能很多。

**int x** 渲染的x坐标\
**int y** 渲染的y坐标\
**int width** 渲染位图的宽度，若为0则使用位图的原始宽度\
**int height** 渲染位图的高度，若为0则使用位图的原始高度\
此处宽高若小于源位图，则会被裁切。若大于源位图，则会被拉伸。\
**float scaling** 缩放比例，默认为1，这个缩放比例是宽，高会被同时拉伸的\
**float rotation** 顺时针旋转角度，默认为0\
若位图使用了动画帧，可以使用如下两个参数来绘制。\
**int columns** 每行有多少帧，默认为1\
**int frames** 帧数，表示渲染第几帧，默认为0\
还可以对位图进行镜像翻转。\
**int reverse** 0为正常\
传入EASY_TURNLR表示左右翻转，\
传入EASY_TURNUD表示上下翻转。\
传入EASY_TURNLR|EASY_TURNUD表示左右上下都翻转。


还有DrawItDirect/DrawItFrames/DrawItReverse等函数，都是DrawIt的简化版，对应参数填入即可。

#### 2.3.3 释放

调用Release成员函数释放位图资源，可以使用Create成员函数重新创建。

析构时候会自动释放。

#### 2.3.4 其他

可以手动编辑像素，并使用CopyFromMemory函数上传，注意这个二维vector的大小必须和原图一样。
头部：

```cpp
void CopyFromMemory(vector<vector<EasyPixel>> &vec);
```

使用一个二维EasyPixel数组上传像素。

可以重新设定渲染目标，使用SetRenderTarget函数
头部：
```cpp
void SetRenderTarget(ID2D1DeviceContext **pRenderTarget);
```


### 2.4 EasyFont 类，字体类

该类表示一个字体渲染对象，可以用来渲染到任意缓冲区上。

#### 2.4.1 创建

只能使用构造函数构造，头部如下
```cpp
EasyFont(string FontName, int SIZE, DWRITE_FONT_WEIGHT FontWeight = DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE FontStyle = DWRITE_FONT_STYLE_NORMAL);
```

**string FontName** 字体名称，需要系统中装了这个字体。\
**int SIZE** 字体大小，单位为像素。\
**DWRITE_FONT_WEIGHT FontWeight** 字体粗细，默认为正常。\
**DWRITE_FONT_STYLE FontStyle** 字体斜体样式，默认为正常。

对于后两个参数，具体选项可以参考Microsoft的官方文档。

还可以调用Create成员函数，参数和构造函数一样。

若是在EasyPaintingStart之前构造的，它们会在EasyPaintingStart时全部统一加载。

#### 2.4.2 渲染

调用Print函数，头部如下:

```cpp
void Print(string text, int x, int y, EasyPixel color = {255, 0, 0, 0}, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
```

**string text** 要渲染的文本，支持换行符\
**int x** 渲染x坐标\
**int y** 渲染y坐标\
**EasyPixel color** 渲染颜色，默认为黑色\
**ID2D1DeviceContext \*pRenderTarget** 渲染目标，默认为全局渲染目标

#### 2.4.3 释放

使用Release函数释放，头部如下:

```cpp
void Release();
```

析构时候会自动释放。

### 2.5 EasyBuffer 类，缓冲区类

该类表示一个缓冲区，可以用来渲染到主缓冲区上。前文提到多次的渲染目标就是这个类，传入渲染目标时，可以直接整个传入，会自动转化为ID2D1DeviceContext**。

全局缓冲区应该用EasyGetScreenBuffer()函数获取。

#### 2.5.1 创建

使用StartUp函数。头部如下:

```cpp
void StartUp(int width, int height);
```

**int width** 缓冲区宽度\
**int height** 缓冲区高度

也可以使用构造函数，效果和参数与StartUp函数相同。

#### 2.5.2 渲染

这是完整的一个缓冲区，渲染模式跟主缓冲区相似。

需要调用DrawStart/DrawEnd成员函数。

对于EasySurface的渲染，只需要直接调用EasySurface的Draw函数即可。注意构造EasySurface时，需要传入当前缓冲区作为渲染目标。

对于EasyFont的渲染，需要在Print函数内传入当前缓冲区作为渲染目标。

还有Clear函数，用于清空缓冲区，头部如下:

```cpp
void Clear(EasyPixel color);
```

将color填充到整个缓冲区中


#### 2.5.3 输出

使用Render函数渲染到另一个缓冲区上，头部如下:

```cpp
 void Render(int x, int y, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
```

**int x** 渲染x坐标\
**int y** 渲染y坐标\
**ID2D1DeviceContext \*pRenderTarget** 渲染目标，默认为全局渲染目标

#### 2.5.4 释放

使用Release函数释放缓冲区，头部如下:

```cpp
void Release();
```

析构时候会自动释放。

#### 2.5.6 几何

支持如下函数

```cpp
void DrawRectangle(int x, int y, int width, int height, EasyPixel color, float thickness);
void DrawLine(DXY point0, DXY point1, EasyPixel color, float thickness, D2D1_CAP_STYLE capStyle = D2D1_CAP_STYLE_ROUND, D2D1_DASH_STYLE dashStyle = D2D1_DASH_STYLE_SOLID);
void DrawRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float thickness, float radiusX, float radiusY);
void DrawEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY, float thickness);
void FillRectangle(int x, int y, int width, int height, EasyPixel color);
void FillRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float radiusX, float radiusY);
void FillEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY);
```

与EasyGeometry函数集类似，可以参照下面的说明。

#### 2.5.7 easyScreenBuffer对象

该对象表示屏幕缓冲区，成员函数和普通Buffer一样，但是不能释放，也不能重新创建。

### 2.6 EasyEffect 类

该类用于渲染特效，如模糊、阴影等。它表示了一个特效链\
*特效链：一个特效链表示一个特效的叠加，每个特效链可以包含多个特效。

#### 2.6.1 创建

使用StartUp函数。头部如下:

```cpp
void StartUp(ID2D1Bitmap1 *bitmap, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
void StartUp(EasySurface *surface, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
```

指定原始位图，以及该特效链的渲染目标缓冲区。

#### 2.6.2 添加特效

使用PushEffect函数添加一层特效，头部如下:

```cpp
void PushEffect(REFCLSID effectId);
```

传入一个DX2D的特效ID，具体类型可以翻阅Microsoft的文档。

使用SetValue函数设定特效参数，头部如下:

```cpp
template <typename T>
void SetValue(UINT32 prop, T &&value);
```

指定参数名还有具体参数值即可，详细参数可以翻阅Microsoft的文档。

若需要指定多个输入源，可以使用SetInput函数，EasyEffect的默认输入源索引为0，头部如下:

```cpp
void SetInput(ID2D1Bitmap1 *bitmap, UINT32 index);
void SetInput(ID2D1Effect *effect, UINT32 index);
```

指定输入源以及输入源索引即可。

EasyEffect提供了对ID2D1Effect*的转化，返回最后一层特效。

#### 2.6.3 快速添加特效

为了使用方便，不需要时刻查询文档，EasyEffect提供了快速添加特效的函数:

这些函数支持链式调用。

##### 2.6.3.1 高斯模糊

头部如下：
```cpp
EasyEffect &PushGaussianBlur(float standardDeviation);
```
指定了标准差。

##### 2.6.3.2 缩放
头部如下：
```cpp
EasyEffect &PushScale(float scaleX, float scaleY);
```
设定了宽高方向的拉伸比例。

##### 2.6.3.3 亮度
头部如下：
```cpp
EasyEffect &PushBrightness(float brightness);
```
指定了亮度变化比例。

##### 2.6.3.4 颜色变换

头部如下：
```cpp
EasyEffect &PushColorMatrix(EasyEffect::ColorMatrix matrix);
```
用来指定一个颜色变换矩阵。EasyEffect::ColorMatrix本质上是个二维数组，指定了矩阵。

具体的，令每个像素都乘上这个颜色矩阵
$$
 [R',G',B',A'] = [R,G,B,A] * matrix
$$

其中矩阵第五行是偏移项，即每个通道都会加上第五行对应的值。

##### 2.6.3.5 阴影

头部如下：
```cpp
EasyEffect &PushShadow(float standardDeviation, EasyPixel color);
```

指定阴影标准差和阴影颜色。

##### 2.6.3.6 裁剪

头部如下：
```cpp
EasyEffect &PushCrop(int x, int y, int width, int height);
```

指定裁剪区域。

#### 2.6.4 渲染

使用Render函数渲染特效链，头部如下:
```cpp
void Render(int x, int y, float rotation = 0, int midpointx = 0, int midpointy = 0, int reverse = 0);
```

**int x** 渲染位置x坐标\
**int y** 渲染位置y坐标\
**float rotation** 旋转角度\
**int midpointx** 旋转中心x坐标\
**int midpointy** 旋转中心y坐标\
**int reverse** 0为正常\
传入EASY_TURNLR表示左右翻转，\
传入EASY_TURNUD表示上下翻转。\
传入EASY_TURNLR|EASY_TURNUD表示左右上下都翻转。

注意此处的旋转中心坐标的原点为位图左上角，而不是中心，与EasySurface::DrawItEx函数不同。

#### 2.6.5 释放

可以通过Release函数释放特效链，头部如下:

```cpp
void Release();
```

还可以用PopEffect函数释放一层特效，头部如下:

```cpp
void PopEffect();
```

析构时候会自动释放。

### 2.7 EasyGeometry 函数集 与 EasyGeometryPath 类

#### 2.7.1 EasyDrawRectangle/EasyFillRectangle 函数

EasyDrawRectangle函数用于绘制一个空心矩形，头部如下:

```cpp
void EasyDrawRectangle(int x, int y, int width, int height, EasyPixel color, float thickness, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
```

**int x** 矩形左上角x坐标\
**int y** 矩形左上角y坐标\
**int width** 矩形宽度\
**int height** 矩形高度\
**EasyPixel color** 矩形颜色\
**float thickness** 线条宽度\
**ID2D1DeviceContext \*pRenderTarget** 渲染目标，默认为全局渲染目标


EasyFillRectangle函数用于绘制一个实心矩形，头部如下:

```cpp
void EasyFillRectangle(int x, int y, int width, int height, EasyPixel color, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
```

其中参数与EasyDrawRectangle函数相同，去除了线条宽度参数。


#### 2.7.2 EasyDrawRoundedRectangle/EasyFillRoundedRectangle 函数

EasyDrawRoundedRectangle函数用于绘制一个空心圆角矩形，头部如下:

```cpp
void EasyDrawRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float thickness, float radiusX, float radiusY, ID2D1DeviceContext *pRenderTarget =&EasyPainting::pRenderTarget);
```

**int x** 矩形左上角x坐标\
**int y** 矩形左上角y坐标\
**int width** 矩形宽度\
**int height** 矩形高度\
**EasyPixel color** 矩形颜色\
**float thickness** 线条宽度\
**float radiusX** 圆角x半径\
**float radiusY** 圆角y半径\
**ID2D1DeviceContext \*pRenderTarget** 渲染目标，默认为全局渲染目标

EasyFillRoundedRectangle函数用于绘制一个实心圆角矩形，头部如下:

```cpp
void EasyFillRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float radiusX, float radiusY, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
```

其中参数与EasyDrawRoundedRectangle函数相同，去除了线条宽度参数。


#### 2.7.3 EasyDrawEllipse/EasyFillEllipse 函数

EasyDrawEllipse函数用于绘制一个空心椭圆，头部如下:

```cpp
void EasyDrawEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY, float thickness, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
```

**int x** 椭圆中心x坐标\
**int y** 椭圆中心y坐标\
**EasyPixel color** 椭圆颜色\
**float radiusX** 椭圆x半径\
**float radiusY** 椭圆y半径\
**float thickness** 线条宽度\
**ID2D1DeviceContext \*pRenderTarget** 渲染目标，默认为全局渲染目标

EasyFillEllipse函数用于绘制一个实心椭圆，头部如下:

```cpp
void EasyFillEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
```

其中参数与EasyDrawEllipse函数相同，去除了线条宽度参数。


#### 2.7.4 EasyDrawLine 函数

该函数用于绘制一条线段，头部如下:

```cpp
void EasyDrawLine(DXY point0, DXY point1, EasyPixel color, float thickness, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget, D2D1_CAP_STYLE CapStyle = D2D1_CAP_STYLE_ROUND, D2D1_DASH_STYLE DashStyle = D2D1_DASH_STYLE_SOLID);
```

**DXY point0** 线段起点\
**DXY point1** 线段终点\
**EasyPixel color** 线段颜色\
**float thickness** 线段粗细\
**ID2D1DeviceContext \*pRenderTarget** 渲染目标，默认为全局渲染目标\
**D2D1_CAP_STYLE CapStyle** 线段端点样式，默认为D2D1_CAP_STYLE_ROUND，圆角端点\
**D2D1_DASH_STYLE DashStyle** 线段样式，默认为D2D1_DASH_STYLE_SOLID，实线

对于后两个参数的详细类型，请参考Microsoft的文档。


#### 2.7.5 EasyGeometryPath 类

该类用来绘制一个路径。

##### 2.7.5.1 启动

使用StartUp函数，头部如下:

```cpp
void StartUp();
```

##### 2.7.5.2 录入点

使用FigureStart函数进行开始录制。头部如下:

```cpp
void FigureStart(float x, float y);
```

**float x** 起点x坐标\
**float y** 起点y坐标

使用FigureEnd函数进行结束录制。头部如下:

```cpp
void FigureEnd(bool close = false);
```

**bool close** 是否闭合路径，默认为false，不闭合，若闭合，则路径会自动连接起点和终点。

使用AddLine函数进行添加点。头部如下:

```cpp
void AddLine(float x, float y);
```

**float x** 点x坐标\
**float y** 点y坐标

##### 2.7.5.3 渲染

使用DrawGeometry进行路径渲染。头部如下:

```cpp
void DrawGeometry(EasyPixel color, float thickness, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
```
**EasyPixel color** 线段颜色\
**float thickness** 线段粗细\
**ID2D1DeviceContext \*pRenderTarget** 渲染目标，默认为全局渲染目标

还可以使用FillGeometry进行填充渲染。头部如下:

```cpp
void FillGeometry(EasyPixel color, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
```

**EasyPixel color** 填充颜色\
**ID2D1DeviceContext \*pRenderTarget** 渲染目标，默认为全局渲染目标

这个函数会填充路径内的区域并渲染。


##### 2.7.5.4 释放

使用Release函数进行释放。头部如下:

```cpp
void Release();
```

可以使用StartUp函数进行重新初始化。

析构时候会自动释放。

### 2.8 EasyPaintingMath 函数集

该函数集用于数学计算。

```cpp
class DXY;
class doubleXY;
template <class T>
class templateXY;
```

分别存不同类型的坐标，其中DXY和doubleXY有互相转化的重载。


```cpp
inline double toRadians(double degrees);
inline double toDegrees(double radians);
```

用于角度和弧度的转换。


```cpp
#define val_distance(a, b)
```

用于求差值。

```cpp
#define TimeIt(a)
```

用于求平方。


```cpp
double LinearVelx(double angle);
double LinearVely(double angle);
```

用于求向一个角度以单位速度移动时的x和y方向速度分量。

```cpp
int DXYDistance(DXY first, DXY second);
```
用于求两个坐标之间的距离。

```cpp
double GoRotation(doubleXY x, doubleXY y);
```

用于求一个坐标相对于另一个坐标的角度，使用屏幕坐标系。

### 2.9 SPRITE 类及相关函数

SPRITE类表示一个精灵，里面有许多成员。

```cpp
class SPRITE
{
public:
    float x, y;
    int width, height;
    float scaling, rotation;
    float velx, vely;
};
```

自行使用。

```cpp
bool SpritePeek(SPRITE sprite1, SPRITE sprite2);
bool SpritePeekLine(SPRITE first, SPRITE second);
```

用于检测两个精灵是否碰撞。\
SpritePeek函数使用矩形判断，SpritePeekLine函数使用距离判断。


### 2.10 EasyPaintingMouseInput 函数集

该函数集用于获取鼠标输入。

```cpp
DXY MouseWinXY(void);
```

得到鼠标在窗口中的坐标，这个坐标是对于EasyPainting的主渲染缓冲区而言的。

```cpp
bool KeyIt(SPRITE in);
```
判断是否按下某个精灵。

### 2.11 EasyCreateConsoleWindow 函数

头部:
```cpp
void EasyCreateConsoleWindow();
```

用于创建控制台并重定向，便于调试。

### 2.12 EasyPaintingBitmapEditor 函数集

该函数集用于编辑位图。

#### 2.12.1 EasyPixel 类

```cpp
class EasyPixel
{
public:
    unsigned char a;
    unsigned char r;
    unsigned char g;
    unsigned char b;
};
```

用于表示一个像素或颜色。
支持传入ARGB构造，传入RGB构造，传入COLORREF构造。alpha默认255。
支持和COLORREF的互相转换。

该类又名EasyColor。

#### 2.12.2 EasyLoadBitmapFromFile函数

```cpp
void EasyLoadBitmapFromFile(string file_path, vector<vector<EasyPixel>> &vec);
```

传入路径，用于将位图加载到像素数组中。vec会被重写，使用方法应该为vec[x][y]。

#### 2.12.3 EasySaveBitmapToFile函数

```cpp
void EasySaveBitmapToFile(string file_path, vector<vector<EasyPixel>> &vec);
```

传入路径，用于将像素数组保存为位图，vec保存方式和LoadBitmapFromFileToPixel一致。

### 2.13 Direct2D/WinGDI辅助函数

```cpp
ID2D1Bitmap1 *LoadBitmap1FromFile(string file_path, int width, int height, EasyPixel transparent_color, ID2D1DeviceContext *DeviceContext = EasyPainting::pRenderTarget);
ID2D1Bitmap1 *CreateBitmap1FromArray(ID2D1DeviceContext *pDeviceContext, const BYTE *pixelData, UINT width, UINT height);
ID2D1Bitmap1 *CreatePureColorBitmap1(ID2D1DeviceContext *pDeviceContext, EasyPixel color, int width, int height);
ID2D1Bitmap1 *CreateNullBitmap1(ID2D1DeviceContext *pDeviceContext, UINT width, UINT height);
void RenderBitmap1(ID2D1DeviceContext *pDeviceContext, ID2D1Bitmap1 *pBitmap, float x, float y);
```

LoadBitmap1FromFile函数用于从文件中读取一个ID2D1Bitmap1对象。\
CreateBitmap1FromArray函数用于从数组中创建一个ID2D1Bitmap1对象。\
CreatePureColorBitmap1函数用于创建一个纯色ID2D1Bitmap1对象。\
CreateNullBitmap1函数用于创建一个空ID2D1Bitmap1对象。\
RenderBitmap1函数用于渲染一个ID2D1Bitmap1对象。

一般比较少用，提供给更底层的操作。

还有对应的ID2D1Bitmap接口的函数，但是推荐使用ID2D1Bitmap1。

```cpp
ID2D1Bitmap *LoadBitmapFromFile(string file_path, int width, int height, EasyPixel transparent_color, ID2D1RenderTarget *pRenderTarget = EasyPainting::pRenderTarget);
ID2D1Bitmap *CreateBitmapFromArray(ID2D1RenderTarget *pRenderTarget, const BYTE *pixelData, UINT width, UINT height);
ID2D1Bitmap *CreatePureColorBitmap(ID2D1RenderTarget *pRenderTarget, EasyPixel color, int width, int height);
ID2D1Bitmap *CreateNullBitmap(ID2D1RenderTarget *pRenderTarget, UINT width, UINT height);
void RenderBitmap(ID2D1RenderTarget *pRenderTarget, ID2D1Bitmap *pBitmap, float x, float y);
```

另外还提供了一些WinGDI的函数，为旧版本遗留，不推荐使用，自行阅读函数头部理解，不介绍。

### 2.14 easyPaintingDevice 对象

一个代表EasyPainting的对象，用于一些全局设置

#### 2.14.1 SetVSync 函数

用于开关垂直同步
头部如下：
```cpp
void SetVSync(bool vsync);
```

#### 2.14.2 SetWindow 函数

头部如下:
```cpp
void SetWindow(HWND window, int WindowWidth, int WindowHeight, EasyPixel BackColor = RGB(255, 255, 255));
```

即重置了EasyPaintingStart的参数，注意这个函数效率较低。

#### 2.14.3 GetFPS 函数

头部如下：

```cpp
int GetFPS();
```

获得最新一个单位秒的帧率。

#### 2.14.4 SetInterpolationMode 函数

用于设置插值模式，头部如下：

```cpp
void SetInterpolationMode(int mode);
```

有两个选项：
- EASY_LINEAR_MODE 线性插值
- EASY_NEAREST_MODE 最近插值


## 3.历史

该库诞生于2023.1.10左右。目前为10.0，三周年版本。\
历史版本及简述：

1.0 - 2023.1.10\
基于WinGDI实现了基本功能。

2.0 - 2023.2.12\
实现了对于窗口的适应拉伸。

2.1 - 2023.2.25\
修复字符拉伸问题。

2.2 - 2023.3.1\
加强了窗口适应拉伸的兼容。

2.3.1 - 2023.3.2\
首次使用总拉伸实现拉伸。

2.3.2 - 2023.3.25\
使用全部重载实现拉伸。

2.3.3 - 2023.3.25\
进行了简单的化简。

2.3.4 - 2023.4.1\
修复了重复拉伸清晰度变低的问题。

3.0 - 2023.4.5\
提供了生成透明底色遮罩的功能。

4.0 - 2023.6.4\
首次实现了多重缓冲区功能。

4.1 - 2023.7.12\
加了遮罩位图的色差容忍功能。

4.2 - 2023.7.15\
增添了一些小功能。

4.3 - 2023.11.25\
修复了一些bug。

5.0 - 2024.1.28\
一周年版本，修复了一些bug。

6.0 - 2024.4.13\
进行了一些化简和优化，使性能稍微提升，重新采用总拉伸。

7.0 - 2024.10.1\
写了EasyPaintingFormatModule模块\
实现了对PNG，JPG等格式的支持。

8.0 - 2024.11.2\
底层修改为DirectX，做了很大改动。

9.0 - 2025.1.16\
二周年版本，添加了镜像翻转功能。

9.1 - 2025.6.20\
修复了已知bug。


By MrJayden.
