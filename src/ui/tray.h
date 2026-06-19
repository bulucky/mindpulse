/**
 * @file tray.h
 * @brief 系统托盘抽象接口定义
 */

#pragma once

#include <vector>
#include <string>
#include <memory>
#include <cstdint>
#include <functional>

/**
 * @brief 托盘右键菜单项结构体
 */
struct TrayMenuItem {
    int id;                         ///< 菜单项 ID
    std::string text;               ///< 菜单项文本
    bool enabled = true;            ///< 是否启用
    bool checked = false;           ///< 是否勾选
    std::function<void()> callback; ///< 点击后的回调函数
};

/**
 * @brief 平台无关的系统托盘抽象接口
 */
class ITray {
public:
    virtual ~ITray() = default;

    /**
     * @brief 初始化并创建系统托盘图标
     * @return true 初始化成功，false 失败
     */
    virtual bool init() = 0;

    /**
     * @brief 更新托盘图标像素数据
     * @param bgra_buffer BGRA 格式的像素缓冲区，大小必须为 width * height * 4
     * @param width 宽度
     * @param height 高度
     * @return true 更新成功，false 失败
     */
    virtual bool update_icon(const std::vector<uint8_t>& bgra_buffer, int width, int height) = 0;

    /**
     * @brief 设置鼠标悬停时的提示文本
     * @param tooltip 提示文本
     */
    virtual void set_tooltip(const std::string& tooltip) = 0;

    /**
     * @brief 添加右键菜单项
     * @param item 菜单项结构体
     */
    virtual void add_menu_item(const TrayMenuItem& item) = 0;

    /**
     * @brief 驱动托盘消息循环（非阻塞更新，常用于处理单次事件泵）
     */
    virtual void pump_messages() = 0;
};

/**
 * @brief 创建对应平台实现的托盘工厂函数
 * @return std::unique_ptr<ITray>
 */
std::unique_ptr<ITray> create_platform_tray();
