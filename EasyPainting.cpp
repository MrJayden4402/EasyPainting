// EasyPainting.cpp 10.0 Celebrate the third anniversary
#include "EasyPainting.h"

namespace EasyPainting
{
    ID2D1Factory1 *pFactory = nullptr;
    ID2D1DeviceContext *pRenderTarget = nullptr;
    IDWriteFactory *pWriteFactory = nullptr;
    IWICImagingFactory *pWICFactory = nullptr;

    ID3D11Device *pD3dDevice = nullptr;
    ID3D11DeviceContext *pD3dContext = nullptr;
    IDXGISwapChain1 *pSwapChain = nullptr;
    ID2D1Device *pD2dDevice = nullptr;
    ID2D1Bitmap1 *pTargetBitmap = nullptr;

    iconv_t iconv_cd;

    HWND WINDOW;

    double ConversionWidth = 1;
    double ConversionHeight = 1;
    int WINDOW_WIDTH;
    int WINDOW_HEIGHT;
    vector<pair<SURFACE *, string>> LoadSURFACE;

    vector<pair<EasyFont *, __Easy_Font_Info>> LoadFont;
    vector<EasyBuffer *> LoadBuffer;

    int hWindowWidth, hWindowHeight;
    COLORREF hBackColor;

    bool EasyPaintingStartFlag = false;

    bool enableVSync = true;

    mutex fpsMutex;
    int fpsCounter = 0;
    int showFPS = 0;

    D2D1_BITMAP_INTERPOLATION_MODE easyInterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
};

// const number
const int EASY_TURNLR = 1;
const int EASY_TURNUD = 2;

const int EASY_LINEAR_MODE = 1;
const int EASY_NEAREST_MODE = 2;

EasyPaintingScreenBuffer *easyScreenBuffer = nullptr;

EasyPaintingDevice *easyPaintingDevice = nullptr;

void __Easy_SetWindow(HWND window, int WindowWidth, int WindowHeight)
{
    using namespace EasyPainting;

    WINDOW_WIDTH = WindowWidth;
    WINDOW_HEIGHT = WindowHeight;
    WINDOW = window;
    hWindowWidth = WindowWidth;
    hWindowHeight = WindowHeight;

    static bool first = true;

    static IDXGIDevice *dxgiDevice = nullptr;
    static IDXGIAdapter *adapter = nullptr;
    static IDXGIFactory2 *dxgiFactory = nullptr;

    if (first)
    {
        // 获取 DXGI 设备
        pD3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);

        // 获取 DXGI 工厂
        dxgiDevice->GetAdapter(&adapter);

        adapter->GetParent(__uuidof(IDXGIFactory2), (void **)&dxgiFactory);
    }

    // 创建 SwapChain
    DXGI_SWAP_CHAIN_DESC1 swapDesc = {};
    swapDesc.Width = WindowWidth;
    swapDesc.Height = WindowHeight;
    swapDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.BufferCount = 2;
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    dxgiFactory->CreateSwapChainForHwnd(
        pD3dDevice,
        window,
        &swapDesc,
        nullptr,
        nullptr,
        &pSwapChain);

    // 获取后台缓冲区
    IDXGISurface *dxgiSurface = nullptr;

    pSwapChain->GetBuffer(0, __uuidof(IDXGISurface), (void **)&dxgiSurface);

    // 创建 D2D Bitmap1
    D2D1_BITMAP_PROPERTIES1 bitmapProps = {};
    bitmapProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    bitmapProps.dpiX = 96.0f;
    bitmapProps.dpiY = 96.0f;
    bitmapProps.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;

    pRenderTarget->CreateBitmapFromDxgiSurface(dxgiSurface, &bitmapProps, &pTargetBitmap);

    dxgiSurface->Release();

    // 设置绘图目标
    pRenderTarget->SetTarget(pTargetBitmap);

    first = false;
}

void EasyPaintingStart(HWND window, int WindowWidth, int WindowHeight, EasyPixel BackColor)
{
    using namespace EasyPainting;

    if (EasyPaintingStartFlag)
        return;
    EasyPaintingStartFlag = true;

    CoInitializeEx(nullptr, COINIT_MULTITHREADED); // 初始化 COM，指定为多线程
    CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWICFactory));

    iconv_cd = iconv_open("WCHAR_T", "GBK");
    if (iconv_cd == (iconv_t)-1)
        throw runtime_error("Failed to open iconv descriptor.");

    if (window == NULL)
        return;

    WINDOW = window;
    RECT rctA;                           // 定义一个RECT结构体，存储窗口的宽高
    GetClientRect(window, &rctA);        // 通过窗口句柄获得窗口的大小存储在rctA结构中
    int width = rctA.right - rctA.left;  // 窗口的宽度
    int height = rctA.bottom - rctA.top; // 窗口的高度
    WINDOW_WIDTH = width;
    WINDOW_HEIGHT = height;
    hWindowWidth = WindowWidth;
    hWindowHeight = WindowHeight;
    hBackColor = BackColor;
    if (hWindowWidth && hWindowHeight)
    {
        ConversionWidth = 1.0 * width / hWindowWidth;
        ConversionHeight = 1.0 * height / hWindowHeight;
    }

    D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, &pFactory);
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown **)&pWriteFactory);

    // 1. 创建 D3D11 设备
    D3D_FEATURE_LEVEL featureLevel;
    D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        D3D11_CREATE_DEVICE_BGRA_SUPPORT,
        nullptr, 0,
        D3D11_SDK_VERSION,
        &pD3dDevice,
        &featureLevel,
        &pD3dContext);

    // 创建 D2D 设备
    IDXGIDevice *dxgiDevice = nullptr;
    pD3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void **)&dxgiDevice);
    pFactory->CreateDevice(dxgiDevice, &pD2dDevice);
    dxgiDevice->Release();

    // 创建 D2D DeviceContext
    pD2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &pRenderTarget);

    __Easy_SetWindow(window, WindowWidth, WindowHeight);

    for (int i = 0; i < LoadBuffer.size(); i++)
        LoadBuffer[i]->StartUp(LoadBuffer[i]->width, LoadBuffer[i]->height);
    for (int i = 0; i < LoadSURFACE.size(); i++)
        LoadSURFACE[i].first->LoadIt(LoadSURFACE[i].second);
    for (int i = 0; i < LoadFont.size(); i++)
        LoadFont[i].first->LoadIt(LoadFont[i].second);

    easyScreenBuffer = new EasyPaintingScreenBuffer();
    easyScreenBuffer->pBufferDevice = EasyPainting::pRenderTarget;

    easyPaintingDevice = new EasyPaintingDevice();

    auto fpsCounterFunc = []()
    {
        using namespace EasyPainting;
        while (true)
        {
            this_thread::sleep_for(chrono::seconds(1));
            lock_guard<mutex> lock(fpsMutex);
            showFPS = fpsCounter;
            fpsCounter = 0;
        }
    };

    thread fpsThread(fpsCounterFunc);
    fpsThread.detach();
}

