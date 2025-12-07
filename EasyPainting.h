// EasyPainting.h 10.0 Celebrate the third anniversary
#pragma once
#include <windows.h>
#include <string>
#include <fstream>
#include <cmath>
#include <iostream>
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

    // EasyPainting Draw
    extern double ConversionWidth;
    extern double ConversionHeight;

    extern bool EasyPaintingStartFlag;
    extern bool enableVSync;

    extern mutex fpsMutex;
    extern int fpsCounter;
    extern int showFPS;

    extern D2D1_BITMAP_INTERPOLATION_MODE easyInterpolationMode;

    struct __Easy_Font_Info
    {
        string FontName;
        float SIZE;
        DWRITE_FONT_WEIGHT FontWeight;
        DWRITE_FONT_STYLE FontStyle;
    };
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

// EasyPainting Bitmap Editor
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

class SURFACE
{
public:
    ID2D1Bitmap1 *image = nullptr;
    ID2D1DeviceContext **pRenderTarget;
    EasyPixel MatteColor;
    float Width = 0, Height = 0;
    SURFACE(string filename, int width, int height, EasyPixel MatteColor, ID2D1DeviceContext **pRenderTarget = &EasyPainting::pRenderTarget);
    SURFACE();
    ~SURFACE(void);
    void Create(string filename, int width, int height, EasyPixel MatteColor, ID2D1DeviceContext **pRenderTarget = &EasyPainting::pRenderTarget);
    void DrawIt(int x, int y, int width, int height, float scaling = 1, float rotation = 0, int columns = 1, int frames = 0, int reverse = 0);
    void DrawItEx(int x, int y, int width, int height, int imagex, int imagey, int imagewidth, int imageheight, float rotation = 0, int midpointdx = 0, int midpointdy = 0, int reverse = 0);
    void DrawItDirect(int x, int y, int width, int height);
    void DrawItFrames(int x, int y, int width, int height, int columns = 1, int frames = 0, float scaling = 1);
    void DrawItReverse(int x, int y, int width, int height, int reverse, float scaling = 1, int rotation = 0);

    void SetRenderTarget(ID2D1DeviceContext **pRenderTarget);
    void CreateFromMemory(vector<vector<EasyPixel>> &vec);
    void CopyFromMemory(vector<vector<EasyPixel>> &vec);

    void Release(void);
    void LoadIt(string filename);
};
typedef SURFACE EasySurface;

class EasyFont
{
public:
    IDWriteTextFormat *pFont = nullptr;

    EasyFont(string FontName, int SIZE, DWRITE_FONT_WEIGHT FontWeight = DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE FontStyle = DWRITE_FONT_STYLE_NORMAL);
    EasyFont(void);
    ~EasyFont(void);
    void Create(string FontName, int SIZE, DWRITE_FONT_WEIGHT FontWeight = DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE FontStyle = DWRITE_FONT_STYLE_NORMAL);
    void Print(string text, int x, int y, EasyPixel color = {255, 0, 0, 0}, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
    void LoadIt(EasyPainting::__Easy_Font_Info info);
    void Release(void);
};

// EasyPainting Buffer
class EasyBufferBase
{
public:
    ID2D1DeviceContext *pBufferDevice;

    void DrawRectangle(int x, int y, int width, int height, EasyPixel color, float thickness);
    void DrawLine(DXY point0, DXY point1, EasyPixel color, float thickness, D2D1_CAP_STYLE capStyle = D2D1_CAP_STYLE_ROUND, D2D1_DASH_STYLE dashStyle = D2D1_DASH_STYLE_SOLID);
    void DrawRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float thickness, float radiusX, float radiusY);
    void DrawEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY, float thickness);

    void FillRectangle(int x, int y, int width, int height, EasyPixel color);
    void FillRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float radiusX, float radiusY);
    void FillEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY);

    void Clear(EasyPixel color);

    operator ID2D1DeviceContext *();
    operator ID2D1DeviceContext **();
};

class EasyBuffer : public EasyBufferBase
{
public:
    ID2D1Bitmap1 *BufferBitmap;
    int width, height;
    mutex mtx;
    EasyBuffer(void);
    EasyBuffer(int width, int height);
    void StartUp(int width, int height);
    void DrawStart(void);
    void DrawEnd(void);
    void Render(int x, int y, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
    void Release(void);

    ~EasyBuffer();
};

class EasyPaintingScreenBuffer : public EasyBufferBase
{
    friend void EasyPaintingStart(HWND window, int WindowWidth, int WindowHeight, EasyPixel BackColor);

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
    vector<ID2D1Effect *> effects;
    ID2D1Bitmap1 *bitmap;
    ID2D1DeviceContext *pRenderTarget;

