/**
 * @file test_state_machine.cpp
 * @brief 验证状态机的多工具隔离、状态流转、嵌套深度计数与优先级聚合的单元测试
 */

#include <gtest/gtest.h>
#include "state_machine.h"

// 验证初始状态为空，聚合状态为 STOPPED
TEST(StateMachineTest, InitialStateAndReset) {
    StateMachine sm;
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::STOPPED);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::STOPPED);

    // 触发 session_start 后应该到 IDLE
    sm.handle_event("claude", StateMachineEvent::SESSION_START);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::IDLE);
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::IDLE);

    // 重置后应该重新归为 STOPPED
    sm.reset();
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::STOPPED);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::STOPPED);
}

// 验证单个工具的基本事件转换流程
TEST(StateMachineTest, BasicTransitions) {
    StateMachine sm;

    // 1. 未注册的工具收到 TOOL_START 自动恢复为 RUNNING
    sm.handle_event("claude", StateMachineEvent::TOOL_START);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::RUNNING);

    // 2. 收到 SESSION_END 强制切到 STOPPED
    sm.handle_event("claude", StateMachineEvent::SESSION_END);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::STOPPED);

    // 3. 收到 SESSION_START 从 STOPPED 切到 IDLE
    sm.handle_event("claude", StateMachineEvent::SESSION_START);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::IDLE);

    // 4. 用户提交 Prompt 进入 RUNNING
    sm.handle_event("claude", StateMachineEvent::USER_PROMPT_SUBMIT);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::RUNNING);

    // 5. 收到 AGENT_STOP 正常归回 IDLE
    sm.handle_event("claude", StateMachineEvent::AGENT_STOP);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::IDLE);
}

// 验证嵌套工具计数 (active_tool_count) 机制
TEST(StateMachineTest, ReentrancyCounting) {
    StateMachine sm;
    sm.handle_event("claude", StateMachineEvent::SESSION_START);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::IDLE);
    EXPECT_EQ(sm.get_tool_active_count("claude"), 0);

    // 第一次工具开始 (嵌套深度 1 -> RUNNING)
    sm.handle_event("claude", StateMachineEvent::TOOL_START);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::RUNNING);
    EXPECT_EQ(sm.get_tool_active_count("claude"), 1);

    // 第二次工具开始 (嵌套深度 2 -> 保持 RUNNING)
    sm.handle_event("claude", StateMachineEvent::TOOL_START);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::RUNNING);
    EXPECT_EQ(sm.get_tool_active_count("claude"), 2);

    // 第一次工具结束 (嵌套深度 1 -> 保持 RUNNING)
    sm.handle_event("claude", StateMachineEvent::TOOL_END);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::RUNNING);
    EXPECT_EQ(sm.get_tool_active_count("claude"), 1);

    // 第二次工具结束 (嵌套深度 0 -> 退回 IDLE)
    sm.handle_event("claude", StateMachineEvent::TOOL_END);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::IDLE);
    EXPECT_EQ(sm.get_tool_active_count("claude"), 0);
}

// 验证 PENDING 授权阻塞状态（不带超时降级，符合阻塞设计）
TEST(StateMachineTest, PendingBlockingAndRecovery) {
    StateMachine sm;
    sm.handle_event("claude", StateMachineEvent::SESSION_START);
    sm.handle_event("claude", StateMachineEvent::TOOL_START); // RUNNING

    // 1. 触发权限请求 -> 进入 PENDING
    sm.handle_event("claude", StateMachineEvent::PERMISSION_REQUEST);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::PENDING);

    // 2. 模拟时间推移，验证确实不会发生超时降级，一直保持 PENDING 阻塞
    sm.tick(10.0);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::PENDING);
    sm.tick(60.0);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::PENDING);

    // 3. 用户拒绝权限 -> 回退到 RUNNING
    sm.handle_event("claude", StateMachineEvent::PERMISSION_DENIED);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::RUNNING);

    // 4. 再次触发权限请求 -> 进入 PENDING
    sm.handle_event("claude", StateMachineEvent::PERMISSION_REQUEST);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::PENDING);

    // 5. 用户确认并调用了新工具 -> 转为 RUNNING 恢复执行
    sm.handle_event("claude", StateMachineEvent::TOOL_START);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::RUNNING);
}