// WinGDI Functions
HBITMAP GetScalingBitmap_Delete(HBITMAP p_bitmap, int width, int height, int OldWidth, int OldHeight)
{
    HDC l_srcDc = CreateCompatibleDC(NULL);
    SelectObject(l_srcDc, p_bitmap);
    HDC l_dstDc = CreateCompatibleDC(l_srcDc);
    HBITMAP l_newBitmap = CreateCompatibleBitmap(l_srcDc, width, height);
    HBITMAP l_oldBitmap = (HBITMAP)::SelectObject(l_dstDc, l_newBitmap);
    SetStretchBltMode(l_dstDc, HALFTONE);
    StretchBlt(l_dstDc, 0, 0, width, height, l_srcDc, 0, 0, OldWidth, OldHeight, SRCCOPY);
    HBITMAP l_clippedBitmap = (HBITMAP)::SelectObject(l_dstDc, l_oldBitmap);
    DeleteDC(l_srcDc);
    DeleteDC(l_dstDc);
    DeleteObject(p_bitmap);
    return l_clippedBitmap;
}
HBITMAP MaskBitmap(HBITMAP p_bitmap, int width, int height, COLORREF maskColor)
{
    HDC l_dstDc = CreateCompatibleDC(NULL);
    HDC l_srcDc = CreateCompatibleDC(l_dstDc);
    HBITMAP l_newBitmap = CreateCompatibleBitmap(l_srcDc, width, height);
    HBITMAP l_oldBitmap = (HBITMAP)SelectObject(l_dstDc, l_newBitmap);
    HBITMAP maskBmp = CreateBitmap(width, height, 1, 1, NULL);
    SelectObject(l_dstDc, maskBmp);
    SelectObject(l_srcDc, p_bitmap);
    SetBkColor(l_srcDc, maskColor);
    BitBlt(l_dstDc, 0, 0, width, height, l_srcDc, 0, 0, NOTSRCCOPY);
    HBITMAP l_clippedBitmap = (HBITMAP)SelectObject(l_dstDc, l_oldBitmap);
    DeleteDC(l_srcDc);
    DeleteDC(l_dstDc);
    DeleteObject(l_newBitmap);
    DeleteObject(l_oldBitmap);
    return l_clippedBitmap;
}
HBITMAP CreateMaskBitmapWithTolerance(HBITMAP hSourceBitmap, COLORREF transparentColor, int tolerance)
{
    // 获取源位图的信息
    BITMAP sourceBitmapInfo;
    GetObject(hSourceBitmap, sizeof(BITMAP), &sourceBitmapInfo);

    // 创建与源位图大小相同的目标位图和设备上下文
    HDC hdcSource = CreateCompatibleDC(NULL);
    HDC hdcDest = CreateCompatibleDC(hdcSource);

    HBITMAP hMaskBitmap = CreateCompatibleBitmap(hdcSource, sourceBitmapInfo.bmWidth, sourceBitmapInfo.bmHeight);

    // 将目标位图选入设备上下文
    SelectObject(hdcSource, hSourceBitmap);
    SelectObject(hdcDest, hMaskBitmap);

    // 使用透明色创建透明掩码并容忍一定的色差
    for (int y = 0; y < sourceBitmapInfo.bmHeight; y++)
    {
        for (int x = 0; x < sourceBitmapInfo.bmWidth; x++)
        {
            COLORREF pixelColor = GetPixel(hdcSource, x, y);

            // 计算像素颜色与透明色之间的色差
            int redDiff = abs(GetRValue(pixelColor) - GetRValue(transparentColor));
            int greenDiff = abs(GetGValue(pixelColor) - GetGValue(transparentColor));
            int blueDiff = abs(GetBValue(pixelColor) - GetBValue(transparentColor));

            // 判断是否在色差容忍度内，并根据条件设置像素为透明色或非透明色
            if (redDiff <= tolerance && greenDiff <= tolerance && blueDiff <= tolerance)
                SetPixel(hdcDest, x, y, RGB(0, 0, 0)); // 设置为透明色
            else
                SetPixel(hdcDest, x, y, RGB(255, 255, 255)); // 设置为非透明色
        }
    }

    // 清理资源
    DeleteDC(hdcSource);
    DeleteDC(hdcDest);

    return hMaskBitmap;
}
HBITMAP GetScalingBitmap(HBITMAP p_bitmap, int width, int height, int OldWidth, int OldHeight)
{
    HDC l_srcDc = CreateCompatibleDC(NULL);
    SelectObject(l_srcDc, p_bitmap);
    HDC l_dstDc = CreateCompatibleDC(l_srcDc);
    HBITMAP l_newBitmap = CreateCompatibleBitmap(l_srcDc, width, height);
    HBITMAP l_oldBitmap = (HBITMAP)::SelectObject(l_dstDc, l_newBitmap);
    SetStretchBltMode(l_dstDc, HALFTONE);
    StretchBlt(l_dstDc, 0, 0, width, height, l_srcDc, 0, 0, OldWidth, OldHeight, SRCCOPY);
    HBITMAP l_clippedBitmap = (HBITMAP)::SelectObject(l_dstDc, l_oldBitmap);
    DeleteDC(l_srcDc);
    DeleteDC(l_dstDc);
    return l_clippedBitmap;
}
HBITMAP CreatePureColorBitmap(COLORREF cNewColor, LONG width, LONG height)
{
    HBITMAP RetBmp = NULL;
    HDC DirectDC = CreateCompatibleDC(NULL);
    if (DirectDC)
    {
        BITMAPINFO RGB32BitsBITMAPINFO;
        ZeroMemory(&RGB32BitsBITMAPINFO, sizeof(BITMAPINFO));
        RGB32BitsBITMAPINFO.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        RGB32BitsBITMAPINFO.bmiHeader.biWidth = width;
        RGB32BitsBITMAPINFO.bmiHeader.biHeight = height;
        RGB32BitsBITMAPINFO.bmiHeader.biPlanes = 1;
        RGB32BitsBITMAPINFO.bmiHeader.biBitCount = 32;
        UINT *ptPixels;
        HBITMAP DirectBitmap = CreateDIBSection(DirectDC,
                                                (BITMAPINFO *)&RGB32BitsBITMAPINFO,
                                                DIB_RGB_COLORS,
                                                (void **)&ptPixels,
                                                NULL, 0);
        if (DirectBitmap)
        {
            HGDIOBJ PreviousObject = SelectObject(DirectDC, DirectBitmap);
            cNewColor = COLORREF(cNewColor);
            for (int i = ((width * height) - 1); i >= 0; i--)
                ptPixels[i] = cNewColor;
            SelectObject(DirectDC, PreviousObject);
            RetBmp = DirectBitmap;
        }
        DeleteDC(DirectDC);
    }
    return RetBmp;
}

