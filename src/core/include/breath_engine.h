/**
 * @file breath_engine.h
 * @brief 呼适应擎声明，负责计算动画曲线和状态平滑过渡
 */

#pragma once

// #include <cstdint>

/**
 * @brief 呼吸灯的状态枚举
 */
enum class BreathState { // NOLINT
    STOPPED,             ///< 浅红静止
    IDLE,                ///< 白深慢呼吸
    RUNNING,             ///< 浅蓝活跃呼吸
    PENDING              ///< 黄橙间歇脉冲
};

/**
 * @brief 表示 RGB 颜色
 */
struct ColorRGB {
    double r; ///< 红色通道 [0.0, 255.0]
    double g; ///< 绿色通道 [0.0, 255.0]
    double b; ///< 蓝色通道 [0.0, 255.0]
};

/**
 * @brief 呼适应擎类，处理多状态动画曲线计算及状态切换平滑差值
 */
class BreathEngine {
public:
    /**
     * @brief 构造函数，初始化为 STOPPED 状态
     */
    BreathEngine();

    /**
     * @brief 更新引擎时间与状态（每帧调用）
     * @param delta_time_sec 距离上一帧的时间间隔，单位：秒
     */
    void tick(double delta_time_sec);

    /**
     * @brief 触发状态切换并开启 0.5s 平滑过渡
     * @param new_state 新的目标状态
     */
    void transition_to(BreathState new_state);

    /**
     * @brief 获取当前帧计算得到的亮度值
     * @return 亮度值 [0.15, 1.0]
     */
    [[nodiscard]] double get_current_brightness() const;

    /**
     * @brief 获取当前帧计算得到的颜色值
     * @return ColorRGB 颜色结构体
     */
    [[nodiscard]] ColorRGB get_current_color() const;

    /**
     * @brief 获取当前帧累积的旋转角度（弧度）
     * @return 旋转角度值
     */
    [[nodiscard]] double get_current_rotation_angle() const;

    /**
     * @brief 获取当前所处的真实目标状态
     * @return BreathState
     */
    [[nodiscard]] BreathState get_current_state() const;

    /**
     * @brief 判断当前是否处于平滑过渡中
     * @return bool
     */
    [[nodiscard]] bool is_in_transition() const;

private:
    /**
     * @brief 根据特定状态及其持续时间计算亮度值
     * @param state 目标状态
     * @param time_sec 该状态累计运行时间（秒）
     * @return 计算出的亮度值
     */
    [[nodiscard]] double calculate_state_brightness(BreathState state, double time_sec) const;

    /**
     * @brief 获取特定状态的基准颜色值
     * @param state 目标状态
     * @return ColorRGB
     */
    [[nodiscard]] ColorRGB get_state_color(BreathState state) const;

    /**
     * @brief 获取特定状态下的基准旋转角速度（弧度/秒）
     * @param state 目标状态
     * @return 角速度值
     */
    [[nodiscard]] double get_state_base_spin_rate(BreathState state) const;

    BreathState current_state_; ///< 当前目标状态
    double state_time_;         ///< 当前状态已持续的时间（秒）
    double rotation_angle_;     ///< 当前旋转角度值（弧度）

    // 过渡状态相关变量
    bool in_transition_;                 ///< 是否处于过渡状态中
    double transition_time_;             ///< 过渡已持续的时间（秒）
    const double transition_duration_;   ///< 过渡总时长（固定 0.5 秒）

    double transition_start_brightness_; ///< 过渡起点时的亮度
    ColorRGB transition_start_color_;    ///< 过渡起点时的颜色
    double transition_start_spin_rate_;  ///< 过渡起点时的自旋角速度

    double current_brightness_;          ///< 当前帧最终计算得到的亮度
    ColorRGB current_color_;             ///< 当前帧最终计算得到的颜色
};
