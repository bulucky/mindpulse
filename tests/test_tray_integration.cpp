/**
 * @file test_tray_integration.cpp
 * @brief 呼适应擎、图标渲染器与系统托盘整体集成的单元测试
 */

#include <gtest/gtest.h>
#include "breath_engine.h"
#include "icon_renderer.h"
#include "tray.h"
#include <chrono>
#include <cstdlib>
#include <thread>
#include <string>
#include <vector>

// 辅助函数：将状态枚举转换为易读的字符串说明
static std::string get_state_string(BreathState state) {
    switch (state) {
        case BreathState::STOPPED:
            return "STOPPED (Static Burgundy)";
        case BreathState::IDLE:
            return "IDLE (Slow Spin Mint-Green)";
        case BreathState::RUNNING:
            return "RUNNING (Fast Spin Lavender)";
        case BreathState::PENDING:
            return "PENDING (Burst Pulse Terracotta)";
    }
    return "UNKNOWN";
}

// 集成测试：联合驱动四种不同状态下的呼适应擎、渲染器更新至系统托盘中进行视觉与功能验证
TEST(TrayIntegrationTest, RunAnimationOnTray) {
    if (std::getenv("MINDPULSE_RUN_TRAY_VISUAL_TEST") == nullptr) {
        GTEST_SKIP() << "Set MINDPULSE_RUN_TRAY_VISUAL_TEST=1 to run the visual tray animation test.";
    }

    auto tray = create_platform_tray();
    ASSERT_NE(tray, nullptr);
    EXPECT_TRUE(tray->init());

    // 实例呼适应擎和渲染器
    BreathEngine engine;
    IconRenderer renderer(32, 32);

    // 定义要依次测试的四种状态及其测试时长
    struct StateTestCase {
        BreathState state;
        double duration_sec;
    };

    std::vector<StateTestCase> cases = {
        {BreathState::STOPPED, 15}, // 1. 待机酒红（静止）
        {BreathState::IDLE, 15},    // 2. 薄荷绿深慢自旋
        {BreathState::RUNNING, 15}, // 3. 熏衣紫快速自旋
        {BreathState::PENDING, 15}  // 4. 暖杏橙间歇爆发脉冲 (2.5s 足够看清 3次脉冲加速+停顿慢行)
    };

    double dt = 0.016; // 模拟约 60fps 的更新率

    for (const auto& test_case : cases) {
        // 1. 触发状态机切换，同时开启 0.5s 平滑插值过渡
        engine.transition_to(test_case.state);

        // 2. 动态更新托盘提示语，指示当前测试的状态
        std::string tooltip = "MindPulse Test - " + get_state_string(test_case.state);
        tray->set_tooltip(tooltip);

        auto state_start_time = std::chrono::steady_clock::now();
        double elapsed_sec = 0.0;

        // 3. 在指定测试时间里进行帧循环更新
        while (elapsed_sec < test_case.duration_sec) {
            // 呼适应擎时间向前推移并计算新的自旋偏角
            engine.tick(dt);

            // 渲染器生成图像字节
            auto bgra_buffer = renderer.render(
                engine.get_current_brightness(),
                engine.get_current_rotation_angle(),
                engine.get_current_color());

            // 将像素数据刷入系统托盘
            EXPECT_TRUE(tray->update_icon(bgra_buffer, 32, 32));

            // 处理窗口消息，以便响应悬停文字
            tray->pump_messages();

            // 维持约 60fps 帧率
            std::this_thread::sleep_for(std::chrono::milliseconds(16));

            elapsed_sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - state_start_time).count();
        }
    }
}
