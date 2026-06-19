/**
 * @file test_tray_integration.cpp
 * @brief 呼适应擎、图标渲染器与系统托盘整体集成的单元测试
 */

#include <gtest/gtest.h>
#include "core/breath_engine.h"
#include "core/icon_renderer.h"
#include "ui/tray.h"
#include <chrono>
#include <thread>

// 集成测试：联合驱动呼适应擎与渲染器更新至系统托盘中，运行 2 秒
TEST(TrayIntegrationTest, RunAnimationOnTray) {
    auto tray = create_platform_tray();
    ASSERT_NE(tray, nullptr);
    EXPECT_TRUE(tray->init());

    // 设置提示语及右键退出回调
    tray->set_tooltip("MindPulse Integration Test Running...");

    bool exit_clicked = false;
    tray->add_menu_item({1, "Exit Test Animation", true, false, [&exit_clicked]() {
                             exit_clicked = true;
                         }});

    // 实例呼适应擎和渲染器
    BreathEngine engine;
    IconRenderer renderer(32, 32);

    // 切换至活跃自旋状态 (浅蓝紫色 `#61649f` 旋转变化)
    engine.transition_to(BreathState::RUNNING);

    auto start_time = std::chrono::steady_clock::now();
    double dt = 0.016; // 模拟约 60fps 的更新率

    // 模拟运行 2 秒动画 (或直到右键菜单点击了退出)
    while (std::chrono::steady_clock::now() - start_time < std::chrono::seconds(30)) {
        // 1. 呼适应擎 tick
        engine.tick(dt);

        // 2. 渲染器渲染 BGRA 字节数据（传入亮度及累积自旋角度）
        auto bgra_buffer = renderer.render(
            engine.get_current_brightness(),
            engine.get_current_rotation_angle(),
            engine.get_current_color());

        // 3. 将字节数据提交至系统托盘更新
        EXPECT_TRUE(tray->update_icon(bgra_buffer, 32, 32));

        // 4. 非阻塞分发系统事件，使得右键能响应
        tray->pump_messages();

        if (exit_clicked) {
            break;
        }

        // 维持帧速
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
