// EasyPainting.h 10.0 Celebrate the third anniversary
#pragma once
#include <windows.h>
#include <string>
#include <cmath>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <chrono>

// Direct2D
#include <d2d1_1.h>
#include <dwrite.h>

#include <d3d11.h>
#include <d2d1_1helper.h>
#include <dxgi1_2.h>
#include <d2d1effects.h>
#include <d2d1effects_1.h>
#include <d2d1effects_2.h>

// wincodec
#include <wincodec.h>

#include <iconv.h>

#include <shlwapi.h>

using namespace std;

namespace EasyPainting
{
    extern ID2D1Factory1 *pFactory;
    extern ID2D1DeviceContext *pRenderTarget;
    extern IDWriteFactory *pWriteFactory;

    extern ID3D11Device *pD3dDevice;
    extern ID3D11DeviceContext *pD3dContext;
    extern IDXGISwapChain1 *pSwapChain;
    extern ID2D1Device *pD2dDevice;
    extern ID2D1Bitmap1 *pTargetBitmap;

    extern D2D1_INTERPOLATION_MODE easyInterpolationMode;

    struct __Easy_Font_Info
    {
        string fontName;
        float fontSize;
        DWRITE_FONT_WEIGHT fontWeight;
        DWRITE_FONT_STYLE fontStyle;
    };

    extern D2D1_MATRIX_3X2_F identityMatrix;
};

// EasyPainting Point
class DXY;
class doubleXY;

class DXY
{
public:
    int x, y;
    DXY(int x, int y);
    DXY();
    operator doubleXY();
};
class doubleXY
{
public:
    double x, y;
    doubleXY(double x, double y);
    doubleXY();
    operator DXY();
};
typedef doubleXY EasyPoint;

template <class T>
class templateXY
{
public:
    T x, y;
    templateXY(T x, T y);
    templateXY();
};

// EasyPainting Sprite
class SPRITE
{
public:
    float x, y;
    int width, height;
    float scaling, rotation;
    float velx, vely;
    SPRITE();
    SPRITE(int x_, int y_, int width_, int height_);
};

// EasyPainting Pixel
class EasyPixel
{
public:
    unsigned char a;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    EasyPixel();
    EasyPixel(unsigned char a, unsigned char r, unsigned char g, unsigned char b);
    EasyPixel(unsigned char r, unsigned char g, unsigned char b);
    EasyPixel(COLORREF color);
    EasyPixel(D2D1_COLOR_F color);
    operator COLORREF();
    operator D2D1_COLOR_F();
};

typedef EasyPixel EasyColor;

class EasyBrush;

class EasySurface
{
public:
    EasySurface(const EasySurface &other) = delete;
    EasySurface &operator=(const EasySurface &other) = delete;

    ID2D1Bitmap1 *image = nullptr;
    EasyPixel transColor;
    float imageWidth = 0, imageHeight = 0;
    EasySurface(string filename, int width, int height, EasyPixel transColor);
    EasySurface();
    ~EasySurface(void);
    void Create(string filename, int width, int height, EasyPixel transColor);
    void DrawIt(float x, float y, float width, float height, float scaling = 1, float rotation = 0, int columns = 1, int frames = 0, int reverse = 0);
    void DrawItEx(float x, float y, float width, float height, int imagex, int imagey, int imagewidth, int imageheight, float rotation = 0, int midpointdx = 0, int midpointdy = 0, int reverse = 0);
    void DrawItPoint(EasyPoint p0, EasyPoint p1, EasyPoint p2, int imagex = 0, int imagey = 0, int imagewidth = 0, int imageheight = 0);
    void DrawItDirect(float x, float y, float width, float height);
    void DrawItFrames(float x, float y, float width, float height, int columns = 1, int frames = 0, float scaling = 1);
    void DrawItReverse(float x, float y, float width, float height, int reverse, float scaling = 1, int rotation = 0);

    void CreateFromMemory(vector<vector<EasyPixel>> &vec, EasyPixel transColor = RGB(0, 0, 0));
    void CopyFromMemory(vector<vector<EasyPixel>> &vec, EasyPixel transColor = RGB(0, 0, 0));

    bool GetPixelData(vector<vector<EasyPixel>> &vec);

    void Release(void);
    void LoadIt(string filename);
};
typedef EasySurface SURFACE;

class EasyFont
{
public:
    EasyFont(const EasyFont &other) = delete;
    EasyFont &operator=(const EasyFont &other) = delete;

    IDWriteTextFormat *pFont = nullptr;

