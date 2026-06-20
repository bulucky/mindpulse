/**
 * @file test_server.cpp
 * @brief HTTP 服务与状态机对接的集成测试
 */

#include <gtest/gtest.h>
#include "server.h"
#include "core/include/config_manager.h"
#include "core/include/state_machine.h"
#include <httplib.h>
#include <nlohmann/json.hpp>

// 集成测试：启动 HTTP 服务，通过 Client 发送事件，检验状态机状态是否联动更新
TEST(HttpServerTest, DispatchEventsToStateMachine) {
    // 1. 初始化配置管理器与状态机
    // 假设运行目录为 build 目录，config_dir 需要指向正确的路径
    // 如果在根目录下运行，则是 "./config"。在 windows Cmake 构架下，使用 "../config" 或 "./config"
    // 为保险起见，我们可以检测存在哪个目录，然后使用之。
    std::string config_dir = "./config";
    if (!std::filesystem::exists(config_dir)) {
        config_dir = "../config";
    }
    
    ConfigManager config_mgr(config_dir);
    StateMachine state_machine;

    // 确保预加载 claude 的配置
    config_mgr.preload_tool_config("claude");

    // 2. 初始化并启动 HTTP 服务，监听本地 19876 端口
    HttpServer server(config_mgr, state_machine);
    ASSERT_TRUE(server.start("127.0.0.1", 19876));
    EXPECT_TRUE(server.is_running());

    // 等待后台线程完成 Socket 绑定
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 创建 HTTP 客户端
    httplib::Client client("127.0.0.1", 19876);
    client.set_connection_timeout(0, 500000); // 500ms 超时

    // 2.1 初始状态可查询，且未收到任何 Hook 事件
    {
        auto status_res = client.Get("/status/claude");
        ASSERT_NE(status_res, nullptr);
        EXPECT_EQ(status_res->status, 200);
        auto status = nlohmann::json::parse(status_res->body);
        EXPECT_EQ(status["state"], "STOPPED");
        EXPECT_EQ(status["last_event"], "UNKNOWN");
        EXPECT_EQ(status["received_event_count"], 0);
    }

    // 3. 测试发送非法 JSON
    {
        auto res = client.Post("/hook/claude", "invalid { json", "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 400);
    }

    // 4. 测试发送缺少 event 的 JSON
    {
        nlohmann::json bad_body = {{"tool_id", "claude"}};
        auto res = client.Post("/hook/claude", bad_body.dump(), "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 400);
    }

    // 5. 测试发送官方 HTTP hook 字段 hook_event_name
    {
        nlohmann::json body = {{"hook_event_name", "SessionStart"}};
        auto res = client.Post("/hook/claude", body.dump(), "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 200);
        EXPECT_EQ(res->body, "{}");
        EXPECT_EQ(res->get_header_value("Content-Type"), "application/json");

        // 检验状态机联动：SessionStart -> IDLE
        EXPECT_EQ(state_machine.get_tool_state("claude"), BreathState::IDLE);
        EXPECT_EQ(state_machine.get_aggregate_state(), BreathState::IDLE);

        auto status_res = client.Get("/status/claude");
        ASSERT_NE(status_res, nullptr);
        EXPECT_EQ(status_res->status, 200);
        auto status = nlohmann::json::parse(status_res->body);
        EXPECT_EQ(status["state"], "IDLE");
        EXPECT_EQ(status["aggregate_state"], "IDLE");
        EXPECT_EQ(status["active_tool_count"], 0);
        EXPECT_EQ(status["last_event"], "SESSION_START");
        EXPECT_EQ(status["received_event_count"], 1);
        EXPECT_EQ(status["last_raw_event"], "SessionStart");
        EXPECT_EQ(status["last_event_field"], "hook_event_name");
        EXPECT_EQ(status["last_error"], "");
    }

    // 6. 测试发送兼容字段 event (工具调用启动)
    {
        nlohmann::json body = {{"event", "PreToolUse"}};
        auto res = client.Post("/hook/claude", body.dump(), "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 200);
        EXPECT_EQ(res->body, "{}");
        EXPECT_EQ(res->get_header_value("Content-Type"), "application/json");

        // 检验状态机联动：PreToolUse -> TOOL_START -> RUNNING
        EXPECT_EQ(state_machine.get_tool_state("claude"), BreathState::RUNNING);
        EXPECT_EQ(state_machine.get_tool_active_count("claude"), 1);
        EXPECT_EQ(state_machine.get_aggregate_state(), BreathState::RUNNING);
    }

    // 7. 测试发送 PermissionRequest (授权等待弹窗)
    {
        nlohmann::json body = {{"event", "PermissionRequest"}};
        auto res = client.Post("/hook/claude", body.dump(), "application/json");
        ASSERT_NE(res, nullptr);
        EXPECT_EQ(res->status, 200);

        // 检验状态机联动：PermissionRequest -> PERMISSION_REQUEST -> PENDING
        EXPECT_EQ(state_machine.get_tool_state("claude"), BreathState::PENDING);
        EXPECT_EQ(state_machine.get_aggregate_state(), BreathState::PENDING);
    }

    // 8. 停止服务器
    server.stop();
    EXPECT_FALSE(server.is_running());
}
