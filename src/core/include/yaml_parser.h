/**
 * @file yaml_parser.h
 * @brief 声明事件映射的统一状态机事件枚举
 */

#pragma once

#include <string>

/**
 * @brief 统一的状态机处理事件，将各 AI 工具的原始 Hook 事件归一化
 */
enum class StateMachineEvent {
    SESSION_START,       ///< 会话启动 (对应 SessionStart 等)
    SESSION_ACTIVE,      ///< 会话已活跃但不重置运行中/等待中状态
    SESSION_END,         ///< 会话结束 (对应 SessionEnd 等)
    USER_PROMPT_SUBMIT,  ///< 用户提交 Prompt (对应 UserPromptSubmit 等)
    AGENT_RUNNING,       ///< Agent 活动中但不改变工具嵌套计数
    TOOL_START,          ///< 工具调用开始 (对应 PreToolUse, SubagentStart 等)
    TOOL_END,            ///< 工具调用结束 (对应 PostToolUse, SubagentStop 等)
    PERMISSION_REQUEST,  ///< 触发权限请求/人工干预 (对应 PermissionRequest, Elicitation 等)
    PERMISSION_DENIED,   ///< 权限拒绝 (对应 PermissionDenied 等)
    AGENT_STOP,          ///< 响应结束 (对应 Stop 等)
    NOOP,                ///< 已知但不影响指示灯状态的事件
    UNKNOWN              ///< 未映射事件
};

/**
 * @brief 将 YAML 中的映射目标字符串转换为状态机事件枚举
 * @param str 目标映射名称，如 "SESSION_START"
 * @return StateMachineEvent
 */
inline StateMachineEvent parse_event_string(const std::string& str) {
    if (str == "SESSION_START")       return StateMachineEvent::SESSION_START;
    if (str == "SESSION_ACTIVE")      return StateMachineEvent::SESSION_ACTIVE;
    if (str == "SESSION_END")         return StateMachineEvent::SESSION_END;
    if (str == "USER_PROMPT_SUBMIT")  return StateMachineEvent::USER_PROMPT_SUBMIT;
    if (str == "AGENT_RUNNING")       return StateMachineEvent::AGENT_RUNNING;
    if (str == "TOOL_START")          return StateMachineEvent::TOOL_START;
    if (str == "TOOL_END")            return StateMachineEvent::TOOL_END;
    if (str == "PERMISSION_REQUEST")  return StateMachineEvent::PERMISSION_REQUEST;
    if (str == "PERMISSION_DENIED")   return StateMachineEvent::PERMISSION_DENIED;
    if (str == "AGENT_STOP")          return StateMachineEvent::AGENT_STOP;
    if (str == "NOOP")                return StateMachineEvent::NOOP;
    return StateMachineEvent::UNKNOWN;
}
