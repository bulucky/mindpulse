/**
 * @file breath_engine.cpp
 * @brief 呼适应擎实现，包含曲线计算和插值过渡
 */

#include "breath_engine.h"
#include <cmath>
// #include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

BreathEngine::BreathEngine()
    : current_state_(BreathState::STOPPED),
      state_time_(0.0),
      rotation_angle_(0.0),
      in_transition_(false),
      transition_time_(0.0),
      transition_duration_(0.5),
      transition_start_brightness_(0.15),
      transition_start_color_({121.0, 39.0, 64.0}), // 默认 STOP = #792740
      transition_start_spin_rate_(0.0),
      current_brightness_(0.15),
      current_color_({121.0, 39.0, 64.0}) {}

void BreathEngine::tick(double delta_time_sec) {
    double current_spin_rate = 0.0;

    if (in_transition_) {
        transition_time_ += delta_time_sec;
        state_time_ += delta_time_sec;

        if (transition_time_ >= transition_duration_) {
            // 过渡结束，完全切换到目标状态
            in_transition_ = false;
            current_brightness_ = calculate_state_brightness(current_state_, state_time_);
            current_color_ = get_state_color(current_state_);
            current_spin_rate = get_state_base_spin_rate(current_state_) * current_brightness_;
        } else {
            // 计算目标状态在当前相位下的亮度
            double target_brightness = calculate_state_brightness(current_state_, state_time_);
            ColorRGB target_color = get_state_color(current_state_);
            double target_spin_rate = get_state_base_spin_rate(current_state_) * target_brightness;

            // 计算平滑插值因子 (Ease-In-Out)
            double k = transition_time_ / transition_duration_;
            double k_smooth = 3.0 * k * k - 2.0 * k * k * k;

            // 亮度和颜色以及自旋速度插值
            current_brightness_ = transition_start_brightness_ * (1.0 - k_smooth) + target_brightness * k_smooth;
            current_color_.r = transition_start_color_.r * (1.0 - k_smooth) + target_color.r * k_smooth;
            current_color_.g = transition_start_color_.g * (1.0 - k_smooth) + target_color.g * k_smooth;
            current_color_.b = transition_start_color_.b * (1.0 - k_smooth) + target_color.b * k_smooth;
            current_spin_rate = transition_start_spin_rate_ * (1.0 - k_smooth) + target_spin_rate * k_smooth;
        }
    } else {
        state_time_ += delta_time_sec;
        current_brightness_ = calculate_state_brightness(current_state_, state_time_);
        current_color_ = get_state_color(current_state_);
        current_spin_rate = get_state_base_spin_rate(current_state_) * current_brightness_;
    }

    // 累加旋转角度并约束在 [0, 2*PI) 范围
    rotation_angle_ += current_spin_rate * delta_time_sec;
    rotation_angle_ = std::fmod(rotation_angle_, 2.0 * M_PI);
    if (rotation_angle_ < 0.0) {
        rotation_angle_ += 2.0 * M_PI;
    }
}

void BreathEngine::transition_to(BreathState new_state) {
    if (current_state_ == new_state && !in_transition_) {
        return;
    }

    // 记录当前时刻的瞬时参数，作为过渡起点
    transition_start_brightness_ = current_brightness_;
    transition_start_color_ = current_color_;

    // 计算瞬时的旋转角速度
    double start_base_rate = get_state_base_spin_rate(current_state_);
    if (in_transition_) {
        // 如果原本在过渡中，计算当前的过渡速度
        double k = transition_time_ / transition_duration_;
        double k_smooth = 3.0 * k * k - 2.0 * k * k * k;
        double target_rate = get_state_base_spin_rate(current_state_) * calculate_state_brightness(current_state_, state_time_);
        transition_start_spin_rate_ = transition_start_spin_rate_ * (1.0 - k_smooth) + target_rate * k_smooth;
    } else {
        transition_start_spin_rate_ = start_base_rate * current_brightness_;
    }

    current_state_ = new_state;
    state_time_ = 0.0; // 重置目标状态的时间相位
    transition_time_ = 0.0;
    in_transition_ = true;
}

