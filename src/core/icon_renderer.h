/**
 * @file icon_renderer.h
 * @brief 图标渲染器声明，负责程序化渲染圆环带有三个缺口（黄金比例）的呼吸旋转图标
 */

#pragma once

#include <vector>
#include <cstdint>
#include "core/breath_engine.h" // 复用 ColorRGB 结构体

/**
 * @brief 图标渲染器类，通过多层高斯分布及极坐标计算生成三缺口黄金比例旋转圆环的 BGRA 像素矩阵
 */
class IconRenderer {
public:
    /**
     * @brief 构造函数
     * @param width 图标像素宽度，默认 32
     * @param height 图标像素高度，默认 32
     */
    IconRenderer(int width = 32, int height = 32);

    /**
     * @brief 重置渲染的分辨率以适配高 DPI
     * @param width 新宽度
     * @param height 新高度
     */
    void resize(int width, int height);

    /**
     * @brief 执行圆环极坐标渲染，产出带有 3 个黄金比例缺口且正在自旋的 BGRA 格式数据
     * @param brightness 当前动画帧对应的亮度/速度映射系数 [0.15, 1.0]
     * @param rotation_angle 当前自旋的累积角度（弧度）
     * @param color 当前动画帧对应的基准 RGB 颜色
     * @return std::vector<uint8_t> BGRA 像素序列（大小为 width * height * 4 字节）
     */
    std::vector<uint8_t> render(double brightness, double rotation_angle, const ColorRGB& color);

    /**
     * @brief 获取渲染器当前宽度
     * @return int
     */
    [[nodiscard]] int get_width() const { return width_; }

    /**
     * @brief 获取渲染器当前高度
     * @return int
     */
    [[nodiscard]] int get_height() const { return height_; }

private:
    int width_;  ///< 像素宽度
    int height_; ///< 像素高度
};
