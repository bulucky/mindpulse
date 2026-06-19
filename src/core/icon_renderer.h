/**
 * @file icon_renderer.h
 * @brief 图标渲染器声明，负责程序化渲染高斯光晕呼吸灯图标
 */

#pragma once

#include <vector>
#include <cstdint>
#include "core/breath_engine.h" // 复用 ColorRGB 结构体

/**
 * @brief 图标渲染器类，通过多层高斯分布公式生成自适应 DPI 尺寸的 BGRA 图标像素矩阵
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
     * @brief 执行高斯模糊光晕渲染，产出 BGRA 格式数据
     * @param brightness 当前动画帧对应的亮度 [0.15, 1.0]
     * @param color 当前动画帧对应的基准 RGB 颜色
     * @return std::vector<uint8_t> BGRA 像素序列（大小为 width * height * 4 字节）
     */
    std::vector<uint8_t> render(double brightness, const ColorRGB& color);

    /**
     * @brief 获取渲染器当前宽度
     * @return int
     */
    int get_width() const { return width_; }

    /**
     * @brief 获取渲染器当前高度
     * @return int
     */
    int get_height() const { return height_; }

private:
    int width_;   ///< 像素宽度
    int height_;  ///< 像素高度
};
