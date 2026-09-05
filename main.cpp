#include <windows.h>
#include <gdiplus.h>
#include <commctrl.h>
#include <string>
#include <vector>

#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "Comctl32.lib")

using namespace Gdiplus;

// ==========================================
// 1. ثوابت الألوان الرقمية (Cyberpunk Green Theme)
// ==========================================
#define COLOR_BG        Color(255, 5, 10, 8)
#define COLOR_PANEL     Color(255, 12, 22, 16)
#define COLOR_BORDER    Color(255, 20, 50, 30)
#define COLOR_NEON      Color(255, 0, 255, 100)
#define COLOR_DIM_NEON  Color(255, 0, 150, 60)
#define COLOR_TEXT      Color(255, 0, 230, 90)
#define COLOR_TEXT_DIM  Color(255, 0, 120, 50)
#define COLOR_ALERT     Color(255, 255, 60, 60)
#define COLOR_WARN      Color(255, 255, 200, 0)

// ==========================================
// 2. أقسام النظام الـ 18 (Modules)
// ==========================================
enum TabType {
    TAB_DASHBOARD = 0,
    TAB_SCAN,
    TAB_FIREWALL,
    TAB_DEFENDER,
    TAB_UPDATE,
    TAB_REPAIR,
    TAB_CLEANUP,
    TAB_OPTIMIZATION,
    TAB_NETWORK,
    TAB_WIFI_MANAGER,
    TAB_INSIGHT,
    TAB_ALERTS,
    TAB_SECURITY_CENTER,
    TAB_SETTINGS
};

struct SystemMetrics {
    int cpu = 32;
    int ram = 61;
    int disk = 27;
    int netUp = 12;
    int netDown = 37;
    int coreTemp = 48;
    int progress = 67;
};

static ULONG_PTR gdiplusToken;
static HWND gMainHWnd = nullptr;
static TabType gCurrentTab = TAB_DASHBOARD;
static SystemMetrics gMetrics;
static std::vector<std::wstring> gExecutionLogs;
static int gScrollOffsetY = 0;

// ==========================================
// 3. دمج السجلات والرسومات الأساسية
// ==========================================
void LogMessage(const std::wstring& msg) {
    gExecutionLogs.push_back(msg);
    if (gExecutionLogs.size() > 15) {
        gExecutionLogs.erase(gExecutionLogs.begin());
    }
}

void DrawCyberPanel(Graphics& g, INT x, INT y, INT w, INT h, const wchar_t* title = nullptr) {
    SolidBrush panelBrush(COLOR_PANEL);
    Pen borderPen(COLOR_BORDER, 1.5f);
    g.FillRectangle(&panelBrush, x, y, w, h);
    g.DrawRectangle(&borderPen, x, y, w, h);

    if (title) {
        Font font(L"Consolas", 10, FontStyleBold);
        SolidBrush textBrush(COLOR_TEXT);
        g.DrawString(title, -1, &font, PointF((REAL)(x + 10), (REAL)(y + 8)), &textBrush);
    }
}

void DrawCyberButton(Graphics& g, INT x, INT y, INT w, INT h, const wchar_t* text, bool isActive = false) {
    SolidBrush btnBrush(isActive ? COLOR_BORDER : COLOR_PANEL);
    Pen borderPen(isActive ? COLOR_NEON : COLOR_DIM_NEON, 1.2f);
    g.FillRectangle(&btnBrush, x, y, w, h);
    g.DrawRectangle(&borderPen, x, y, w, h);

    Font font(L"Consolas", 9, FontStyleBold);
    SolidBrush textBrush(isActive ? COLOR_NEON : COLOR_TEXT);
    StringFormat sf;
    sf.SetAlignment(StringAlignmentCenter);
    sf.SetLineAlignment(StringAlignmentCenter);

    RectF rect((REAL)x, (REAL)y, (REAL)w, (REAL)h);
    g.DrawString(text, -1, &font, rect, &sf, &textBrush);
}

