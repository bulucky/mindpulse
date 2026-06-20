/**
 * @file tray_win.cpp
 * @brief Win32 原生系统托盘实现
 */

#ifdef _WIN32

#include "tray_win.h"
#include <cstring>
// #include <stdexcept>

TrayWin::TrayWin()
    : hwnd_(NULL),
      current_hicon_(NULL) {
    std::memset(&nid_, 0, sizeof(nid_));
}

TrayWin::~TrayWin() {
    if (hwnd_) {
        // 从托盘删除图标
        Shell_NotifyIconA(NIM_DELETE, &nid_);
        DestroyWindow(hwnd_);
    }
    if (current_hicon_) {
        DestroyIcon(current_hicon_);
    }
}

bool TrayWin::init() {
    HINSTANCE hInst = GetModuleHandleA(NULL);

    // 1. 注册隐藏辅助窗口的窗口类
    WNDCLASSEXA wc;
    std::memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = window_proc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;

    if (!RegisterClassExA(&wc)) {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }
    }

    // 2. 创建隐藏的 Message-only/辅助 窗口
    hwnd_ = CreateWindowExA(
        0,
        CLASS_NAME,
        "MindPulse Helper Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT,
        NULL, NULL,
        hInst,
        this // 传入 this 用于在窗口过程关联实例
    );

    if (!hwnd_) {
        return false;
    }

    // 3. 配置托盘图标基础数据
    nid_.cbSize = sizeof(NOTIFYICONDATAA);
    nid_.hWnd = hwnd_;
    nid_.uID = 1;
    nid_.uFlags = NIF_MESSAGE | NIF_TIP;
    nid_.uCallbackMessage = WM_TRAYICON;
    std::strcpy(nid_.szTip, "MindPulse");

    // 添加空托盘图标占位
    if (!Shell_NotifyIconA(NIM_ADD, &nid_)) {
        return false;
    }

    return true;
}

bool TrayWin::update_icon(const std::vector<uint8_t>& bgra_buffer, int width, int height) {
    if (bgra_buffer.size() != static_cast<size_t>(width * height * 4)) {
        return false;
    }

    HICON hNewIcon = create_hicon_from_bgra(bgra_buffer, width, height);
    if (!hNewIcon) {
        return false;
    }

    nid_.hIcon = hNewIcon;
    nid_.uFlags |= NIF_ICON;

    // 修改托盘中的图标
    Shell_NotifyIconA(NIM_MODIFY, &nid_);

    // 销毁旧图标避免 GDI 泄漏
    if (current_hicon_) {
        DestroyIcon(current_hicon_);
    }
    current_hicon_ = hNewIcon;

    return true;
}

void TrayWin::set_tooltip(const std::string& tooltip) {
    std::memset(nid_.szTip, 0, sizeof(nid_.szTip));
    std::strncpy(nid_.szTip, tooltip.c_str(), sizeof(nid_.szTip) - 1);
    nid_.uFlags |= NIF_TIP;
    Shell_NotifyIconA(NIM_MODIFY, &nid_);
}

void TrayWin::add_menu_item(const TrayMenuItem& item) {
    menu_items_.push_back(item);
}

void TrayWin::pump_messages() {
    MSG msg;
    // 非阻塞处理所有排队的窗口消息
    while (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
}

LRESULT CALLBACK TrayWin::window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    TrayWin* self = nullptr;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam); // NOLINT
        self = reinterpret_cast<TrayWin*>(cs->lpCreateParams);
        SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<TrayWin*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA)); // NOLINT
    }

    if (self) {
        if (uMsg == WM_TRAYICON) {
            self->handle_tray_icon_event(lParam);
            return 0;
        }
    }

    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void TrayWin::handle_tray_icon_event(LPARAM lParam) {
    if (lParam == WM_RBUTTONUP) {
        show_context_menu();
    }
}

void TrayWin::show_context_menu() {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreatePopupMenu();
    if (!hMenu) return;

    for (const auto& item : menu_items_) {
        UINT flags = MF_STRING;
        if (!item.enabled) flags |= MF_GRAYED;
        if (item.checked) flags |= MF_CHECKED;
        AppendMenuA(hMenu, flags, item.id, item.text.c_str());
    }

    // 确保点击菜单外部时菜单能正常消失 (Win32 原生限制处理)
    SetForegroundWindow(hwnd_);

    int select_id = TrackPopupMenu(
        hMenu,
        TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTALIGN | TPM_RIGHTBUTTON,
        pt.x, pt.y,
        0, hwnd_, NULL);

    DestroyMenu(hMenu);

    // 保证窗口切换焦点正常
    PostMessageA(hwnd_, WM_NULL, 0, 0);

    if (select_id > 0) {
        for (const auto& item : menu_items_) {
            if (item.id == select_id) {
                if (item.callback) {
                    item.callback();
                }
                break;
            }
        }
    }
}

HICON TrayWin::create_hicon_from_bgra(const std::vector<uint8_t>& bgra_buffer, int width, int height) {
    // 1. 设置位图头信息为 32位 BGRA DIB Section
    BITMAPV5HEADER bi;
    std::memset(&bi, 0, sizeof(bi));
    bi.bV5Size = sizeof(BITMAPV5HEADER);
    bi.bV5Width = width;
    bi.bV5Height = -height; // 负值表示自上而下的 DIB 像素拷贝
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    bi.bV5RedMask = 0x00FF0000;
    bi.bV5GreenMask = 0x0000FF00;
    bi.bV5BlueMask = 0x000000FF;
    bi.bV5AlphaMask = 0xFF000000;

    HDC hdc = GetDC(NULL);
    void* bits = nullptr;
    HBITMAP hColor = CreateDIBSection(hdc, reinterpret_cast<BITMAPINFO*>(&bi), DIB_RGB_COLORS, &bits, NULL, 0);
    ReleaseDC(NULL, hdc);

    if (!hColor) {
        return nullptr;
    }

    // 2. 拷贝 BGRA 数据到 DIB 存储区
    std::memcpy(bits, bgra_buffer.data(), bgra_buffer.size());

    // 3. 创建全黑掩码位图（Alpha 通道独立控色）
    HBITMAP hMask = CreateBitmap(width, height, 1, 1, NULL);
    if (!hMask) {
        DeleteObject(hColor);
        return nullptr;
    }

    ICONINFO ii;
    std::memset(&ii, 0, sizeof(ii));
    ii.fIcon = TRUE;
    ii.xHotspot = 0;
    ii.yHotspot = 0;
    ii.hbmMask = hMask;
    ii.hbmColor = hColor;

    HICON hIcon = CreateIconIndirect(&ii);

    // 4. 清理创建的位图资源避免泄漏
    DeleteObject(hColor);
    DeleteObject(hMask);

    return hIcon;
}

std::unique_ptr<ITray> create_platform_tray() {
    return std::make_unique<TrayWin>();
}

#endif // _WIN32
