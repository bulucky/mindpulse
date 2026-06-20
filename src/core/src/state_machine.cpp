/**
 * @file state_machine.cpp
 * @brief 状态机模块实现，处理事件流、嵌套计数与多工具聚合
 */

#include "state_machine.h"
#include <cstdio>
// #include <algorithm>

/**
 * @brief 辅助函数：状态枚举转换为易读的字符串，便于日志输出
 */
static const char* breath_state_to_string(BreathState state) {
    switch (state) {
        case BreathState::STOPPED:
            return "STOPPED";
        case BreathState::IDLE:
            return "IDLE";
        case BreathState::RUNNING:
            return "RUNNING";
        case BreathState::PENDING:
            return "PENDING";
    }
    return "UNKNOWN";
}

/**
 * @brief 辅助函数：事件枚举转换为易读的字符串，便于日志输出
 */
static const char* event_to_string(StateMachineEvent event) {
    switch (event) {
        case StateMachineEvent::SESSION_START:
            return "SESSION_START";
        case StateMachineEvent::SESSION_END:
            return "SESSION_END";
        case StateMachineEvent::USER_PROMPT_SUBMIT:
            return "USER_PROMPT_SUBMIT";
        case StateMachineEvent::TOOL_START:
            return "TOOL_START";
        case StateMachineEvent::TOOL_END:
            return "TOOL_END";
        case StateMachineEvent::PERMISSION_REQUEST:
            return "PERMISSION_REQUEST";
        case StateMachineEvent::PERMISSION_DENIED:
            return "PERMISSION_DENIED";
        case StateMachineEvent::AGENT_STOP:
            return "AGENT_STOP";
        case StateMachineEvent::UNKNOWN:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}

ToolStateContext& StateMachine::get_or_create_context(const std::string& tool_id) {
    // 假设调用此私有方法前已经锁定了 mutex_
    auto it = tools_.find(tool_id);
    if (it == tools_.end()) {
        ToolStateContext ctx;
        ctx.tool_id = tool_id;
        tools_[tool_id] = ctx;
    }
    return tools_[tool_id];
}

BreathState StateMachine::handle_event(const std::string& tool_id, StateMachineEvent event) {
    std::lock_guard<std::mutex> lock(mutex_);
    ToolStateContext& ctx = get_or_create_context(tool_id);
    BreathState old_state = ctx.state;

    switch (event) {
        case StateMachineEvent::SESSION_START:
            if (ctx.state == BreathState::STOPPED) {
                ctx.state = BreathState::IDLE;
            }
            ctx.active_tool_count = 0;
            break;

        case StateMachineEvent::SESSION_END:
            ctx.state = BreathState::STOPPED;
            ctx.active_tool_count = 0;
            break;

        case StateMachineEvent::USER_PROMPT_SUBMIT:
            ctx.state = BreathState::RUNNING;
            break;

        case StateMachineEvent::TOOL_START:
            ctx.state = BreathState::RUNNING;
            ctx.active_tool_count++;
            break;

        case StateMachineEvent::TOOL_END:
            if (ctx.active_tool_count > 0) {
                ctx.active_tool_count--;
            }
            if (ctx.active_tool_count == 0) {
                if (ctx.state == BreathState::RUNNING) {
                    ctx.state = BreathState::IDLE;
                }
            }
            break;

        case StateMachineEvent::PERMISSION_REQUEST:
            ctx.state = BreathState::PENDING;
            break;

        case StateMachineEvent::PERMISSION_DENIED:
            ctx.state = BreathState::RUNNING;
            break;

        case StateMachineEvent::AGENT_STOP:
            ctx.state = BreathState::IDLE;
            ctx.active_tool_count = 0;
            break;

        case StateMachineEvent::UNKNOWN:
        default:
            break;
    }

    if (old_state != ctx.state) {
        std::printf("[StateMachine] 工具 '%s' 触发事件 %s 状态转换: %s -> %s (嵌套深度: %d)\n",
                    tool_id.c_str(),
                    event_to_string(event),
                    breath_state_to_string(old_state),
                    breath_state_to_string(ctx.state),
                    ctx.active_tool_count);
    }

    return get_aggregate_state_unlocked();
}

BreathState StateMachine::tick(double delta_time_sec) {
    (void)delta_time_sec;
    std::lock_guard<std::mutex> lock(mutex_);
    return get_aggregate_state_unlocked();
}

BreathState StateMachine::get_aggregate_state() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return get_aggregate_state_unlocked();
}

BreathState StateMachine::get_aggregate_state_unlocked() const {
    if (tools_.empty()) {
        return BreathState::STOPPED;
    }

    bool has_idle = false;
    bool has_running = false;
    bool has_pending = false;

    for (const auto& [id, ctx] : tools_) {
        if (ctx.state == BreathState::PENDING) {
            has_pending = true;
        } else if (ctx.state == BreathState::RUNNING) {
            has_running = true;
        } else if (ctx.state == BreathState::IDLE) {
            has_idle = true;
        }
    }

    if (has_pending) return BreathState::PENDING;
    if (has_running) return BreathState::RUNNING;
    if (has_idle) return BreathState::IDLE;
    return BreathState::STOPPED;
}

BreathState StateMachine::get_tool_state(const std::string& tool_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tools_.find(tool_id);
    if (it != tools_.end()) {
        return it->second.state;
    }
    return BreathState::STOPPED;
}

int StateMachine::get_tool_active_count(const std::string& tool_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = tools_.find(tool_id);
    if (it != tools_.end()) {
        return it->second.active_tool_count;
    }
    return 0;
}

void StateMachine::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    tools_.clear();
}