DXY GetBitmapWidthHeight(HBITMAP image)
{
    BITMAP bm;
    GetObject(image, sizeof(BITMAP), &bm);
    return DXY(bm.bmWidth, bm.bmHeight);
}
HBITMAP LoadBitmapFromFile(string filename)
{
    return (HBITMAP)LoadImage(0, filename.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}
void DrawhBitmap(HDC device, HBITMAP image, int x, int y)
{
    BITMAP bm;
    GetObject(image, sizeof(BITMAP), &bm);
    HDC hdcImage = CreateCompatibleDC(device);
    SelectObject(hdcImage, image);
    BitBlt(device, x, y, bm.bmWidth, bm.bmHeight, hdcImage, 0, 0, SRCCOPY);
    DeleteDC(hdcImage);
}
void DrawhBitmapWithMatte(HDC device, HBITMAP image, int x, int y, HBITMAP matte)
{
    BITMAP bm;
    GetObject(image, sizeof(BITMAP), &bm);
    POINT cube[3];
    cube[0].x = x;
    cube[0].y = y;
    cube[1].x = x + bm.bmWidth;
    cube[1].y = y;
    cube[2].x = x;
    cube[2].y = y + bm.bmHeight;
    HDC DrawPicture = CreateCompatibleDC(device);
    SelectObject(DrawPicture, image);
    PlgBlt(device, &cube[0], DrawPicture, 0, 0, bm.bmWidth, bm.bmHeight, matte, 0, 0);
    DeleteDC(DrawPicture);
}
HBITMAP GetSelectedBitmap(HBITMAP p_bitmap, int x, int y, int width, int height)
{
    HDC l_srcDc = CreateCompatibleDC(NULL);
    SelectObject(l_srcDc, p_bitmap);
    HDC l_dstDc = CreateCompatibleDC(l_srcDc);
    HBITMAP l_newBitmap = CreateCompatibleBitmap(l_srcDc, width, height);
    HBITMAP l_oldBitmap = (HBITMAP)SelectObject(l_dstDc, l_newBitmap);
    BitBlt(l_dstDc, 0, 0, width, height, l_srcDc, x, y, SRCCOPY);
    HBITMAP l_clippedBitmap = (HBITMAP)SelectObject(l_dstDc, l_oldBitmap);
    DeleteDC(l_srcDc);
    DeleteDC(l_dstDc);
    return l_clippedBitmap;
}
void DrawBitmap(HDC device, HBITMAP image, int x, int y, HBITMAP matte, int rotation)
{
    BITMAP bm;
    GetObject(image, sizeof(BITMAP), &bm);
    POINT cube[3];
    int width = bm.bmWidth, height = bm.bmHeight;
    // 计算旋转
    int mx, my;
    double xyde;
    int GoRo;
    if (rotation == 0)
    {
        cube[0].x = x;
        cube[0].y = y;
        cube[1].x = x + width;
        cube[1].y = y;
        cube[2].x = x;
        cube[2].y = y + height;
        goto DRAW;
    }
    if (rotation == 180) // 180会使PlgBlt出问题,但只要偏离一点点就可以了(2024.4.8)
    {
        cube[0].x = x + width + 1; // 所以加一
        cube[0].y = y + height;
        cube[1].x = x;
        cube[1].y = y + height;
        cube[2].x = x + width;
        cube[2].y = y;
        goto DRAW;
    }
    rotation = rotation == 180 ? 179 : rotation; // 180会使三角函数出问题,切成179(2024.4.7)
    {
        mx = x + width / 2;                                                                // 中点x
        my = y + height / 2;                                                               // 中点y
        xyde = sqrt(TimeIt(width / 2) + TimeIt(height / 2));                               // 中点到顶点的距离
        GoRo = toDegrees(atan((long double)((double)(height / 2) / (double)(width / 2)))); // 中点到顶点的角度
        int RoTo[3] = {(rotation + GoRo + 180) % 360, (360 - GoRo + rotation) % 360, (180 - GoRo + rotation) % 360};
        for (int i = 0; i < 3; i++)
        {
            cube[i].x = mx + LinearVelx(RoTo[i]) * xyde;
            cube[i].y = mx + LinearVely(RoTo[i]) * xyde;
        }
    }
DRAW:
    HDC DrawPicture = CreateCompatibleDC(device);
    SelectObject(DrawPicture, image);
    PlgBlt(device, &cube[0], DrawPicture, 0, 0, bm.bmWidth, bm.bmHeight, matte, 0, 0);
    DeleteDC(DrawPicture);
}
HPEN EasyCreatePen(COLORREF color, int cWidth)
{
    return CreatePen(PS_SOLID, cWidth, color);
}
void DrawLine(HDC device, HPEN pen, DXY f, DXY t)
{
    MoveToEx(device, f.x, f.y, NULL);
    SelectObject(device, pen);
    LineTo(device, t.x, t.y);
}
void DrawRect(HDC device, HPEN pen, int x, int y, int width, int height)
{
    SelectObject(device, pen);
    Rectangle(device, x, y, x + width, y + height);
}
void DrawCircle(HDC device, HPEN pen, int x, int y)
{
    MoveToEx(device, x, y, NULL);
    SelectObject(device, pen);
    LineTo(device, x, y);
}
inline void GetDevice(HWND window, HDC *device) { (*device) = GetDC(window); }

DXY::DXY(int x, int y) : x(x), y(y) {}
DXY::DXY() {}
doubleXY::doubleXY(double x, double y) : x(x), y(y) {}
doubleXY::doubleXY() {}
DXY::operator doubleXY() { return doubleXY{this->x, this->y}; }
doubleXY::operator DXY() { return DXY{this->x, this->y}; }
template <class T>
templateXY<T>::templateXY(T x, T y) : x(x), y(y) {}
template <class T>
templateXY<T>::templateXY() {}
const double PI = 3.1415926536;
const double PI_over_180 = PI / 180.0f;
const double PI_under_180 = 180.0f / PI;
inline double toRadians(double degrees)
{
    return degrees * PI_over_180;
}
inline double toDegrees(double radians)
{
    return radians * PI_under_180;
}

double GoRotation(doubleXY x, doubleXY y)
{
    if (x.x == y.x)
        return x.y >= y.y ? 270 : 90; // 垂直方向：下为90，上为270

    double deg = toDegrees(atan((double)(abs(x.y - y.y)) / abs(x.x - y.x))) * (((x.x >= y.x) == (x.y >= y.y)) ? -1 : 1);
    double angle = (x.x >= y.x) ? (180 + deg) : (x.y >= y.y ? deg : 360 + deg);

    // 将逆时针角度转换为顺时针角度
    angle = fmod(360 - angle, 360);
    return angle;
}

double LinearVelx(double angle)
{
    if (angle < 0)
        angle += 360;
    return cos((long double)(angle * PI_over_180));
}
double LinearVely(double angle)
{
    if (angle < 0)
        angle += 360;
    return sin((long double)(angle * PI_over_180));
}
int DXYDistance(DXY first, DXY second)
{
    int vx = val_distance(first.x, second.x);
    int vy = val_distance(first.y, second.y);
    return sqrt(vx * vx + vy * vy);
}
double EasyPointDistance(EasyPoint first, EasyPoint second)
{
    return sqrt((first.x - second.x) * (first.x - second.x) + (first.y - second.y) * (first.y - second.y));
}

SPRITE::SPRITE()
{
    x = y = 0;
    width = height = 0;
    scaling = 1.0f;
    rotation = 0.0f;
    velx = vely = 0.0f;
}
SPRITE::SPRITE(int x_, int y_, int width_, int height_)
{
    x = x_;
    y = y_;
    width = width_;
    height = height_;
    scaling = 1.0f;
    rotation = 0.0f;
    velx = vely = 0.0f;
}
bool SpritePeek(SPRITE sprite1, SPRITE sprite2)
{
    RECT rect1;
    rect1.left = (long)sprite1.x;
    rect1.top = (long)sprite1.y;
    rect1.right = (long)sprite1.x + sprite1.width * sprite1.scaling;
    rect1.bottom = (long)sprite1.y + sprite1.height * sprite1.scaling;
    RECT rect2;
    rect2.left = (long)sprite2.x;
    rect2.top = (long)sprite2.y;
    rect2.right = (long)sprite2.x + sprite2.width * sprite2.scaling;
    rect2.bottom = (long)sprite2.y + sprite2.height * sprite2.scaling;
    RECT dest;
    return (bool)IntersectRect(&dest, &rect1, &rect2);
}
bool SpritePeekLine(SPRITE first, SPRITE second)
{
    int line = sqrt(TimeIt(val_distance(first.x, second.x)) +
                    TimeIt(val_distance(first.y, second.y)));
    int rfirst = max(first.width, first.height) * first.scaling / 2;
    int rsecond = max(second.width, second.height) * second.scaling / 2;
    return (line < rfirst + rsecond);
}
DXY MouseWinRawXY(void)
{
    using namespace EasyPainting;
    HWND hWnd = WINDOW;
    for (POINT pt = {};; Sleep(1))
    {
        POINT pt_new;
        GetCursorPos(&pt_new);
        if (pt_new.x == pt.x && pt_new.y == pt.y)
            continue;
        pt = pt_new;
        {
            POINT tmp = pt;
            ScreenToClient(hWnd, &tmp);
            return DXY(tmp.x, tmp.y);
        }
    }
}
DXY MouseWinDetailedXY(void)
{
    using namespace EasyPainting;
    DXY tmp = MouseWinRawXY();
    return DXY(tmp.x / ConversionWidth, tmp.y / ConversionHeight);
}
#define MouseWinXY MouseWinDetailedXY
bool KeyIt(SPRITE in)
{
    using namespace EasyPainting;
    DXY mxy = MouseWinRawXY();
    int x = in.x;
    int y = in.y;
    int width = in.width * in.scaling;
    int height = in.height * in.scaling;
    x *= ConversionWidth;
    y *= ConversionHeight;
    width *= ConversionWidth;
    height *= ConversionHeight;
    SPRITE it(x, y, width, height);
    SPRITE mouse(mxy.x, mxy.y, 1, 1);
    return SpritePeek(mouse, it) && ((GetAsyncKeyState(MOUSE_MOVED) & 0x8000) ? 1 : 0);
}

ID2D1Bitmap *CreateBitmapFromArray(ID2D1RenderTarget *pRenderTarget, const BYTE *pixelData, UINT width, UINT height)
{
    // 设置位图属性 (BGRA 格式)
    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    // 计算每行字节数 (宽度 * 4 因为每像素4字节)
    UINT stride = width * 4;

    ID2D1Bitmap *pBitmap = nullptr;
    HRESULT hr = pRenderTarget->CreateBitmap(
        D2D1::SizeU(width, height), // 尺寸
        pixelData,                  // 源数据
        stride,                     // 每行字节数
        &props,                     // 格式属性
        &pBitmap                    // 输出位图
    );

    return SUCCEEDED(hr) ? pBitmap : nullptr;
}

ID2D1Bitmap1 *CreateBitmap1FromArray(ID2D1DeviceContext *pDeviceContext, const BYTE *pixelData, UINT width, UINT height)
{
    // 设置位图属性（BGRA + Premultiplied Alpha）
    D2D1_BITMAP_PROPERTIES1 props = {};
    props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
    props.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE; // 或 D2D1_BITMAP_OPTIONS_TARGET / CANNOT_DRAW 视用途而定
    props.dpiX = 96.0f;
    props.dpiY = 96.0f;

    UINT stride = width * 4;
    ID2D1Bitmap1 *pBitmap1 = nullptr;
    HRESULT hr = pDeviceContext->CreateBitmap(
        D2D1::SizeU(width, height),
        pixelData,
        stride,
        &props,
        &pBitmap1);

    return SUCCEEDED(hr) ? pBitmap1 : nullptr;
}

ID2D1Bitmap *CreatePureColorBitmap(ID2D1RenderTarget *pRenderTarget, EasyPixel color, int width, int height)
{
    BYTE *pixel = new BYTE[(width * height) << 2];
    for (UINT y = 0; y < height; y++)
        for (UINT x = 0; x < width; x++)
        {
            int index = (y * width + x) << 2;
            pixel[index + 3] = 255;
            pixel[index] = GetBValue(color);
            pixel[index + 1] = GetGValue(color);
            pixel[index + 2] = GetRValue(color);
        }

    // 设置位图属性 (BGRA 格式)
    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    // 计算每行字节数 (宽度 * 4 因为每像素4字节)
    UINT stride = width * 4;

    ID2D1Bitmap *pBitmap = nullptr;
    HRESULT hr = pRenderTarget->CreateBitmap(
        D2D1::SizeU(width, height), // 尺寸
        pixel,                      // 源数据
        stride,                     // 每行字节数
        &props,                     // 格式属性
        &pBitmap                    // 输出位图
    );

    delete[] pixel;

    return SUCCEEDED(hr) ? pBitmap : nullptr;
}

ID2D1Bitmap1 *CreatePureColorBitmap1(ID2D1DeviceContext *pDeviceContext, EasyPixel color, int width, int height)
{
    // 分配 BGRA 像素数据
    BYTE *pixel = new BYTE[(width * height) << 2];
    for (UINT y = 0; y < height; y++)
        for (UINT x = 0; x < width; x++)
        {
            int index = (y * width + x) << 2;
            pixel[index + 0] = color.b; // Blue
            pixel[index + 1] = color.g; // Green
            pixel[index + 2] = color.r; // Red
            pixel[index + 3] = color.a; // Alpha
        }

    // 设置位图属性（BGRA + Premultiplied Alpha）
    D2D1_BITMAP_PROPERTIES1 props = {};
    props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
    props.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE; // 或 D2D1_BITMAP_OPTIONS_TARGET / CANNOT_DRAW 视用途而定
    props.dpiX = 96.0f;
    props.dpiY = 96.0f;

    UINT stride = width * 4;
    ID2D1Bitmap1 *pBitmap1 = nullptr;
    HRESULT hr = pDeviceContext->CreateBitmap(
        D2D1::SizeU(width, height),
        pixel,
        stride,
        &props,
        &pBitmap1);

    delete[] pixel;
    return SUCCEEDED(hr) ? pBitmap1 : nullptr;
}

ID2D1Bitmap *CreateNullBitmap(ID2D1RenderTarget *pRenderTarget, UINT width, UINT height)
{
    // 设置位图属性 (BGRA 格式)
    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED));

    // 计算每行字节数 (宽度 * 4 因为每像素4字节)
    UINT stride = width * 4;

    ID2D1Bitmap *pBitmap = nullptr;
    HRESULT hr = pRenderTarget->CreateBitmap(
        D2D1::SizeU(width, height), // 尺寸
        nullptr,                    // 源数据
        stride,                     // 每行字节数
        &props,                     // 格式属性
        &pBitmap                    // 输出位图
    );

    return SUCCEEDED(hr) ? pBitmap : nullptr;
}