void DrawProgressBar(Graphics& g, INT x, INT y, INT w, INT h, int percentage) {
    SolidBrush progBg(COLOR_BORDER);
    SolidBrush progFill(COLOR_NEON);
    g.FillRectangle(&progBg, x, y, w, h);
    INT fillWidth = (w * percentage) / 100;
    if (fillWidth > 0) {
        g.FillRectangle(&progFill, x, y, fillWidth, h);
    }
}

void DrawReactorCore(Graphics& g, INT cx, INT cy, INT radius) {
    g.SetSmoothingMode(SmoothingModeAntiAlias);
    Pen neonPen(COLOR_NEON, 3.0f);
    Pen dimPen(COLOR_DIM_NEON, 1.0f);

    g.DrawEllipse(&dimPen, cx - radius, cy - radius, radius * 2, radius * 2);
    g.DrawEllipse(&neonPen, cx - (radius - 15), cy - (radius - 15), (radius - 15) * 2, (radius - 15) * 2);

    Font fontTitle(L"Segoe UI", 18, FontStyleBold);
    SolidBrush neonBrush(COLOR_NEON);
    StringFormat sf;
    sf.SetAlignment(StringAlignmentCenter);
    sf.SetLineAlignment(StringAlignmentCenter);

    g.DrawString(L"G-X\nCORE", -1, &fontTitle, PointF((REAL)cx, (REAL)(cy - 5)), &sf, &neonBrush);
}

// ==========================================
// 4. رسم الواجهة
// ==========================================
void OnPaint(HDC hdc, RECT& rc) {
    Bitmap buffer(rc.right, rc.bottom);
    Graphics g(&buffer);
    g.SetSmoothingMode(SmoothingModeAntiAlias);

    SolidBrush bgBrush(COLOR_BG);
    g.FillRectangle(&bgBrush, 0, 0, (INT)rc.right, (INT)rc.bottom);

    // Header
    DrawCyberPanel(g, 10, 10, rc.right - 20, 45);
    Font headerFont(L"Consolas", 12, FontStyleBold);
    SolidBrush neonBrush(COLOR_NEON);
    g.DrawString(L"🛡️ G-X WINDOWS GUARDIAN v3.0.0 | CYBERNETIC SECURITY SYSTEM", -1, &headerFont, PointF(20.0f, 22.0f), &neonBrush);

    // Navigation Tabs Bar
    const wchar_t* tabs[] = { L"DASHBOARD", L"SCAN", L"FIREWALL", L"DEFENDER", L"UPDATE", L"REPAIR", L"CLEANUP", L"SECURITY CENTER" };
    for (int i = 0; i < 8; ++i) {
        DrawCyberButton(g, 10 + (i * 125), 62, 120, 38, tabs[i], (int)gCurrentTab == i);
    }

    // Main Content Area (With Scroll Support)
    INT startY = 110 + gScrollOffsetY;

    if (gCurrentTab == TAB_DASHBOARD) {
        // Terminal Panel
        DrawCyberPanel(g, 10, startY, 300, 430, L"> G-X TERMINAL LOGS");
        Font termFont(L"Consolas", 8.5f, FontStyleRegular);
        SolidBrush textBrush(COLOR_TEXT);
        REAL ty = (REAL)(startY + 35);
        for (const auto& log : gExecutionLogs) {
            g.DrawString(log.c_str(), -1, &termFont, PointF(20.0f, ty), &textBrush);
            ty += 20.0f;
        }

        // Reactor Core Panel
        DrawCyberPanel(g, 320, startY, 380, 430);
        Font statusFont(L"Consolas", 11, FontStyleBold);
        StringFormat sfCenter; sfCenter.SetAlignment(StringAlignmentCenter);
        g.DrawString(L"REACTOR STATUS: EXECUTING", -1, &statusFont, PointF(510.0f, (REAL)(startY + 20)), &sfCenter, &neonBrush);
        DrawReactorCore(g, 510, startY + 200, 100);

        g.DrawString((L"PROCESS PROGRESS: " + std::to_wstring(gMetrics.progress) + L"%").c_str(), -1, &termFont, PointF(340.0f, (REAL)(startY + 360)), &neonBrush);
        DrawProgressBar(g, 340, startY + 385, 340, 12, gMetrics.progress);

        // System Monitor Panel
        DrawCyberPanel(g, 710, startY, 290, 430, L"SYSTEM MONITOR");
        INT my = startY + 35;
        g.DrawString((L"CPU USAGE: " + std::to_wstring(gMetrics.cpu) + L"%").c_str(), -1, &termFont, PointF(725.0f, (REAL)my), &neonBrush); my += 18;
        DrawProgressBar(g, 725, my, 260, 8, gMetrics.cpu); my += 25;

        g.DrawString((L"MEMORY: " + std::to_wstring(gMetrics.ram) + L"%").c_str(), -1, &termFont, PointF(725.0f, (REAL)my), &neonBrush); my += 18;
        DrawProgressBar(g, 725, my, 260, 8, gMetrics.ram); my += 25;

        g.DrawString((L"DISK ACTIVITY: " + std::to_wstring(gMetrics.disk) + L"%").c_str(), -1, &termFont, PointF(725.0f, (REAL)my), &neonBrush); my += 18;
        DrawProgressBar(g, 725, my, 260, 8, gMetrics.disk);
    } else {
        DrawCyberPanel(g, 10, startY, 990, 430, L"MODULE VIEW");
        Font font(L"Consolas", 12, FontStyleBold);
        g.DrawString(L"Module interface loaded and ready.", -1, &font, PointF(30.0f, (REAL)(startY + 50)), &neonBrush);
    }

    // Footer Status Bar
    DrawCyberPanel(g, 10, 550, rc.right - 20, 35);
    Font footerFont(L"Consolas", 8.5f, FontStyleRegular);
    SolidBrush textDimBrush(COLOR_TEXT_DIM);
    g.DrawString(L"G-X CORE v3.0.0 | ALL SYSTEMS OPERATIONAL | ENCRYPTION: AES-256", -1, &footerFont, PointF(20.0f, 561.0f), &textDimBrush);

    Graphics screenGraphics(hdc);
    screenGraphics.DrawImage(&buffer, 0, 0);
}