// 验证权限等待后收到工具结束事件时可以离开 PENDING，避免指示灯卡死
TEST(StateMachineTest, PendingClearsOnToolEnd) {
    StateMachine sm;
    sm.handle_event("claude", StateMachineEvent::SESSION_START);
    sm.handle_event("claude", StateMachineEvent::TOOL_START);
    sm.handle_event("claude", StateMachineEvent::PERMISSION_REQUEST);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::PENDING);
    EXPECT_EQ(sm.get_tool_active_count("claude"), 1);

    sm.handle_event("claude", StateMachineEvent::TOOL_END);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::IDLE);
    EXPECT_EQ(sm.get_tool_active_count("claude"), 0);
}

// 验证等待用户确认时，普通活动信号不能抢占 PENDING 指示
TEST(StateMachineTest, PendingIsStickyUntilExplicitRecovery) {
    StateMachine sm;
    sm.handle_event("claude", StateMachineEvent::SESSION_START);
    sm.handle_event("claude", StateMachineEvent::USER_PROMPT_SUBMIT);
    sm.handle_event("claude", StateMachineEvent::PERMISSION_REQUEST);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::PENDING);

    sm.handle_event("claude", StateMachineEvent::AGENT_RUNNING);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::PENDING);
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::PENDING);

    sm.handle_event("claude", StateMachineEvent::NOOP);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::PENDING);

    sm.handle_event("claude", StateMachineEvent::PERMISSION_DENIED);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::RUNNING);
}

// 验证无配对事件不会增加嵌套计数，活动信号只改变状态
TEST(StateMachineTest, NonCountingEventsDoNotLeakActiveCount) {
    StateMachine sm;
    sm.handle_event("claude", StateMachineEvent::SESSION_START);

    sm.handle_event("claude", StateMachineEvent::NOOP);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::IDLE);
    EXPECT_EQ(sm.get_tool_active_count("claude"), 0);

    sm.handle_event("claude", StateMachineEvent::AGENT_RUNNING);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::RUNNING);
    EXPECT_EQ(sm.get_tool_active_count("claude"), 0);

    sm.handle_event("claude", StateMachineEvent::AGENT_STOP);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::IDLE);
    EXPECT_EQ(sm.get_tool_active_count("claude"), 0);
}

// 验证多工具状态的聚合优先级决策 (PENDING > RUNNING > IDLE > STOPPED)
TEST(StateMachineTest, MultiToolAggregation) {
    StateMachine sm;

    // 1. 初始化两个工具
    sm.handle_event("claude", StateMachineEvent::SESSION_START);
    sm.handle_event("codex", StateMachineEvent::SESSION_START);
    // 两个都处于 IDLE -> 聚合状态为 IDLE
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::IDLE);
    EXPECT_EQ(sm.get_tool_state("codex"), BreathState::IDLE);
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::IDLE);

    // 2. 一个 RUNNING, 一个 IDLE -> 聚合状态为 RUNNING
    sm.handle_event("claude", StateMachineEvent::TOOL_START);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::RUNNING);
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::RUNNING);

    // 3. 一个 RUNNING, 一个 PENDING -> 聚合状态为 PENDING
    sm.handle_event("codex", StateMachineEvent::PERMISSION_REQUEST);
    EXPECT_EQ(sm.get_tool_state("codex"), BreathState::PENDING);
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::PENDING);

    // 4. PENDING 工具被解决 (RUNNING) -> 聚合状态转为 RUNNING
    sm.handle_event("codex", StateMachineEvent::TOOL_START);
    EXPECT_EQ(sm.get_tool_state("codex"), BreathState::RUNNING);
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::RUNNING);

    // 5. 一个回归 IDLE，另一个依然 RUNNING -> 聚合状态为 RUNNING
    sm.handle_event("codex", StateMachineEvent::AGENT_STOP);
    EXPECT_EQ(sm.get_tool_state("codex"), BreathState::IDLE);
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::RUNNING);

    // 6. 另一个也回归 IDLE -> 聚合状态为 IDLE
    sm.handle_event("claude", StateMachineEvent::TOOL_END);
    EXPECT_EQ(sm.get_tool_state("claude"), BreathState::IDLE);
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::IDLE);

    // 7. 均退出 Session -> 聚合状态为 STOPPED
    sm.handle_event("claude", StateMachineEvent::SESSION_END);
    sm.handle_event("codex", StateMachineEvent::SESSION_END);
    EXPECT_EQ(sm.get_aggregate_state(), BreathState::STOPPED);
}