ID2D1Bitmap1 *CreateNullBitmap1(ID2D1DeviceContext *pDeviceContext, UINT width, UINT height)
{
    // 设置位图属性（BGRA + Premultiplied Alpha）
    D2D1_BITMAP_PROPERTIES1 props = {};
    props.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED);
    props.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE; // 或 D2D1_BITMAP_OPTIONS_TARGET / CANNOT_DRAW 视用途而定
    props.dpiX = 96.0f;
    props.dpiY = 96.0f;

    UINT stride = width * 4;
    ID2D1Bitmap1 *pBitmap1 = nullptr;
    HRESULT hr = pDeviceContext->CreateBitmap(
        D2D1::SizeU(width, height),
        nullptr,
        stride,
        &props,
        &pBitmap1);

    return SUCCEEDED(hr) ? pBitmap1 : nullptr;
}

void RenderBitmap(ID2D1RenderTarget *pRenderTarget, ID2D1Bitmap *pBitmap, float x, float y)
{
    if (!pBitmap)
        return;

    // 获取位图原始尺寸
    D2D1_SIZE_F size = pBitmap->GetSize();

    // 设置目标矩形
    D2D1_RECT_F destRect = D2D1::RectF(
        x,
        y,
        x + size.width,
        y + size.height);

    pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());

    // 绘制位图
    pRenderTarget->DrawBitmap(
        pBitmap,
        destRect,
        1.0f,                               // 不透明度
        EasyPainting::easyInterpolationMode // 插值模式
    );
}

void RenderBitmap1(ID2D1DeviceContext *pDeviceContext, ID2D1Bitmap1 *pBitmap, float x, float y)
{
    if (!pBitmap)
        return;

    // 获取位图原始尺寸
    D2D1_SIZE_F size = pBitmap->GetSize();

    // 设置目标矩形
    D2D1_RECT_F destRect = D2D1::RectF(
        x,
        y,
        x + size.width,
        y + size.height);

    pDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());

    // 绘制位图
    pDeviceContext->DrawBitmap(
        pBitmap,
        destRect,
        1.0f,                               // 不透明度
        EasyPainting::easyInterpolationMode // 插值模式
    );
}

wstring __Easy_gbk_to_wstring(const string &input)
{
    using namespace EasyPainting;
    // 输入字符串和输出缓冲区
    const char *in_buf = input.c_str();
    size_t in_bytes_left = input.size();
    // 设置一个足够大的输出缓冲区
    size_t out_bytes_left = in_bytes_left * 2; // UTF-16需要更多空间
    vector<char> out_buf(out_bytes_left);
    char *out_ptr = out_buf.data();
    size_t out_left = out_bytes_left;
    // 调用iconv进行转换
    size_t result = iconv(iconv_cd, (char **)&in_buf, &in_bytes_left, &out_ptr, &out_left);
    if (result == (size_t)-1)
    {
        iconv_close(iconv_cd);
        throw runtime_error("Conversion failed.");
    }
    // 将输出缓冲区转换为wstring
    wstring wstr(reinterpret_cast<wchar_t *>(out_buf.data()), (out_bytes_left - out_left) / sizeof(wchar_t));

    return wstr;
}

