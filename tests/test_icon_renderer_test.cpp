/**
 * @file test_icon_renderer_test.cpp
 * @brief 验证新三缺口圆环图像生成规格、边界值及环形结构的单元测试
 */

#include <gtest/gtest.h>
#include "icon_renderer.h"

// 验证渲染输出的缓冲区大小与规格
TEST(IconRendererTest, RenderBufferSizeAndResize) {
    IconRenderer renderer(32, 32);
    ColorRGB color = {97.0, 100.0, 159.0}; // 浅蓝紫色 #61649f

    auto buffer = renderer.render(0.5, 0.0, color);
    // 32x32 分辨率 * 4 字节 (BGRA) = 4096 字节
    EXPECT_EQ(buffer.size(), 32 * 32 * 4);

    // 验证高 DPI 分辨率缩放
    renderer.resize(48, 48);
    buffer = renderer.render(0.8, 1.2, color);
    EXPECT_EQ(buffer.size(), 48 * 48 * 4);
}

// 验证四角像素的 Alpha 通道是否被底噪掩码完美淡出（避免方框暗边）
TEST(IconRendererTest, CornerAlphaFadeOut) {
    IconRenderer renderer(32, 32);
    ColorRGB color = {229.0, 158.0, 103.0}; // 暖橙 #e59e67

    auto buffer = renderer.render(0.15, 0.0, color);

    // 左上角角落像素 (0,0) 应该几乎透明（接近 0，远低于底噪 3% / 7.65）
    int corner_alpha = buffer[4 * (0 * 32 + 0) + 3];
    EXPECT_LT(corner_alpha, 3); // 验证平滑渐出
}

// 验证圆环的径向几何结构：圆心是空心的（低Alpha），环周是实心的（高Alpha）
TEST(IconRendererTest, RingAlphaDistribution) {
    IconRenderer renderer(32, 32);
    ColorRGB color = {97.0, 100.0, 159.0}; // 浅蓝紫色 #61649f

    // 渲染一帧数据，无旋转偏置，亮度 1.0
    auto buffer = renderer.render(1.0, 0.0, color);

    // 1. 中心点 (16,16) 处于圆环内侧空心区：其 Alpha 应当较低
    int center_alpha = buffer[4 * (16 * 32 + 16) + 3];

    // 2. 在半径 R=9.5 处（例如 x=16, y=25，距离中心 9 像素）：应当处于实心环上，Alpha 明显更高
    int ring_pixel_alpha = buffer[4 * (25 * 32 + 16) + 3];

    // 实心环像素的 Alpha 应该明显大于空心中心像素的 Alpha
    EXPECT_GT(ring_pixel_alpha, center_alpha);
}
