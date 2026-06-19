/**
 * @file test_icon_renderer_test.cpp
 * @brief 验证图像生成规格、边界值及白热化效果的单元测试
 */

#include <gtest/gtest.h>
#include "core/icon_renderer.h"

// 验证渲染输出的缓冲区大小与规格
TEST(IconRendererTest, RenderBufferSizeAndResize) {
    IconRenderer renderer(32, 32);
    ColorRGB color = {140.0, 170.0, 238.0}; // 浅蓝

    auto buffer = renderer.render(0.5, color);
    // 32x32 分辨率 * 4 字节 (BGRA) = 4096 字节
    EXPECT_EQ(buffer.size(), 32 * 32 * 4);

    // 验证高 DPI 分辨率缩放
    renderer.resize(48, 48);
    buffer = renderer.render(0.8, color);
    EXPECT_EQ(buffer.size(), 48 * 48 * 4);
}

// 验证四角像素的 Alpha 通道是否被底噪掩码完美淡出（避免方框暗边）
TEST(IconRendererTest, CornerAlphaFadeOut) {
    IconRenderer renderer(32, 32);
    ColorRGB color = {220.0, 138.0, 120.0}; // 黄橙

    auto buffer = renderer.render(0.15, color);

    // 左上角角落像素 (0,0) 应该几乎透明（接近 0，远低于底噪 3% / 7.65）
    int corner_alpha = buffer[4 * (0 * 32 + 0) + 3];
    EXPECT_LT(corner_alpha, 3); // 验证平滑渐出

    // 中心像素 (16,16) 的 Alpha 应当非常饱满
    int center_alpha = buffer[4 * (16 * 32 + 16) + 3];
    EXPECT_GT(center_alpha, 38); // 大于 0.15 的 Alpha 亮度 (38/255)
}

// 验证中心发光白热化（Center White-hot Effect）是否生效
TEST(IconRendererTest, CenterWhiteHotCore) {
    IconRenderer renderer(32, 32);
    ColorRGB base_color = {140.0, 170.0, 238.0}; // 浅蓝

    // 在最大亮度 1.0 时渲染
    auto buffer = renderer.render(1.0, base_color);

    int center_offset = 4 * (16 * 32 + 16);
    int b = buffer[center_offset + 0];
    int g = buffer[center_offset + 1];
    int r = buffer[center_offset + 2];

    // 中心像素应该明显比基准色更亮（混合了部分白色）
    EXPECT_GT(b, base_color.b);
    EXPECT_GT(g, base_color.g);
    EXPECT_GT(r, base_color.r);

    // 验证外圈 (16, 5) 颜色依旧保持原纯色渐变
    int edge_offset = 4 * (16 * 32 + 5);
    int er = buffer[edge_offset + 2];
    
    // 外圈应该接近浅蓝色比例，红色（r）分量不会被强力白化
    EXPECT_LT(er, 200);
}