template <typename DeviceContext, typename Bitmap>
void __Easy_LoadBitmapFromFile(string file_path, int width, int height, COLORREF transparent_color, DeviceContext *pDeviceContext, Bitmap **pBitmap)
{
    using namespace EasyPainting;
    wstring uri = __Easy_gbk_to_wstring(file_path);

    BYTE transparent_R = GetRValue(transparent_color);
    BYTE transparent_G = GetGValue(transparent_color);
    BYTE transparent_B = GetBValue(transparent_color);

    IWICBitmapDecoder *pDecoder = nullptr;
    IWICBitmapFrameDecode *pFrame = nullptr;

    HRESULT hr;
    if (SUCCEEDED(hr))
        hr = pWICFactory->CreateDecoderFromFilename(uri.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
    if (SUCCEEDED(hr))
        hr = pDecoder->GetFrame(0, &pFrame);

    // 创建格式转换器
    IWICFormatConverter *pConverter = nullptr;

    hr = pWICFactory->CreateFormatConverter(&pConverter);
    if (FAILED(hr))
    {
        pFrame->Release();
        pDecoder->Release();
        return;
    }
    pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut);

    // 获取图像的尺寸
    UINT ori_width, ori_height;
    pConverter->GetSize(&ori_width, &ori_height);

    if (width == 0 || height == 0)
        width = ori_width, height = ori_height; // 如果没有指定宽高，则使用原图尺寸

    // 创建一个缓冲区来存储像素数据
    BYTE *pixels = new BYTE[(ori_width * ori_height) << 2];

    // 复制像素数据
    UINT stride = ori_width << 2; // 每行字节数
    pConverter->CopyPixels(
        nullptr,                       // 指定行的宽度
        stride,                        // 每行字节数
        (ori_width * ori_height) << 2, // 输出缓冲区大小
        pixels                         // 输出缓冲区
    );
    if (pConverter)
        pConverter->Release();
    for (UINT y = 0; y < ori_height; y++)
        for (UINT x = 0; x < ori_width; x++)
        {
            UINT index = (y * ori_width + x) << 2;
            if (pixels[index + 2] == transparent_R && pixels[index + 1] == transparent_G && pixels[index + 0] == transparent_B)
                pixels[index + 3] = pixels[index + 2] = pixels[index + 1] = pixels[index + 0] = 0;
        }
    IWICBitmap *wicBitmap;
    hr = pWICFactory->CreateBitmapFromMemory(ori_width, ori_height, GUID_WICPixelFormat32bppPBGRA, ori_width * 4, ori_width * ori_height * 4, pixels, &wicBitmap);

    IWICBitmapScaler *pScaler = nullptr; // 缩放器
    hr = pWICFactory->CreateBitmapScaler(&pScaler);
    hr = pScaler->Initialize(wicBitmap, width, height, WICBitmapInterpolationModeNearestNeighbor);

    if (wicBitmap)
        wicBitmap->Release();

    if (SUCCEEDED(hr))
        hr = pDeviceContext->CreateBitmapFromWicBitmap(pScaler, pBitmap);

    if (pScaler)
        pScaler->Release();
    if (pFrame)
        pFrame->Release();
    if (pDecoder)
        pDecoder->Release();

    delete[] pixels;
}

ID2D1Bitmap *LoadBitmapFromFile(string file_path, int width, int height, EasyPixel transparent_color, ID2D1RenderTarget *pRenderTarget)
{
    ID2D1Bitmap *pBitmap = nullptr;
    __Easy_LoadBitmapFromFile(file_path, width, height, transparent_color, pRenderTarget, &pBitmap);
    return pBitmap;
}

ID2D1Bitmap1 *LoadBitmap1FromFile(string file_path, int width, int height, EasyPixel transparent_color, ID2D1DeviceContext *DeviceContext)
{
    ID2D1Bitmap1 *pBitmap = nullptr;
    __Easy_LoadBitmapFromFile(file_path, width, height, transparent_color, DeviceContext, &pBitmap);
    return pBitmap;
}

void EasyDrawRectangle(int x, int y, int width, int height, EasyPixel color, float thickness, ID2D1DeviceContext *pRenderTarget)
{
    ID2D1SolidColorBrush *brush;
    pRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f), &brush);
    pRenderTarget->DrawRectangle(D2D1::RectF(x, y, x + width, y + height), brush, thickness);
    brush->Release();
}

void EasyDrawRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float thickness, float radiusX, float radiusY, ID2D1DeviceContext *pRenderTarget)
{
    ID2D1SolidColorBrush *brush;
    pRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f), &brush);
    pRenderTarget->DrawRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(x, y, x + width, y + height), radiusX, radiusY), brush, thickness);
    brush->Release();
}

void EasyDrawLine(DXY point0, DXY point1, EasyPixel color, float thickness, ID2D1DeviceContext *pRenderTarget, D2D1_CAP_STYLE capStyle, D2D1_DASH_STYLE dashStyle)
{
    using namespace EasyPainting;

    D2D1_STROKE_STYLE_PROPERTIES props = {};
    props.startCap = capStyle;
    props.endCap = capStyle;
    props.dashStyle = dashStyle;

    ID2D1StrokeStyle *strokeStyle = nullptr;
    pFactory->CreateStrokeStyle(&props, nullptr, 0, &strokeStyle);

    ID2D1SolidColorBrush *brush;
    pRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f), &brush);
    D2D1_POINT_2F p0 = D2D1::Point2F(point0.x, point0.y);
    D2D1_POINT_2F p1 = D2D1::Point2F(point1.x, point1.y);

    pRenderTarget->DrawLine(p0, p1, brush, thickness, strokeStyle);

    strokeStyle->Release();
    brush->Release();
}

void EasyDrawEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY, float thickness, ID2D1DeviceContext *pRenderTarget)
{
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radiusX, radiusY);
    ID2D1SolidColorBrush *brush;
    pRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f), &brush);
    pRenderTarget->DrawEllipse(ellipse, brush, thickness);
    brush->Release();
}

void EasyFillRectangle(int x, int y, int width, int height, EasyPixel color, ID2D1DeviceContext *pRenderTarget)
{
    ID2D1SolidColorBrush *brush;
    pRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f), &brush);
    pRenderTarget->FillRectangle(D2D1::RectF(x, y, x + width, y + height), brush);
    brush->Release();
}

void EasyFillRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float radiusX, float radiusY, ID2D1DeviceContext *pRenderTarget)
{
    ID2D1SolidColorBrush *brush;
    pRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f), &brush);
    pRenderTarget->FillRoundedRectangle(D2D1::RoundedRect(D2D1::RectF(x, y, x + width, y + height), radiusX, radiusY), brush);
    brush->Release();
}

void EasyFillEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY, ID2D1DeviceContext *pRenderTarget)
{
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radiusX, radiusY);
    ID2D1SolidColorBrush *brush;
    pRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f), &brush);
    pRenderTarget->FillEllipse(ellipse, brush);
    brush->Release();
}

void __Easy_SaveIWICBitmapToFile(IWICBitmap *pBitmap, const LPCSTR filePath)
{
    using namespace EasyPainting;

    // 创建编码器
    IWICBitmapEncoder *pEncoder = nullptr;
    pWICFactory->CreateEncoder(GUID_ContainerFormatPng, nullptr, &pEncoder);

    // 创建文件流
    IStream *pStream = nullptr;
    SHCreateStreamOnFile(filePath, STGM_WRITE | STGM_CREATE, &pStream);
    // 初始化编码器
    pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);

    // 创建编码器的帧
    IWICBitmapFrameEncode *pFrameEncode = nullptr;
    pEncoder->CreateNewFrame(&pFrameEncode, nullptr);

    // 初始化帧
    pFrameEncode->Initialize(nullptr);

    // 将 bitmap 编码到文件
    pFrameEncode->WriteSource(pBitmap, nullptr);

    // 提交帧并保存文件
    pFrameEncode->Commit();
    pEncoder->Commit();

    // 清理资源
    pFrameEncode->Release();
    pEncoder->Release();
    pStream->Release();
}

// EasyPainting Bitmap Editor
void EasyLoadBitmapFromFile(string file_path, vector<vector<EasyPixel>> &vec)
{
    using namespace EasyPainting;

    wstring uri = __Easy_gbk_to_wstring(file_path);

    IWICBitmapDecoder *pDecoder = nullptr;
    IWICBitmapFrameDecode *pFrame = nullptr;

    pWICFactory->CreateDecoderFromFilename(uri.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);

    pDecoder->GetFrame(0, &pFrame);

    // 创建格式转换器
    IWICFormatConverter *pConverter = nullptr;

    pWICFactory->CreateFormatConverter(&pConverter);
    pConverter->Initialize(pFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut);

    // 获取图像的尺寸
    UINT ori_width, ori_height;
    pConverter->GetSize(&ori_width, &ori_height);

    // 创建一个缓冲区来存储像素数据
    BYTE *pixels = new BYTE[(ori_width * ori_height) << 2];

    // 复制像素数据
    UINT stride = ori_width << 2; // 每行字节数
    pConverter->CopyPixels(
        nullptr,                       // 指定行的宽度
        stride,                        // 每行字节数
        (ori_width * ori_height) << 2, // 输出缓冲区大小
        pixels                         // 输出缓冲区
    );
    if (pConverter)
        pConverter->Release();

    vec.resize(ori_width, vector<EasyPixel>(ori_height));

    for (UINT y = 0; y < ori_height; y++)
        for (UINT x = 0; x < ori_width; x++)
        {
            UINT index = (y * ori_width + x) << 2;
            EasyPixel p;
            p.a = pixels[index + 3];
            p.r = pixels[index + 2];
            p.g = pixels[index + 1];
            p.b = pixels[index + 0];
            vec[x][y] = p;
        }

    if (pFrame)
        pFrame->Release();
    if (pDecoder)
        pDecoder->Release();

    delete[] pixels;

    return;
}

