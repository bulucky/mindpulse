/**
 * @file tray_stub.cpp
 * @brief 非 Windows 平台的系统托盘桩实现 (Stub)
 */

#ifndef _WIN32

#include "tray.h"
#include <iostream>

/**
 * @brief 空壳桩类，仅输出调试日志而不调用任何平台特有接口
 */
class TrayStub : public ITray {
public:
    TrayStub() = default;
    ~TrayStub() override = default;

    bool init() override {
        std::cout << "[TrayStub] 初始化桩系统托盘\n";
        return true;
    }

    bool update_icon(const std::vector<uint8_t>& bgra_buffer, int width, int height) override {
        // 无操作，模拟成功
        return true;
    }

    void set_tooltip(const std::string& tooltip) override {
        std::cout << "[TrayStub] 设置悬停文本: \"" << tooltip << "\"\n";
    }

    void add_menu_item(const TrayMenuItem& item) override {
        menu_items_.push_back(item);
        std::cout << "[TrayStub] 注册右键菜单项: \"" << item.text << "\"\n";
    }

    void pump_messages() override {
        // 无操作
    }

private:
    std::vector<TrayMenuItem> menu_items_;
};

std::unique_ptr<ITray> create_platform_tray() {
    return std::make_unique<TrayStub>();
}

#endif // !_WIN32