    void StartUp(EasySurface *surface, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
    void StartUp(ID2D1Bitmap1 *bitmap, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
    void SetInput(ID2D1Bitmap1 *bitmap, UINT32 index);
    void SetInput(ID2D1Effect *effect, UINT32 index);
    template <typename T>
    void SetValue(UINT32 prop, T &&value);
    void PushEffect(REFCLSID effectId);
    void PopEffect(void);
    void Render(int x, int y, float rotation = 0, int midpointx = 0, int midpointy = 0, int reverse = 0);
    void Release(void);

    void PushGaussianBlur(float standardDeviation);
    void PushScale(float scaleX, float scaleY);
    void PushBrightness(float brightness);
    typedef D2D1_MATRIX_5X4_F ColorMatrix;
    void PushColorMatrix(ColorMatrix matrix);
    void PushShadow(float standardDeviation, EasyPixel color);
    void PushCrop(int x, int y, int width, int height);

    operator ID2D1Effect *();
    ~EasyEffect();
};

// EasyPainting GeometryPath
class EasyGeometryPath
{
public:
    ID2D1PathGeometry *pathGeometry = nullptr;
    ID2D1GeometrySink *sink = nullptr;

    void StartUp();
    void FigureStart(float x, float y);
    void FigureEnd(bool close = false);
    void AddLine(float x, float y);
    void DrawGeometry(EasyPixel color, float thickness, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
    void FillGeometry(EasyPixel color, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
    void Release();

    ~EasyGeometryPath();
};

class EasyPaintingDevice
{
    friend void EasyPaintingStart(HWND window, int WindowWidth, int WindowHeight, EasyPixel BackColor);

private:
    EasyPaintingDevice();
    EasyPaintingDevice(const EasyPaintingDevice &other) = delete;
    EasyPaintingDevice &operator=(const EasyPaintingDevice &other) = delete;

public:
    void SetVSync(bool vsync);

    void SetWindow(HWND window, int WindowWidth, int WindowHeight, EasyPixel BackColor = RGB(255, 255, 255));

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

void EasyDrawRectangle(int x, int y, int width, int height, EasyPixel color, float thickness, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
void EasyDrawLine(DXY point0, DXY point1, EasyPixel color, float thickness, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget, D2D1_CAP_STYLE capStyle = D2D1_CAP_STYLE_ROUND, D2D1_DASH_STYLE dashStyle = D2D1_DASH_STYLE_SOLID);
void EasyDrawRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float thickness, float radiusX, float radiusY, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
void EasyDrawEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY, float thickness, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);

void EasyFillRectangle(int x, int y, int width, int height, EasyPixel color, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
void EasyFillRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float radiusX, float radiusY, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);
void EasyFillEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY, ID2D1DeviceContext *pRenderTarget = EasyPainting::pRenderTarget);

bool SpritePeek(SPRITE sprite1, SPRITE sprite2);
bool SpritePeekLine(SPRITE first, SPRITE second);
DXY MouseWinDetailedXY(void);
#define MouseWinXY MouseWinDetailedXY
bool KeyIt(SPRITE in);

ID2D1Bitmap *LoadBitmapFromFile(string file_path, int width, int height, EasyPixel transparent_color, ID2D1RenderTarget *pRenderTarget = EasyPainting::pRenderTarget);
ID2D1Bitmap *CreateBitmapFromArray(ID2D1RenderTarget *pRenderTarget, const BYTE *pixelData, UINT width, UINT height);
ID2D1Bitmap *CreatePureColorBitmap(ID2D1RenderTarget *pRenderTarget, EasyPixel color, int width, int height);
ID2D1Bitmap *CreateNullBitmap(ID2D1RenderTarget *pRenderTarget, UINT width, UINT height);
void RenderBitmap(ID2D1RenderTarget *pRenderTarget, ID2D1Bitmap *pBitmap, float x, float y);

ID2D1Bitmap1 *LoadBitmap1FromFile(string file_path, int width, int height, EasyPixel transparent_color, ID2D1DeviceContext *DeviceContext = EasyPainting::pRenderTarget);
ID2D1Bitmap1 *CreateBitmap1FromArray(ID2D1DeviceContext *pDeviceContext, const BYTE *pixelData, UINT width, UINT height);
ID2D1Bitmap1 *CreatePureColorBitmap1(ID2D1DeviceContext *pDeviceContext, EasyPixel color, int width, int height);
ID2D1Bitmap1 *CreateNullBitmap1(ID2D1DeviceContext *pDeviceContext, UINT width, UINT height);
void RenderBitmap1(ID2D1DeviceContext *pDeviceContext, ID2D1Bitmap1 *pBitmap, float x, float y);

void EasyLoadBitmapFromFile(string file_path, vector<vector<EasyPixel>> &vec);
void EasySaveBitmapToFile(string file_path, vector<vector<EasyPixel>> &vec);

void EasyCreateConsoleWindow();

ID2D1DeviceContext *EasyGetScreenBuffer();

// EasyPainting Math
extern const double PI;
extern const double PI_over_180;
extern const double PI_under_180;
inline double toRadians(double degrees);
inline double toDegrees(double radians);
#define val_distance(a, b) (a > b ? (a - b) : (b - a))
#define TimeIt(a) ((a) * (a))
double GoRotation(doubleXY x, doubleXY y);
double LinearVelx(double angle);
double LinearVely(double angle);
int DXYDistance(DXY first, DXY second);
double EasyPointDistance(EasyPoint first, EasyPoint second);

// const number
extern const int EASY_TURNLR;
extern const int EASY_TURNUD;

extern const int EASY_LINEAR_MODE;
extern const int EASY_NEAREST_MODE;

void EasyPaintingStart(HWND window, int WindowWidth, int WindowHeight, EasyPixel BackColor = RGB(255, 255, 255));
void DrawStart(void);
void DrawEnd();