void EasySaveBitmapToFile(string file_path, vector<vector<EasyPixel>> &vec)
{
    using namespace EasyPainting;

    if (vec.empty())
        return;

    const int ori_width = vec.size();
    const int ori_height = vec[0].size();

    BYTE *pixels = new BYTE[(ori_width * ori_height) << 2];

    for (UINT y = 0; y < ori_height; y++)
        for (UINT x = 0; x < ori_width; x++)
        {
            UINT index = (y * ori_width + x) << 2;
            EasyPixel p = vec[x][y];
            pixels[index + 3] = p.a;
            pixels[index + 2] = p.r;
            pixels[index + 1] = p.g;
            pixels[index + 0] = p.b;
        }

    IWICBitmap *wicBitmap;
    pWICFactory->CreateBitmapFromMemory(ori_width, ori_height, GUID_WICPixelFormat32bppPBGRA, ori_width * 4, ori_width * ori_height * 4, pixels, &wicBitmap);

    __Easy_SaveIWICBitmapToFile(wicBitmap, file_path.c_str());

    if (wicBitmap)
        wicBitmap->Release();

    delete[] pixels;

    return;
}

void EasyCreateConsoleWindow()
{
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
}

ID2D1DeviceContext *EasyGetScreenBuffer()
{
    return EasyPainting::pRenderTarget;
}

void DrawStart(void)
{
    using namespace EasyPainting;
    RECT rctA;                           // 定义一个RECT结构体，存储窗口的宽高
    GetClientRect(WINDOW, &rctA);        // 通过窗口句柄获得窗口的大小存储在rctA结构中
    int width = rctA.right - rctA.left;  // 窗口的宽度
    int height = rctA.bottom - rctA.top; // 窗口的高度
    ConversionWidth = 1.0 * width / hWindowWidth;
    ConversionHeight = 1.0 * height / hWindowHeight;
    pRenderTarget->BeginDraw();
    pRenderTarget->Clear(D2D1::ColorF(GetRValue(hBackColor) / 255.0, GetGValue(hBackColor) / 255.0, GetBValue(hBackColor) / 255.0));
}
void DrawEnd()
{
    using namespace EasyPainting;
    pRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
    pRenderTarget->EndDraw();
    pSwapChain->Present((UINT)enableVSync, 0);

    lock_guard<mutex> lock(fpsMutex);
    fpsCounter++;
}

SURFACE::SURFACE(string filename, int width, int height, EasyPixel MatteColor, ID2D1DeviceContext **pRenderTarget)
{
    this->Create(filename, width, height, MatteColor, pRenderTarget);
}

void SURFACE::LoadIt(string filename)
{
    this->image = nullptr;
    this->image = LoadBitmap1FromFile(filename.c_str(), this->Width, this->Height, this->MatteColor);
    if (this->Width == 0 || this->Height == 0)
    {
        D2D1_SIZE_F size = this->image->GetSize();
        this->image->GetSize(&size);
        this->Width = size.width;
        this->Height = size.height;
    }
}

SURFACE::SURFACE()
{
    this->image = nullptr;
    this->pRenderTarget = &EasyPainting::pRenderTarget;
}

void SURFACE::Create(string filename, int width, int height, EasyPixel MatteColor, ID2D1DeviceContext **pRenderTarget)
{
    this->Width = width;
    this->Height = height;
    this->MatteColor = MatteColor;
    this->pRenderTarget = pRenderTarget;
    if (!EasyPainting::EasyPaintingStartFlag)
        EasyPainting::LoadSURFACE.push_back({this, filename});
    else
        this->LoadIt(filename);
}

void SURFACE::SetRenderTarget(ID2D1DeviceContext **pRenderTarget)
{
    this->pRenderTarget = pRenderTarget;
}

void SURFACE::CreateFromMemory(vector<vector<EasyPixel>> &vec)
{
    using namespace EasyPainting;

    if (vec.empty())
        return;

    const int ori_width = vec.size();
    const int ori_height = vec[0].size();

    BYTE *pixels = new BYTE[(ori_width * ori_height) << 2];

    this->MatteColor = EasyPixel(0, 0, 0, 0);

    for (UINT y = 0; y < ori_height; y++)
        for (UINT x = 0; x < ori_width; x++)
        {
            UINT index = (y * ori_width + x) << 2;
            EasyPixel p = vec[x][y];

            if (p.r == this->MatteColor.r && p.g == this->MatteColor.g && p.b == this->MatteColor.b)
                p = EasyPixel(0, 0, 0, 0);

            pixels[index + 3] = p.a;
            pixels[index + 2] = p.r;
            pixels[index + 1] = p.g;
            pixels[index + 0] = p.b;
        }

    this->image = CreateBitmap1FromArray(*(this->pRenderTarget), pixels, ori_width, ori_height);

    this->Width = ori_width;
    this->Height = ori_height;

    delete[] pixels;
}

void SURFACE::CopyFromMemory(vector<vector<EasyPixel>> &vec)
{
    using namespace EasyPainting;

    if (vec.empty())
        return;

    const int ori_width = vec.size();
    const int ori_height = vec[0].size();

    BYTE *pixels = new BYTE[(ori_width * ori_height) << 2];

    for (UINT y = 0; y < ori_height; y++)
        for (UINT x = 0; x < ori_width; x++)
        {
            UINT index = (y * ori_width + x) << 2;
            EasyPixel p = vec[x][y];

            if (p.r == this->MatteColor.r && p.g == this->MatteColor.g && p.b == this->MatteColor.b)
                p = EasyPixel(0, 0, 0, 0);

            pixels[index + 3] = p.a;
            pixels[index + 2] = p.r;
            pixels[index + 1] = p.g;
            pixels[index + 0] = p.b;
        }

    this->image->CopyFromMemory(NULL, pixels, ori_width << 2);

    delete[] pixels;
}

SURFACE::~SURFACE(void)
{
    this->Release();
}

void SURFACE::DrawItEx(int x, int y, int width, int height, int imagex, int imagey, int imagewidth, int imageheight, float rotation, int midpointdx, int midpointdy, int reverse)
{
    if (this->image == nullptr)
        return;
    D2D1_RECT_F bitmap_rect = D2D1::RectF(imagex, imagey, imagex + imagewidth, imagey + imageheight);
    D2D1_RECT_F rect = D2D1::RectF(x, y, x + width, y + height);
    D2D1_POINT_2F center = {x + (width / 2.0f) + midpointdx, y + (height / 2.0f) + midpointdy};
    FLOAT angle = rotation; // 旋转角度，单位为度
    // 将角度转换为弧度
    D2D1_MATRIX_3X2_F Matrix = D2D1::Matrix3x2F::Rotation(angle, center);
    // 翻转
    center = {x + (width / 2.0f), y + (height / 2.0f)};
    if (reverse & EASY_TURNLR)
        Matrix = D2D1::Matrix3x2F::Scale(D2D1::SizeF(-1.0f, 1.0f), center) * Matrix;
    if (reverse & EASY_TURNUD)
        Matrix = D2D1::Matrix3x2F::Scale(D2D1::SizeF(1.0f, -1.0f), center) * Matrix;
    (*pRenderTarget)->SetTransform(Matrix);
    (*pRenderTarget)->DrawBitmap(this->image, &rect, 1.0f, EasyPainting::easyInterpolationMode, &bitmap_rect);
}

void SURFACE::DrawIt(int x, int y, int width, int height, float scaling, float rotation, int columns, int frames, int reverse)
{
    if (this->image == nullptr)
        return;
    if (width == 0 || height == 0)
        width = this->Width, height = this->Height;
    const int framex = (frames % columns) * width;
    const int framey = (frames / columns) * height;
    D2D1_RECT_F bitmap_rect = D2D1::RectF(framex, framey, framex + width, framey + height);
    width *= scaling;
    height *= scaling;
    D2D1_RECT_F rect = D2D1::RectF(x, y, x + width, y + height);
    D2D1_POINT_2F center = {x + (width / 2.0f), y + (height / 2.0f)};
    FLOAT angle = rotation; // 旋转角度，单位为度
    // 将角度转换为弧度
    FLOAT radians = angle * PI_over_180;
    D2D1_MATRIX_3X2_F Matrix = D2D1::Matrix3x2F::Rotation(angle, center);
    // 翻转
    if (reverse & EASY_TURNLR)
        Matrix = D2D1::Matrix3x2F::Scale(D2D1::SizeF(-1.0f, 1.0f), center) * Matrix;
    if (reverse & EASY_TURNUD)
        Matrix = D2D1::Matrix3x2F::Scale(D2D1::SizeF(1.0f, -1.0f), center) * Matrix;
    (*pRenderTarget)->SetTransform(Matrix);
    (*pRenderTarget)->DrawBitmap(this->image, &rect, 1.0f, EasyPainting::easyInterpolationMode, &bitmap_rect);
}

