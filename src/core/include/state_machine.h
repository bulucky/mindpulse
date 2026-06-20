/**
 * @file state_machine.h
 * @brief 状态机模块声明，支持多工具状态跟踪、嵌套计数及优先级聚合
 */

#pragma once

#include <string>
#include <unordered_map>
#include <mutex>
#include "breath_engine.h" // 复用 BreathState
#include "yaml_parser.h"   // 复用 StateMachineEvent

/**
 * @brief 单个 AI 工具的状态上下文
 */
struct ToolStateContext {
    std::string tool_id;                      ///< 工具唯一标识 (如 "claude")
    BreathState state = BreathState::STOPPED; ///< 当前状态
    int active_tool_count = 0;                ///< 活跃的工具嵌套计数
};

/**
 * @brief 系统状态机类，负责对 AI 工具生命周期事件与状态映射进行统一管理与聚合
 */
class StateMachine {
public:
    StateMachine() = default;

    /**
     * @brief 处理特定工具的标准化事件，驱动状态转换
     * @param tool_id 工具标识（例如 "claude", "codex"）
     * @param event 标准化状态机事件
     * @return BreathState 转换后计算出的系统全局聚合状态
     */
    BreathState handle_event(const std::string& tool_id, StateMachineEvent event);

    /**
     * @brief 驱动状态机时间步进（每帧更新），保留接口以备未来扩展
     * @param delta_time_sec 帧间隔时间（秒）
     * @return BreathState 更新后计算出的系统全局聚合状态
     */
    BreathState tick(double delta_time_sec);

    /**
     * @brief 计算并获取当前的系统全局聚合状态
     * @return BreathState 聚合状态
     */
    [[nodiscard]] BreathState get_aggregate_state() const;

    /**
     * @brief 查询特定工具的当前状态
     * @param tool_id 工具标识
     * @return BreathState 状态
     */
    [[nodiscard]] BreathState get_tool_state(const std::string& tool_id) const;

    /**
     * @brief 查询特定工具的嵌套活跃计数
     * @param tool_id 工具标识
     * @return int 嵌套计数
     */
    [[nodiscard]] int get_tool_active_count(const std::string& tool_id) const;

    /**
     * @brief 清空并重置所有工具上下文为 STOPPED 状态
     */
    void reset();

private:
    /**
     * @brief 辅助方法：获取或创建工具状态上下文
     */
    ToolStateContext& get_or_create_context(const std::string& tool_id);

    /**
     * @brief 辅助方法：无锁获取全局聚合状态（用于类内部调用）
     */
    [[nodiscard]] BreathState get_aggregate_state_unlocked() const;

    std::unordered_map<std::string, ToolStateContext> tools_; ///< 所有被追踪的工具集合
    mutable std::mutex mutex_;                                ///< 线程同步互斥锁

};