    EasyFont(string fontName, int fontSize, DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL);
    EasyFont();
    ~EasyFont();
    void Create(string fontName, int fontSize, DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL);
    void Print(string text, float x, float y, EasyPixel color = {255, 0, 0, 0});
    void Print(string text, float x, float y, EasyBrush &brush);
    void PrintEx(string text, float x, float y, float width, float height, EasyBrush &brush, EasyPoint center = {0, 0}, float rotation = 0.0f);
    void PrintEx(string text, float x, float y, float width, float height, EasyPixel color, EasyPoint center = {0, 0}, float rotation = 0.0f);
    void PrintEx(string text, float width, float height, EasyPixel color, EasyPoint p0, float angle1 = 0.0f, float angle2 = 90.0f, float scalew = 1.0f, float scaleh = 1.0f);
    void PrintEx(string text, float width, float height, EasyBrush &brush, EasyPoint p0, float angle1 = 0.0f, float angle2 = 90.0f, float scalew = 1.0f, float scaleh = 1.0f);
    void LoadIt(EasyPainting::__Easy_Font_Info info);
    void Release();
};

// EasyPainting Buffer
class EasyBufferBase
{
public:
    ID2D1DeviceContext *pBufferDevice = nullptr;

    EasyBufferBase() = default;
    EasyBufferBase(const EasyBufferBase &other) = delete;
    EasyBufferBase &operator=(const EasyBufferBase &other) = delete;

    void Clear(EasyPixel color);

    void UseAsRenderTarget();

    operator ID2D1DeviceContext *();
    operator ID2D1DeviceContext **();
};

class EasyBuffer : public EasyBufferBase, public EasySurface
{
public:
    ID2D1Bitmap1 *&pBufferBitmap = EasySurface::image;
    float &width = EasySurface::imageWidth, &height = EasySurface::imageHeight;
    EasyBuffer();
    EasyBuffer(int width, int height);
    void StartUp(int width, int height);
    void DrawStart(void);
    void DrawEnd(void);

    void Release(void);

    ~EasyBuffer();
};

class EasyPaintingScreenBuffer : public EasyBufferBase
{
    friend void EasyPaintingStart(HWND window, int windowWidth, int windowHeight, EasyPixel backColor);

protected:
    EasyPaintingScreenBuffer();
    EasyPaintingScreenBuffer(const EasyPaintingScreenBuffer &other) = delete;
    EasyPaintingScreenBuffer &operator=(const EasyPaintingScreenBuffer &other) = delete;

public:
    void DrawStart();
    void DrawEnd();
};

extern EasyPaintingScreenBuffer *easyScreenBuffer;

// EasyPainting Effect
class EasyEffect
{
public:
    EasyEffect() = default;
    EasyEffect(const EasyEffect &other) = delete;
    EasyEffect &operator=(const EasyEffect &other) = delete;

    vector<ID2D1Effect *> effects;
    ID2D1Bitmap1 *bitmap;

    void StartUp(EasySurface &surface);
    void StartUp(ID2D1Bitmap1 *bitmap);
    void SetInput(ID2D1Bitmap1 *bitmap, UINT32 index);
    void SetInput(ID2D1Effect *effect, UINT32 index);
    template <typename T>
    void SetValue(UINT32 prop, T &&value);
    void PushEffect(REFCLSID effectId);
    void PopEffect(void);
    void Render(int x, int y, float rotation = 0, int midpointx = 0, int midpointy = 0, float scaleX = 1, float scaleY = 1, int reverse = 0);
    void RenderMatrix(D2D1_MATRIX_3X2_F &matrix);
    void Release(void);

    EasyEffect &PushGaussianBlur(float standardDeviation);
    EasyEffect &PushScale(float scaleX, float scaleY);
    EasyEffect &PushBrightness(float brightness);
    typedef D2D1_MATRIX_5X4_F ColorMatrix;
    EasyEffect &PushColorMatrix(ColorMatrix matrix);
    EasyEffect &PushShadow(float standardDeviation, EasyPixel color);
    EasyEffect &PushCrop(int x, int y, int width, int height);

    operator ID2D1Effect *();
    ~EasyEffect();
};

// EasyPainting Brush
class EasyBrush
{
public:
    EasyBrush(const EasyBrush &other) = delete;
    EasyBrush &operator=(const EasyBrush &other) = delete;

    ID2D1Brush *pBrush = nullptr;

    EasyBrush(void) = default;