double BreathEngine::get_current_brightness() const {
    return current_brightness_;
}

ColorRGB BreathEngine::get_current_color() const {
    return current_color_;
}

double BreathEngine::get_current_rotation_angle() const {
    return rotation_angle_;
}

BreathState BreathEngine::get_current_state() const {
    return current_state_;
}

bool BreathEngine::is_in_transition() const {
    return in_transition_;
}

double BreathEngine::calculate_state_brightness(BreathState state, double time_sec) const {
    switch (state) {
        case BreathState::STOPPED:
            return 0.15; // 静止暗态

        case BreathState::IDLE: {
            double period = 4.0;   // 4s 周期
            double t = std::fmod(time_sec, period);
            double p = t / period; // 归一化相位 [0, 1)

            if (p < 0.375) {
                // 吸气阶段 (Inhalation: 37.5%, 即 1.5s) -> Ease-In-Out
                double x = p / 0.375;
                double y = 0.5 * (1.0 - std::cos(M_PI * x));
                return 0.15 + 0.85 * y;
            } else if (p < 0.920) {
                // 呼气阶段 (Exhalation: 54.5%, 即 2.18s) -> Ease-Out
                double x = (p - 0.375) / 0.545;
                double y = (1.0 - x) * (1.0 - x); // 二次方快降缓停
                return 0.15 + 0.85 * y;
            } else {
                // 停顿阶段 (Pause: 8%, 即 0.32s) -> 恒定底部
                return 0.15;
            }
        }

        case BreathState::RUNNING: {
            double period = 1.8; // 1.8s 周期
            double t = std::fmod(time_sec, period);
            double p = t / period;

            if (p < 0.375) {
                // 吸气阶段 (Inhalation: 37.5%, 即 0.675s)
                double x = p / 0.375;
                double y = 0.5 * (1.0 - std::cos(M_PI * x));
                return 0.15 + 0.85 * y;
            } else if (p < 0.920) {
                // 呼气阶段 (Exhalation: 54.5%, 即 0.981s)
                double x = (p - 0.375) / 0.545;
                double y = (1.0 - x) * (1.0 - x);
                return 0.15 + 0.85 * y;
            } else {
                // 停顿阶段 (Pause: 8%, 即 0.144s)
                return 0.15;
            }
        }

        case BreathState::PENDING: {
            // 3.0s 周期 (3次 x 0.5s 脉冲 + 1.5s 停顿)
            double period = 3.0;
            double t = std::fmod(time_sec, period);

            if (t < 1.5) {
                // 处于 3 次脉冲爆发期
                double local_t = std::fmod(t, 0.5); // 单个脉冲 0.5s
                double u = local_t / 0.5;
                double y = std::sin(M_PI * u);      // 正弦脉冲波形
                return 0.15 + 0.85 * y;
            } else {
                // 处于 1.5s 停顿静止期
                return 0.15;
            }
        }

        default:
            return 0.15;
    }
}

ColorRGB BreathEngine::get_state_color(BreathState state) const {
    switch (state) {
        case BreathState::STOPPED:
            return {121.0, 39.0, 64.0};   // #792740
        case BreathState::IDLE:
            return {105.0, 173.0, 155.0}; // #69ad9b
        case BreathState::RUNNING:
            return {97.0, 100.0, 159.0};  // #61649f
        case BreathState::PENDING:
            return {229.0, 158.0, 103.0}; // #e59e67 (搭配颜色)
    }
    return {121.0, 39.0, 64.0};
}

double BreathEngine::get_state_base_spin_rate(BreathState state) const {
    switch (state) {
        case BreathState::STOPPED:
            return 0.0; // 静止不旋转
        case BreathState::IDLE:
            return 1.5; // 慢速旋转 (~1.5 rad/s)
        case BreathState::RUNNING:
            return 4.0; // 快速旋转 (~4.0 rad/s)
        case BreathState::PENDING:
            return 6.0; // 脉冲高速自旋 (~6.0 rad/s)
    }
    return 0.0;
}
