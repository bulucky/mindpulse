/**
 * @file tray_win.h
 * @brief Win32 原生系统托盘实现声明
 */

#pragma once

#ifdef _WIN32

#include "tray.h"
#include <windows.h>
#include <shellapi.h>
#include <vector>

/**
 * @brief Windows 平台的系统托盘实现类
 */
class TrayWin : public ITray {
public:
    /**
     * @brief 构造函数
     */
    TrayWin();

    /**
     * @brief 析构函数，负责清理 HICON 及托盘资源
     */
    ~TrayWin() override;

    bool init() override;
    bool update_icon(const std::vector<uint8_t>& bgra_buffer, int width, int height) override;
    void set_tooltip(const std::string& tooltip) override;
    void add_menu_item(const TrayMenuItem& item) override;
    void pump_messages() override;

private:
    /**
     * @brief 静态窗口回调函数
     */
    static LRESULT CALLBACK window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    /**
     * @brief 处理托盘图标的鼠标事件
     * @param lParam 关联的消息代码 (如 WM_RBUTTONUP)
     */
    void handle_tray_icon_event(LPARAM lParam);

    /**
     * @brief 显示右键上下文菜单
     */
    void show_context_menu();

    /**
     * @brief 从原始 BGRA 内存缓冲区创建一个 Windows HICON 图标句柄
     */
    HICON create_hicon_from_bgra(const std::vector<uint8_t>& bgra_buffer, int width, int height);

    HWND hwnd_;                                                      ///< 隐藏辅助窗口的句柄
    NOTIFYICONDATAA nid_;                                            ///< Windows 托盘图标数据结构
    HICON current_hicon_;                                            ///< 当前显示的图标句柄
    std::vector<TrayMenuItem> menu_items_;                           ///< 绑定的右键菜单项

    static const UINT WM_TRAYICON = WM_USER + 1;                     ///< 托盘回调自定义消息
    static constexpr const char* CLASS_NAME = "MindPulseTrayWindow"; ///< 隐藏窗口的类名
};

#endif // _WIN32