    void CreateSolid(EasyPixel color);
    void CreateLinearGradientBrush(vector<pair<float, EasyPixel>> &stops, EasyPoint start, EasyPoint end);
    void CreateRadialGradientBrush(vector<pair<float, EasyPixel>> &stops, EasyPoint center, EasyPoint offset, float radiusX, float radiusY);
    void CreateBitmapBrush(EasySurface &surface, int offsetx, int offsety);

    void DrawRectangle(float x, float y, float width, float height, float thickness);
    void DrawLine(EasyPoint point0, EasyPoint point1, float thickness, D2D1_CAP_STYLE capStyle = D2D1_CAP_STYLE_ROUND, D2D1_DASH_STYLE dashStyle = D2D1_DASH_STYLE_SOLID);
    void DrawRoundedRectangle(float x, float y, float width, float height, float thickness, float radiusX, float radiusY);
    void DrawEllipse(float x, float y, float radiusX, float radiusY, float thickness);

    void FillRectangle(float x, float y, float width, float height);
    void FillRoundedRectangle(float x, float y, float width, float height, float radiusX, float radiusY);
    void FillEllipse(float x, float y, float radiusX, float radiusY);

    void DrawRectangleEx(D2D1_MATRIX_3X2_F transform, float x, float y, float width, float height, float thickness);
    void DrawLineEx(D2D1_MATRIX_3X2_F transform, EasyPoint point0, EasyPoint point1, float thickness, D2D1_CAP_STYLE capStyle = D2D1_CAP_STYLE_ROUND, D2D1_DASH_STYLE dashStyle = D2D1_DASH_STYLE_SOLID);
    void DrawRoundedRectangleEx(D2D1_MATRIX_3X2_F transform, float x, float y, float width, float height, float thickness, float radiusX, float radiusY);
    void DrawEllipseEx(D2D1_MATRIX_3X2_F transform, float x, float y, float radiusX, float radiusY, float thickness);

    void FillRectangleEx(D2D1_MATRIX_3X2_F transform, float x, float y, float width, float height);
    void FillRoundedRectangleEx(D2D1_MATRIX_3X2_F transform, float x, float y, float width, float height, float radiusX, float radiusY);
    void FillEllipseEx(D2D1_MATRIX_3X2_F transform, float x, float y, float radiusX, float radiusY);

    void Release(void);
    ~EasyBrush(void);
};

// EasyPainting GeometryPath
class EasyGeometryPath
{
public:
    EasyGeometryPath() = default;
    EasyGeometryPath(const EasyGeometryPath &other) = delete;
    EasyGeometryPath &operator=(const EasyGeometryPath &other) = delete;

    ID2D1PathGeometry *pathGeometry = nullptr;
    ID2D1GeometrySink *sink = nullptr;

    void StartUp();
    void FigureStart(float x, float y);
    void FigureEnd(bool close = false);
    void AddLine(float x, float y);

    static const int CLOCK_WISE = D2D1_SWEEP_DIRECTION_CLOCKWISE;
    static const int COUNTER_CLOCKWISE = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;

    static const int LARGE_ARC = D2D1_ARC_SIZE_LARGE;
    static const int SMALL_ARC = D2D1_ARC_SIZE_SMALL;

    void AddArc(float x, float y, float radiusX, float radiusY, int sweepDirection = CLOCK_WISE, int arcSize = SMALL_ARC, float rotationAngle = 0.0f);

    void AddQuadraticBezier(EasyPoint target, EasyPoint control);

    void AddBezier(EasyPoint target1, EasyPoint control1, EasyPoint control2);

    void DrawGeometry(EasyBrush &brush, float thickness);
    void FillGeometry(EasyBrush &brush);
    void DrawGeometry(EasyPixel color, float thickness);
    void FillGeometry(EasyPixel color);

    void DrawGeometryEx(EasyBrush &brush, float thickness, EasyPoint p0, float angle1 = 0.0f, float angle2 = 90.0f, float scalew = 1.0f, float scaleh = 1.0f);
    void FillGeometryEx(EasyBrush &brush, EasyPoint p0, float angle1 = 0.0f, float angle2 = 90.0f, float scalew = 1.0f, float scaleh = 1.0f);
    void DrawGeometryEx(EasyPixel color, float thickness, EasyPoint p0, float angle1 = 0.0f, float angle2 = 90.0f, float scalew = 1.0f, float scaleh = 1.0f);
    void FillGeometryEx(EasyPixel color, EasyPoint p0, float angle1 = 0.0f, float angle2 = 90.0f, float scalew = 1.0f, float scaleh = 1.0f);