void SURFACE::DrawItDirect(int x, int y, int width, int height)
{
    this->DrawIt(x, y, width, height);
}
void SURFACE::DrawItFrames(int x, int y, int width, int height, int columns, int frames, float scaling)
{
    this->DrawIt(x, y, width, height, scaling, 0, columns, frames);
}
void SURFACE::DrawItReverse(int x, int y, int width, int height, int reverse, float scaling, int rotation)
{
    this->DrawIt(x, y, width, height, scaling, rotation, 1, 0, reverse);
}

void SURFACE::Release()
{
    this->Width = this->Height = 0;
    this->MatteColor = 0;
    this->pRenderTarget = NULL;
    if (this->image)
        this->image->Release();
    this->image = nullptr;
}

EasyFont::EasyFont(string FontName, int SIZE, DWRITE_FONT_WEIGHT FontWeight, DWRITE_FONT_STYLE FontStyle)
{
    this->Create(FontName, SIZE, FontWeight, FontStyle);
}

void EasyFont::Create(string FontName, int SIZE, DWRITE_FONT_WEIGHT FontWeight, DWRITE_FONT_STYLE FontStyle)
{
    if (!EasyPainting::EasyPaintingStartFlag)
        EasyPainting::LoadFont.push_back({this, {FontName, (float)SIZE, FontWeight, FontStyle}});
    else
        this->LoadIt({FontName, (float)SIZE, FontWeight, FontStyle});
}

IDWriteTextFormat *__Easy_CreateTextFormat(string font, int size, DWRITE_FONT_WEIGHT FontWeight, DWRITE_FONT_STYLE FontStyle)
{
    using namespace EasyPainting;
    wstring wfont = __Easy_gbk_to_wstring(font);
    IDWriteTextFormat *pTextFormat = nullptr;
    pWriteFactory->CreateTextFormat(wfont.c_str(), nullptr, FontWeight,
                                    FontStyle, DWRITE_FONT_STRETCH_NORMAL,
                                    size, L"zh-cn", &pTextFormat);
    return pTextFormat;
}
void EasyFont::LoadIt(EasyPainting::__Easy_Font_Info info)
{
    this->pFont = __Easy_CreateTextFormat(info.FontName, info.SIZE, info.FontWeight, info.FontStyle);
}
EasyFont::EasyFont(void) {}
void EasyFont::Print(string text, int x, int y, EasyPixel color, ID2D1DeviceContext *pRenderTarget)
{
    pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0, 0));
    ID2D1SolidColorBrush *pBrush = nullptr;
    pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f), &pBrush);
    wstring wtext = __Easy_gbk_to_wstring(text);
    pRenderTarget->DrawText(wtext.c_str(),                             // 要绘制的文本
                            wtext.length(),                            // 文本长度
                            this->pFont,                               // 文本格式
                            D2D1::RectF(x, y, 0x7fffffff, 0x7fffffff), // 绘制区域
                            pBrush                                     // 画笔
    );
    pBrush->Release();
}

void EasyFont::Release(void)
{
    if (this->pFont)
        this->pFont->Release();
    this->pFont = nullptr;
}

EasyFont::~EasyFont(void)
{
    this->Release();
}

EasyBuffer::EasyBuffer() {}

EasyBuffer::EasyBuffer(int width, int height)
{
    this->StartUp(width, height);
}

void EasyBuffer::StartUp(int width, int height)
{
    using namespace EasyPainting;
    this->width = width;
    this->height = height;

    if (!EasyPaintingStartFlag)
        return;

    pD2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &this->pBufferDevice);

    auto CreateBufferBitmap = [&](ID2D1DeviceContext *pDevice, int width, int height) -> ID2D1Bitmap1 *
    {
        D2D1_BITMAP_PROPERTIES1 props = {};
        props.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
        props.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
        props.dpiX = 96.0f;
        props.dpiY = 96.0f;
        props.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;

        ID2D1Bitmap1 *bitmap = nullptr;
        pDevice->CreateBitmap(D2D1::SizeU(width, height), nullptr, 0, &props, &bitmap);
        return bitmap;
    };

    this->BufferBitmap = CreateBufferBitmap(this->pBufferDevice, width, height);

    this->pBufferDevice->SetTarget(this->BufferBitmap);
}

void EasyBuffer::DrawStart()
{
    this->mtx.lock();
    this->pBufferDevice->BeginDraw();
}

void EasyBuffer::DrawEnd()
{
    this->pBufferDevice->SetTransform(D2D1::Matrix3x2F::Identity());
    this->pBufferDevice->EndDraw();
    this->mtx.unlock();
}

void EasyBufferBase::Clear(EasyPixel color)
{
    this->pBufferDevice->Clear(D2D1::ColorF(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f));
}

void EasyBuffer::Render(int x, int y, ID2D1DeviceContext *pRenderTarget)
{
    this->mtx.lock();
    pRenderTarget->DrawBitmap(this->BufferBitmap, D2D1::RectF(x, y, x + this->width, y + this->height));
    this->mtx.unlock();
}

void EasyBufferBase::DrawRectangle(int x, int y, int width, int height, EasyPixel color, float thickness)
{
    EasyDrawRectangle(x, y, width, height, color, thickness, this->pBufferDevice);
}

void EasyBufferBase::DrawLine(DXY point0, DXY point1, EasyPixel color, float thickness, D2D1_CAP_STYLE capStyle, D2D1_DASH_STYLE dashStyle)
{
    EasyDrawLine(point0, point1, color, thickness, this->pBufferDevice, capStyle, dashStyle);
}

void EasyBufferBase::DrawRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float thickness, float radiusX, float radiusY)
{
    EasyDrawRoundedRectangle(x, y, width, height, color, thickness, radiusX, radiusY, this->pBufferDevice);
}

void EasyBufferBase::DrawEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY, float thickness)
{
    EasyDrawEllipse(x, y, color, radiusX, radiusY, thickness, this->pBufferDevice);
}

void EasyBufferBase::FillRectangle(int x, int y, int width, int height, EasyPixel color)
{
    EasyFillRectangle(x, y, width, height, color, this->pBufferDevice);
}

void EasyBufferBase::FillRoundedRectangle(int x, int y, int width, int height, EasyPixel color, float radiusX, float radiusY)
{
    EasyFillRoundedRectangle(x, y, width, height, color, radiusX, radiusY, this->pBufferDevice);
}

void EasyBufferBase::FillEllipse(int x, int y, EasyPixel color, float radiusX, float radiusY)
{
    EasyFillEllipse(x, y, color, radiusX, radiusY, this->pBufferDevice);
}

EasyBufferBase::operator ID2D1DeviceContext *()
{
    return this->pBufferDevice;
}

EasyBufferBase::operator ID2D1DeviceContext **() { return &this->pBufferDevice; }

void EasyBuffer::Release()
{
    if (this->BufferBitmap)
        this->BufferBitmap->Release();
    if (this->pBufferDevice)
        this->pBufferDevice->Release();
    this->pBufferDevice = nullptr;
    this->BufferBitmap = nullptr;
}

EasyBuffer::~EasyBuffer()
{
    this->Release();
}

EasyPixel::EasyPixel() {}
EasyPixel::EasyPixel(unsigned char a, unsigned char r, unsigned char g, unsigned char b) : a(a), r(r), g(g), b(b) {}
EasyPixel::EasyPixel(unsigned char r, unsigned char g, unsigned char b) : a(255), r(r), g(g), b(b) {}
EasyPixel::EasyPixel(COLORREF color) : a(255), r(GetRValue(color)), g(GetGValue(color)), b(GetBValue(color)) {}
EasyPixel::EasyPixel(D2D1_COLOR_F color) : a(color.a * 255.0), r(color.r * 255.0), g(color.g * 255.0), b(color.b * 255.0) {}
EasyPixel::operator COLORREF() { return RGB(this->r, this->g, this->b); }
EasyPixel::operator D2D1_COLOR_F() { return D2D1::ColorF(this->r / 255.0f, this->g / 255.0f, this->b / 255.0f, this->a / 255.0f); }

