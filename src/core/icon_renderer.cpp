/**
 * @file icon_renderer.cpp
 * @brief 图标渲染器实现，支持高 DPI 尺寸缩放、核心白热化混合及环境底噪遮罩
 */

#include "core/icon_renderer.h"
#include <cmath>
#include <algorithm>

IconRenderer::IconRenderer(int width, int height)
    : width_(width), height_(height) {}

void IconRenderer::resize(int width, int height) {
    width_ = width;
    height_ = height;
}

std::vector<uint8_t> IconRenderer::render(double brightness, const ColorRGB& color) {
    std::vector<uint8_t> buffer(width_ * height_ * 4, 0);

    // 图像中心坐标
    double cx = (width_ - 1.0) / 2.0;
    double cy = (height_ - 1.0) / 2.0;

    // 自适应 DPI 缩放比例因子
    double scale = width_ / 32.0;
    double sigma_core = 3.0 * scale;   // 核心发光点标准差
    double sigma_glow = 7.0 * scale;   // 外围光环标准差
    double sigma_mask = 12.0 * scale;  // 底噪淡出掩码标准差

    // 外围光环的峰值半径随亮度在 6px - 14px 之间往复扩张/收缩
    double r_min = 6.0 * scale;
    double r_max = 14.0 * scale;
    double target_radius = r_min + (r_max - r_min) * brightness;

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            double dx = x - cx;
            double dy = y - cy;
            double d = std::sqrt(dx * dx + dy * dy);

            // 1. 中心核心发光 (窄高斯分布)
            double i_core = std::exp(-(d * d) / (2.0 * sigma_core * sigma_core));

            // 2. 外部晕染光环 (在 target_radius 处的宽高斯环状分布)
            double diff_r = d - target_radius;
            double i_glow = std::exp(-(diff_r * diff_r) / (2.0 * sigma_glow * sigma_glow));

            // 3. 多层发光强度合成
            double core_intensity = brightness * i_core;
            double glow_intensity = brightness * 0.6 * i_glow;
            double intensity = core_intensity + glow_intensity;

            // 4. 环境底噪淡出，防止纯四角边缘截断产生虚线框
            double noise_mask = std::exp(-(d * d) / (2.0 * sigma_mask * sigma_mask));
            double alpha = std::max(intensity, 0.03 * noise_mask);
            
            if (alpha > 1.0) {
                alpha = 1.0;
            }

            // 5. 核心白热化混合 (Center White-hot Effect)
            // 当越靠近中心且亮度越高时，颜色趋向白炽，提高发光质感
            double blend_factor = i_core * brightness;
            if (blend_factor > 1.0) {
                blend_factor = 1.0;
            }

            double r_val = color.r * (1.0 - blend_factor) + 255.0 * blend_factor;
            double g_val = color.g * (1.0 - blend_factor) + 255.0 * blend_factor;
            double b_val = color.b * (1.0 - blend_factor) + 255.0 * blend_factor;

            // 限制颜色字节范围
            r_val = std::max(0.0, std::min(255.0, r_val));
            g_val = std::max(0.0, std::min(255.0, g_val));
            b_val = std::max(0.0, std::min(255.0, b_val));

            // 6. 填充 BGRA 缓冲区 (Windows DIB 要求字节序列为 B、G、R、A)
            int pixel_offset = 4 * (y * width_ + x);
            buffer[pixel_offset + 0] = static_cast<uint8_t>(b_val); // Blue
            buffer[pixel_offset + 1] = static_cast<uint8_t>(g_val); // Green
            buffer[pixel_offset + 2] = static_cast<uint8_t>(r_val); // Red
            buffer[pixel_offset + 3] = static_cast<uint8_t>(alpha * 255.0); // Alpha
        }
    }

    return buffer;
}
