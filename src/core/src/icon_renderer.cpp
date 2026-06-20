/**
 * @file icon_renderer.cpp
 * @brief 图标渲染器实现，支持高 DPI 尺寸缩放、三缺口黄金比例旋转圆环渲染
 */

#include <cmath>
#include <algorithm>

#include "icon_renderer.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
/**
 * @brief 平滑插值函数（GLSL smoothstep 实现），用于消除边缘锯齿
 */
double smoothstep(double edge0, double edge1, double x) {
    double t = std::max(0.0, std::min(1.0, (x - edge0) / (edge1 - edge0)));
    return t * t * (3.0 - 2.0 * t);
}
} // namespace

IconRenderer::IconRenderer(int width, int height)
    : width_(width), height_(height) {}

void IconRenderer::resize(int width, int height) {
    width_ = width;
    height_ = height;
}

std::vector<uint8_t> IconRenderer::render(double brightness, double rotation_angle, const ColorRGB& color) {
    std::vector<uint8_t> buffer(width_ * height_ * 4, 0);

    // 图像中心坐标
    double cx = (width_ - 1.0) / 2.0;
    double cy = (height_ - 1.0) / 2.0;

    // 自适应 DPI 缩放比例因子
    double scale = width_ / 32.0;

    // 旋转圆环基本参数
    double r_mid = 9.5 * scale;       // 圆环中心半径
    double r_width = 2.0 * scale;     // 圆环边缘的高斯半径/宽度标准差
    double sigma_mask = 12.0 * scale; // 底噪遮罩标准差

    // 三等分缺口与黄金比例设计
    // 圆环被分为三个扇区，每个扇区 120 度 (2*PI / 3)
    double sector_angle = (2.0 * M_PI) / 3.0;

    // 实心段与缺口段符合黄金比例：实心部分占约 61.803%，空心段占约 38.197%
    double solid_angle = sector_angle * 0.6180339887;
    double transition_width = 0.15; // 边缘羽化宽度（单位：弧度，约 8.6 度），防止旋转产生闪烁锯齿

    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            double dx = x - cx;
            double dy = y - cy;
            double d = std::sqrt(dx * dx + dy * dy);

            // 1. 径向高斯分布 (Radial Ring Intensity)
            double i_ring = std::exp(-(d - r_mid) * (d - r_mid) / (2.0 * r_width * r_width));

            // 2. 计算当前像素极坐标角度 [0, 2*PI)
            double theta = std::atan2(dy, dx);
            if (theta < 0) {
                theta += 2.0 * M_PI;
            }

            // 3. 施加旋转偏置，求其在旋转坐标系下的相对角度
            double relative_theta = std::fmod(theta - rotation_angle + 10.0 * M_PI, 2.0 * M_PI);

            // 4. 求其在单个三等分扇区内对应的相对角度
            double psi = std::fmod(relative_theta, sector_angle);

            // 5. 角度方向羽化裁剪：实心圆环弧长为 [0, solid_angle]
            // 使用 smoothstep 在两个交界点进行淡入淡出
            double edge1 = smoothstep(0.0, transition_width, psi);
            double edge2 = 1.0 - smoothstep(solid_angle - transition_width, solid_angle, psi);
            double i_angular = edge1 * edge2;

            // 圆环主体像素发光值 (径向 * 角向)
            double intensity = i_ring * i_angular;

            // 6. 附加背景光晕 (Subtle Glow)：使环即使在缺口处也带有淡淡的、随呼吸波动的光环，提升呼吸灯质感
            double i_glow = std::exp(-(d - r_mid) * (d - r_mid) / (2.0 * 5.0 * scale * 5.0 * scale));
            double glow_intensity = brightness * 0.25 * i_glow;

            double total_intensity = intensity + glow_intensity;

            // 7. 底噪遮罩淡出，防止纯四角边缘截断产生虚线框
            double noise_mask = std::exp(-(d * d) / (2.0 * sigma_mask * sigma_mask));
            double alpha = std::max(total_intensity, 0.03 * noise_mask);

            if (alpha > 1.0) {
                alpha = 1.0;
            }

            // 限制颜色字节范围
            double r_val = std::max(0.0, std::min(255.0, color.r));
            double g_val = std::max(0.0, std::min(255.0, color.g));
            double b_val = std::max(0.0, std::min(255.0, color.b));

            // 8. 填充 BGRA 缓冲区 (Windows DIB 要求字节序列为 B、G、R、A)
            int pixel_offset = 4 * (y * width_ + x);
            buffer[pixel_offset + 0] = static_cast<uint8_t>(b_val);         // Blue
            buffer[pixel_offset + 1] = static_cast<uint8_t>(g_val);         // Green
            buffer[pixel_offset + 2] = static_cast<uint8_t>(r_val);         // Red
            buffer[pixel_offset + 3] = static_cast<uint8_t>(alpha * 255.0); // Alpha
        }
    }

    return buffer;
}