void EasyEffect::StartUp(ID2D1Bitmap1 *bitmap, ID2D1DeviceContext *pRenderTarget)
{
    this->bitmap = bitmap;
    this->pRenderTarget = pRenderTarget;
}

void EasyEffect::StartUp(EasySurface *surface, ID2D1DeviceContext *pRenderTarget)
{
    this->bitmap = surface->image;
    this->pRenderTarget = pRenderTarget;
}

void EasyEffect::PushEffect(REFCLSID effectId)
{
    ID2D1Effect *effect = nullptr;
    pRenderTarget->CreateEffect(effectId, &effect);

    if (this->effects.empty())
    {
        effect->SetInput(0, this->bitmap);
        this->effects.push_back(effect);
        return;
    }

    effect->SetInputEffect(0, this->effects.back());
    this->effects.push_back(effect);
}

void EasyEffect::PopEffect()
{
    if (this->effects.empty())
        return;

    this->effects.back()->Release();
    this->effects.pop_back();
}

void EasyEffect::SetInput(ID2D1Bitmap1 *bitmap, UINT32 index)
{
    if (this->effects.empty())
        return;

    this->effects.back()->SetInput(index, bitmap);
}

void EasyEffect::SetInput(ID2D1Effect *effect, UINT32 index)
{
    if (this->effects.empty())
        return;

    this->effects.back()->SetInputEffect(index, effect);
}

template <typename T>
void EasyEffect::SetValue(UINT32 prop, T &&value)
{
    if (this->effects.empty())
        return;

    this->effects.back()->SetValue(prop, forward<T>(value));
}

void EasyEffect::Release()
{
    while (!this->effects.empty())
    {
        this->effects.back()->Release();
        this->effects.pop_back();
    }
}

void EasyEffect::PushGaussianBlur(float standardDeviation)
{
    this->PushEffect(CLSID_D2D1GaussianBlur);

    const int D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION = 0;

    this->SetValue(D2D1_GAUSSIANBLUR_PROP_STANDARD_DEVIATION, 3.0f);
}

void EasyEffect::PushScale(float scaleX, float scaleY)
{
    this->PushEffect(CLSID_D2D1Scale);

    const int D2D1_SCALE_PROP_SCALE = 0;

    this->SetValue(D2D1_SCALE_PROP_SCALE, D2D1::Vector2F(scaleX, scaleY));
}

void EasyEffect::PushBrightness(float brightness)
{
    D2D1_MATRIX_5X4_F matrix =
        {
            brightness, 0, 0, 0,
            0, brightness, 0, 0,
            0, 0, brightness, 0,
            0, 0, 0, 1,
            0, 0, 0, 0};

    this->PushEffect(CLSID_D2D1ColorMatrix);

    const int D2D1_COLORMATRIX_PROP_COLOR_MATRIX = 0;

    this->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
}

void EasyEffect::PushColorMatrix(ColorMatrix matrix)
{
    this->PushEffect(CLSID_D2D1ColorMatrix);

    const int D2D1_COLORMATRIX_PROP_COLOR_MATRIX = 0;

    this->SetValue(D2D1_COLORMATRIX_PROP_COLOR_MATRIX, matrix);
}

void EasyEffect::PushShadow(float standardDeviation, EasyPixel color)
{
    this->PushEffect(CLSID_D2D1Shadow);

    const int D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION = 0;

    const int D2D1_SHADOW_PROP_COLOR = 1;

    this->SetValue(D2D1_SHADOW_PROP_BLUR_STANDARD_DEVIATION, standardDeviation);

    this->SetValue(D2D1_SHADOW_PROP_COLOR, D2D1_VECTOR_4F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f));
}

void EasyEffect::PushCrop(int x, int y, int width, int height)
{
    this->PushEffect(CLSID_D2D1Crop);

    const int D2D1_CROP_PROP_RECT = 0;

    this->SetValue(D2D1_CROP_PROP_RECT, D2D1::RectF(x, y, x + width, y + height));
}

void EasyEffect::Render(int x, int y, float rotation, int midpointx, int midpointy, int reverse)
{
    if (this->effects.empty())
        return;

    D2D1_MATRIX_3X2_F Matrix = D2D1::Matrix3x2F::Translation(x, y);

    D2D1_POINT_2F center = {midpointx, midpointy};

    Matrix = D2D1::Matrix3x2F::Rotation(rotation, center) * Matrix;

    int scaleX = 1, scaleY = 1;

    if (reverse & EASY_TURNLR)
        scaleX = -1;
    if (reverse & EASY_TURNUD)
        scaleY = -1;

    Matrix = D2D1::Matrix3x2F::Scale(scaleX, scaleY, center) * Matrix;

    pRenderTarget->SetTransform(Matrix);
    pRenderTarget->DrawImage(this->effects.back());
}

EasyEffect::~EasyEffect()
{
    this->Release();
}

EasyEffect::operator ID2D1Effect *()
{
    if (this->effects.empty())
        return nullptr;

    return this->effects.back();
}

void EasyGeometryPath::StartUp()
{
    EasyPainting::pFactory->CreatePathGeometry(&pathGeometry);
    pathGeometry->Open(&sink);
}

void EasyGeometryPath::FigureStart(float x, float y)
{
    sink->BeginFigure(D2D1::Point2F(x, y), D2D1_FIGURE_BEGIN_FILLED);
}

void EasyGeometryPath::FigureEnd(bool close)
{
    sink->EndFigure(close ? D2D1_FIGURE_END_CLOSED : D2D1_FIGURE_END_OPEN);
    sink->Close();
}

void EasyGeometryPath::AddLine(float x, float y)
{
    sink->AddLine(D2D1::Point2F(x, y));
}

void EasyGeometryPath::DrawGeometry(EasyPixel color, float thickness, ID2D1DeviceContext *pRenderTarget)
{
    ID2D1SolidColorBrush *brush;
    pRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f), &brush);
    pRenderTarget->DrawGeometry(pathGeometry, brush, thickness);
    brush->Release();
}

void EasyGeometryPath::FillGeometry(EasyPixel color, ID2D1DeviceContext *pRenderTarget)
{
    ID2D1SolidColorBrush *brush;
    pRenderTarget->CreateSolidColorBrush(D2D1_COLOR_F(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f), &brush);
    pRenderTarget->FillGeometry(pathGeometry, brush);
    brush->Release();
}

void EasyGeometryPath::Release()
{
    if (sink)
        sink->Release();
    if (pathGeometry)
        pathGeometry->Release();
    sink = nullptr;
    pathGeometry = nullptr;
}

EasyGeometryPath::~EasyGeometryPath()
{
    this->Release();
}

EasyPaintingScreenBuffer::EasyPaintingScreenBuffer() {}

void EasyPaintingScreenBuffer::DrawStart()
{
    ::DrawStart();
}

void EasyPaintingScreenBuffer::DrawEnd()
{
    ::DrawEnd();
}

EasyPaintingDevice::EasyPaintingDevice() {}

void EasyPaintingDevice::SetVSync(bool enable)
{
    EasyPainting::enableVSync = enable;
}

void EasyPaintingDevice::SetWindow(HWND window, int WindowWidth, int WindowHeight, EasyPixel BackColor)
{
    using namespace EasyPainting;
    pTargetBitmap->Release();
    pSwapChain->Release();
    __Easy_SetWindow(window, WindowWidth, WindowHeight);
    EasyPainting::hBackColor = BackColor;
    easyScreenBuffer->pBufferDevice = pRenderTarget;
}

int EasyPaintingDevice::GetFPS()
{
    using namespace EasyPainting;
    lock_guard<mutex> lock(fpsMutex);
    return showFPS;
}

void EasyPaintingDevice::SetInterpolationMode(int mode)
{
    switch (mode)
    {
    case EASY_LINEAR_MODE:
        EasyPainting::easyInterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR;
        break;
    case EASY_NEAREST_MODE:
        EasyPainting::easyInterpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR;
        break;
    }
}