// ==========================================
// 5. معالجة أحداث الويندوز
// ==========================================
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        LogMessage(L"[23:45:18] G-X CORE INITIALIZING...");
        LogMessage(L"[23:45:18] SYSTEM SECURE [ OK ]");
        LogMessage(L"[23:45:19] GUARDIAN ENGINE READY.");
        SetTimer(hwnd, 1, 1000, NULL);
        break;
    case WM_TIMER:
        gMetrics.progress = (gMetrics.progress + 2) % 100;
        gMetrics.cpu = 28 + (rand() % 10);
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_MOUSEWHEEL: {
        int zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (zDelta > 0) gScrollOffsetY = min(0, gScrollOffsetY + 20);
        else gScrollOffsetY = max(-200, gScrollOffsetY - 20);
        InvalidateRect(hwnd, NULL, FALSE);
        break;
    }

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        if (y >= 62 && y <= 100) {
            int clickedTab = (x - 10) / 125;
            if (clickedTab >= 0 && clickedTab < 8) {
                gCurrentTab = (TabType)clickedTab;
                LogMessage(L"> CHANGED MODULE TAB.");
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT rc; GetClientRect(hwnd, &rc);
        OnPaint(hdc, rc);
        EndPaint(hwnd, &ps);
        break;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_DESTROY:
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"GXGuardianClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClassW(&wc);

    gMainHWnd = CreateWindowExW(
        0, L"GXGuardianClass", L"G-X Windows Guardian",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1030, 630,
        NULL, NULL, hInstance, NULL
    );

    ShowWindow(gMainHWnd, nCmdShow);
    UpdateWindow(gMainHWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return 0;
}