    void DrawGeometryEx(D2D1_MATRIX_3X2_F transform, EasyBrush &brush, float thickness);
    void FillGeometryEx(D2D1_MATRIX_3X2_F transform, EasyBrush &brush);
    void DrawGeometryEx(D2D1_MATRIX_3X2_F transform, EasyPixel color, float thickness);
    void FillGeometryEx(D2D1_MATRIX_3X2_F transform, EasyPixel color);

    void Release();
    ~EasyGeometryPath();
};

class EasyGeometryPainter
{
    friend void EasyPaintingStart(HWND window, int windowWidth, int windowHeight, EasyPixel backColor);

protected:
    EasyGeometryPainter() = default;
    EasyGeometryPainter(const EasyGeometryPainter &other) = delete;
    EasyGeometryPainter &operator=(const EasyGeometryPainter &other) = delete;

public:
    void DrawRectangle(float x, float y, float width, float height, EasyPixel color, float thickness);
    void DrawLine(EasyPoint point0, EasyPoint point1, EasyPixel color, float thickness, D2D1_CAP_STYLE capStyle = D2D1_CAP_STYLE_ROUND, D2D1_DASH_STYLE dashStyle = D2D1_DASH_STYLE_SOLID);
    void DrawRoundedRectangle(float x, float y, float width, float height, EasyPixel color, float thickness, float radiusX, float radiusY);
    void DrawEllipse(float x, float y, EasyPixel color, float radiusX, float radiusY, float thickness);

    void FillRectangle(float x, float y, float width, float height, EasyPixel color);
    void FillRoundedRectangle(float x, float y, float width, float height, EasyPixel color, float radiusX, float radiusY);
    void FillEllipse(float x, float y, EasyPixel color, float radiusX, float radiusY);

    void DrawRectangleEx(D2D1_MATRIX_3X2_F transform, float x, float y, float width, float height, EasyPixel color, float thickness);
    void DrawLineEx(D2D1_MATRIX_3X2_F transform, EasyPoint point0, EasyPoint point1, EasyPixel color, float thickness, D2D1_CAP_STYLE capStyle = D2D1_CAP_STYLE_ROUND, D2D1_DASH_STYLE dashStyle = D2D1_DASH_STYLE_SOLID);
    void DrawRoundedRectangleEx(D2D1_MATRIX_3X2_F transform, float x, float y, float width, float height, EasyPixel color, float thickness, float radiusX, float radiusY);
    void DrawEllipseEx(D2D1_MATRIX_3X2_F transform, float x, float y, EasyPixel color, float radiusX, float radiusY, float thickness);

    void FillRectangleEx(D2D1_MATRIX_3X2_F transform, float x, float y, float width, float height, EasyPixel color);
    void FillRoundedRectangleEx(D2D1_MATRIX_3X2_F transform, float x, float y, float width, float height, EasyPixel color, float radiusX, float radiusY);
    void FillEllipseEx(D2D1_MATRIX_3X2_F transform, float x, float y, EasyPixel color, float radiusX, float radiusY);
};

extern EasyGeometryPainter *easyGeometryPainter;

class EasyPaintingDevice
{
    friend void EasyPaintingStart(HWND window, int windowWidth, int windowHeight, EasyPixel backColor);

private:
    EasyPaintingDevice();
    EasyPaintingDevice(const EasyPaintingDevice &other) = delete;
    EasyPaintingDevice &operator=(const EasyPaintingDevice &other) = delete;

public:
    void SetVSync(bool vsync);

    void SetWindow(HWND window, int windowWidth, int windowHeight, EasyPixel backColor = RGB(255, 255, 255));

    void SetInterpolationMode(int mode);

    int GetFPS();
};

extern EasyPaintingDevice *easyPaintingDevice;

// EasyPainting Auxiliary function
HBITMAP CreatePureColorBitmap(COLORREF cNewColor, LONG width, LONG height);
HBITMAP GetScalingBitmap(HBITMAP p_bitmap, int width, int height, int OldWidth, int OldHeight);
HBITMAP GetScalingBitmap_Delete(HBITMAP p_bitmap, int width, int height, int OldWidth, int OldHeight);
HBITMAP MaskBitmap(HBITMAP p_bitmap, int width, int height, COLORREF maskColor);
HBITMAP CreateMaskBitmapWithTolerance(HBITMAP hSourceBitmap, COLORREF transparentColor, int tolerance);
HBITMAP GetSelectedBitmap(HBITMAP p_bitmap, int x, int y, int width, int height);
HBITMAP LoadBitmapFromFile(string filename);
void DrawhBitmap(HDC device, HBITMAP image, int x, int y);
void DrawhBitmapWithMatte(HDC device, HBITMAP image, int x, int y, HBITMAP matte);
void DrawBitmap(HDC device, HBITMAP image, int x, int y, HBITMAP matte, int rotation);
DXY GetBitmapWidthHeight(HBITMAP image);
void GetDevice(HWND window, HDC *device);

