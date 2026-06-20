/**
 * @file test_breath_engine_test.cpp
 * @brief 验证呼吸曲线、自旋角度和过渡特性的单元测试
 */

#include <gtest/gtest.h>
#include "breath_engine.h"

// 验证初始状态为 STOPPED 且亮度与新颜色 (#792740) 正确
TEST(BreathEngineTest, InitialStateStopped) {
    BreathEngine engine;
    EXPECT_EQ(engine.get_current_state(), BreathState::STOPPED);
    EXPECT_NEAR(engine.get_current_brightness(), 0.15, 0.001);

    ColorRGB color = engine.get_current_color();
    EXPECT_DOUBLE_EQ(color.r, 121.0);
    EXPECT_DOUBLE_EQ(color.g, 39.0);
    EXPECT_DOUBLE_EQ(color.b, 64.0);
}

// 验证在 0.5s 平滑过渡阶段的过渡因子和状态正确性
TEST(BreathEngineTest, TransitionDurationAndState) {
    BreathEngine engine;
    engine.transition_to(BreathState::IDLE);

    EXPECT_TRUE(engine.is_in_transition());
    EXPECT_EQ(engine.get_current_state(), BreathState::IDLE);

    // 模拟 tick 0.25 秒 (过渡到一半)
    engine.tick(0.25);
    EXPECT_TRUE(engine.is_in_transition());

    // 模拟再 tick 0.25 秒 (过渡结束)
    engine.tick(0.25);
    EXPECT_FALSE(engine.is_in_transition());
}

// 验证 IDLE 模式下 4.0s 呼吸周期的关键点亮度
TEST(BreathEngineTest, IdleBreathingCurvePoints) {
    BreathEngine engine;

    // 切换到 IDLE 状态，并通过 tick 0.5s 结束过渡期
    engine.transition_to(BreathState::IDLE);
    engine.tick(0.5); // 此时 state_time_ 为 0.5s

    // 再 tick 3.5s，让其达到下一个完整周期的起点 (state_time_ = 4.0s -> 相当于 0.0s)
    engine.tick(3.5);
    EXPECT_NEAR(engine.get_current_brightness(), 0.15, 0.001);

    // 1. 吸气阶段 37.5% 即 1.5s：达到波峰亮度 1.0
    engine.tick(1.5);
    EXPECT_NEAR(engine.get_current_brightness(), 1.0, 0.01);

    // 2. 呼气阶段 54.5% 即 2.18s：达到波谷亮度 0.15
    engine.tick(2.18);
    EXPECT_NEAR(engine.get_current_brightness(), 0.15, 0.01);

    // 3. 停顿阶段 8% 即 0.32s：保持在波谷 0.15
    engine.tick(0.2);
    EXPECT_NEAR(engine.get_current_brightness(), 0.15, 0.001);
}

// 验证过渡的平滑性（无突变悬崖）
TEST(BreathEngineTest, TransitionSmoothness) {
    BreathEngine engine;
    double prev_brightness = engine.get_current_brightness();
    engine.transition_to(BreathState::RUNNING);

    for (int i = 0; i < 5; ++i) {
        engine.tick(0.05); // 每帧 50ms
        double curr_brightness = engine.get_current_brightness();

        EXPECT_NEAR(curr_brightness - prev_brightness, (1.0 - 0.15) * 0.1, 0.15);
        prev_brightness = curr_brightness;
    }
}

// 验证自旋角度在运行状态下累加
TEST(BreathEngineTest, RotationAngleAccumulates) {
    BreathEngine engine;
    engine.transition_to(BreathState::RUNNING);
    engine.tick(0.5); // 消除过渡

    double initial_angle = engine.get_current_rotation_angle();
    engine.tick(0.1); // tick 100ms
    double next_angle = engine.get_current_rotation_angle();

    // 在 RUNNING 状态下，角速度 > 0，累加的角度应当大于初始值
    EXPECT_GT(next_angle, initial_angle);
}
