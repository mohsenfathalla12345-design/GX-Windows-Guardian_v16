#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <objidl.h>
#include <gdiplus.h>
#include <cmath>
#include <string>
#include <vector>

#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;

float g_Angle = 0.0f;

const std::vector<std::wstring> g_Modules = {
    L"System Guardian", L"Security Suite", L"Performance Boost",
    L"Windows Repair", L"Privacy Protector", L"Update Center",
    L"Wi-Fi Manager", L"G-X Insight", L"G-X Game Hub"
};

void DrawBrighterSpaceUI(Graphics& g, RECT rect) {
    g.SetSmoothingMode(SmoothingModeAntiAlias);

    LinearGradientBrush bgBrush(
        Rect(0, 0, rect.right, rect.bottom),
        Color(255, 12, 35, 45),
        Color(255, 5, 18, 25),
        LinearGradientModeVertical
    );
    g.FillRectangle(&bgBrush, 0, 0, rect.right, rect.bottom);

    SolidBrush glowCyan(Color(35, 0, 200, 255));
    SolidBrush glowGreen(Color(45, 0, 255, 150));
    g.FillEllipse(&glowCyan, rect.right / 2 - 300, rect.bottom / 2 - 300, 600, 600);
    g.FillEllipse(&glowGreen, rect.right / 2 - 200, rect.bottom / 2 - 200, 400, 400);

    int centerX = rect.right / 2;
    int centerY = rect.bottom / 2 - 20;

    for (int i = 5; i > 0; --i) {
        Pen glowPen(Color(30 * i, 50, 255, 160), (float)(i * 4));
        g.DrawEllipse(&glowPen, centerX - 110, centerY - 110, 220, 220);
    }

    Pen brightNeonPen(Color(255, 80, 255, 180), 2.5f);
    g.DrawEllipse(&brightNeonPen, centerX - 100, centerY - 100, 200, 200);

    int numModules = static_cast<int>(g_Modules.size());
    float radius = 220.0f;

    for (int i = 0; i < numModules; ++i) {
        float currentAngle = g_Angle + (i * (2.0f * 3.14159f / numModules));
        int modX = static_cast<int>(centerX + radius * cos(currentAngle));
        int modY = static_cast<int>(centerY + radius * sin(currentAngle));

        Pen linePen(Color(120, 0, 255, 150), 1.5f);
        g.DrawLine(&linePen, centerX, centerY, modX, modY);

        SolidBrush nodeBg(Color(220, 15, 45, 35));
        Pen nodeBorder(Color(255, 100, 255, 200), 2.0f);
        g.FillEllipse(&nodeBg, modX - 35, modY - 35, 70, 70);
        g.DrawEllipse(&nodeBorder, modX - 35, modY - 35, 70, 70);
    }

    FontFamily fontFamily(L"Segoe UI");
    Font titleFont(&fontFamily, 16, FontStyleBold, UnitPixel);
    Font textFont(&fontFamily, 11, FontStyleBold, UnitPixel);
    SolidBrush brightText(Color(255, 220, 255, 230));
    SolidBrush greenText(Color(255, 80, 255, 180));

    g.DrawString(L"G-X CONTROL CENTER", -1, &titleFont, PointF(20, 20), &greenText);
    g.DrawString(L"System Protected | High Performance Mode", -1, &textFont, PointF(20, 45), &brightText);
    
    Font coreFont(&fontFamily, 22, FontStyleBold, UnitPixel);
    g.DrawString(L"G-X", -1, &coreFont, PointF(centerX - 24, centerY - 20), &brightText);
    g.DrawString(L"CORE", -1, &textFont, PointF(centerX - 20, centerY + 10), &greenText);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_TIMER:
            g_Angle += 0.008f;
            if (g_Angle >= 6.28318f) g_Angle = 0.0f;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            RECT rect;
            GetClientRect(hwnd, &rect);
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            Graphics g(memDC);
            DrawBrighterSpaceUI(g, rect);

            BitBlt(hdc, 0, 0, rect.right, rect.bottom, memDC, 0, 0, SRCCOPY);
            
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_SIZE:
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GXSpaceUI";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0, L"GXSpaceUI", L"G-X Control Center — Futuristic Cyber Suite",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 650,
        NULL, NULL, hInstance, NULL
    );

    SetTimer(hwnd, 1, 16, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return (int)msg.wParam;
}