// EasyPainting Pen
HPEN EasyCreatePen(COLORREF color, int cWidth);
void DrawLine(HDC device, HPEN pen, DXY f, DXY t);
void DrawRect(HDC device, HPEN pen, int x, int y, int width, int height);
void DrawCircle(HDC device, HPEN pen, int x, int y);

bool SpritePeek(SPRITE sprite1, SPRITE sprite2);
bool SpritePeekLine(SPRITE first, SPRITE second);
DXY EasyMousePos(void);
bool KeyIt(SPRITE in);

ID2D1Bitmap *EasyD2DLoadBitmapFromFile(string file_path, int width, int height, EasyPixel transparent_color, ID2D1RenderTarget *pRenderTarget = EasyPainting::pRenderTarget);
ID2D1Bitmap *EasyD2DCreateBitmapFromArray(ID2D1RenderTarget *pRenderTarget, const BYTE *pixelData, UINT width, UINT height);
ID2D1Bitmap *EasyD2DCreatePureColorBitmap(ID2D1RenderTarget *pRenderTarget, EasyPixel color, int width, int height);
ID2D1Bitmap *EasyD2DCreateNullBitmap(ID2D1RenderTarget *pRenderTarget, UINT width, UINT height);
void EasyD2DRenderBitmap(ID2D1RenderTarget *pRenderTarget, ID2D1Bitmap *pBitmap, float x, float y);
bool EasyD2DGetPixelFromBitmap(ID2D1Bitmap *pBitmap, vector<vector<EasyPixel>> &pixels, ID2D1DeviceContext *pDeviceContext = EasyPainting::pRenderTarget);

ID2D1Bitmap1 *EasyD2DLoadBitmap1FromFile(string file_path, int width, int height, EasyPixel transparent_color, ID2D1DeviceContext *pDeviceContext = EasyPainting::pRenderTarget);
ID2D1Bitmap1 *EasyD2DCreateBitmap1FromArray(ID2D1DeviceContext *pDeviceContext, const BYTE *pixelData, UINT width, UINT height);
ID2D1Bitmap1 *EasyD2DCreatePureColorBitmap1(ID2D1DeviceContext *pDeviceContext, EasyPixel color, int width, int height);
ID2D1Bitmap1 *EasyD2DCreateNullBitmap1(ID2D1DeviceContext *pDeviceContext, UINT width, UINT height);
void EasyD2DRenderBitmap1(ID2D1DeviceContext *pDeviceContext, ID2D1Bitmap1 *pBitmap, float x, float y);
bool EasyD2DGetPixelFromBitmap1(ID2D1Bitmap1 *pBitmap, vector<vector<EasyPixel>> &pixels, ID2D1DeviceContext *pDeviceContext = EasyPainting::pRenderTarget);

void EasyLoadBitmapFromFile(string file_path, vector<vector<EasyPixel>> &vec);
void EasySaveBitmapToFile(string file_path, vector<vector<EasyPixel>> &vec);

void EasyCreateConsoleWindow();

// EasyPainting Math
extern const double PI;
extern const double PI_over_180;
extern const double PI_under_180;
inline double toRadians(double degrees);
inline double toDegrees(double radians);
#define val_distance(a, b) (a > b ? (a - b) : (b - a))
#define TimeIt(a) ((a) * (a))
double GoRotation(EasyPoint x, EasyPoint y);
double LinearVelx(double angle);
double LinearVely(double angle);
int DXYDistance(DXY first, DXY second);
double EasyPointDistance(EasyPoint first, EasyPoint second);

// const number
extern const int EASY_TURNLR;
extern const int EASY_TURNUD;

extern const int EASY_LINEAR_MODE;
extern const int EASY_NEAREST_MODE;
extern const int EASY_CUBIC_MODE;
extern const int EASY_HIGH_QUALITY_CUBIC_MODE;
extern const int EASY_ANISOTROPIC_MODE;

void EasyPaintingStart(HWND window, int windowWidth, int windowHeight, EasyPixel backColor = RGB(255, 255, 255));
void DrawStart();
void DrawEnd();
