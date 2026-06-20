/**
 * @file test_config_manager.cpp
 * @brief 验证独立配置文件和动态更新缓存机制的单元测试
 */

#include <gtest/gtest.h>
#include "config_manager.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

TEST(ConfigManagerTest, ParseAndCacheBuffer) {
    std::string test_dir = "./test_config";
    std::string test_yaml_path = test_dir + "/claude.yaml";

    // 1. 确保清空临时测试目录
    if (std::filesystem::exists(test_dir)) {
        std::filesystem::remove_all(test_dir);
    }

    // 2. 实例化并验证：文件夹会自动创建，当配置不存在时会生成包含默认 claude 30种事件映射的备份模板
    {
        ConfigManager manager(test_dir);
        EXPECT_TRUE(std::filesystem::exists(test_dir));

        // get_event_mapping 内部检测到没加载，会自动调用生成机制并载入
        EXPECT_EQ(manager.get_event_mapping("claude", "SessionStart"), StateMachineEvent::SESSION_START);
        EXPECT_EQ(manager.get_event_mapping("claude", "Elicitation"), StateMachineEvent::PERMISSION_REQUEST);
        EXPECT_EQ(manager.get_event_mapping("claude", "PostToolUse"), StateMachineEvent::TOOL_END);
        EXPECT_EQ(manager.get_event_mapping("claude", "NonExistent"), StateMachineEvent::UNKNOWN);

        EXPECT_EQ(manager.get_tool_name("claude"), "Claude Code");
    }

    // 3. 测试运行时更改文件时的动态重载（无需重启）
    ConfigManager manager(test_dir);

    // 手动覆盖写入一个新配置
    std::ofstream out(test_yaml_path);
    out << "claude:\n"
        << "  name: \"Modified Claude\"\n"
        << "  events:\n"
        << "    SessionStart: \"SESSION_END\"\n"; // 改变原 SessionStart 映射
    out.close();

    // 强制文件修改时间刷新（某些系统文件更新有微弱延迟，等待 100ms）
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 调用 get_event_mapping 会触发 update 并重载缓冲区
    EXPECT_EQ(manager.get_event_mapping("claude", "SessionStart"), StateMachineEvent::SESSION_END);
    EXPECT_EQ(manager.get_tool_name("claude"), "Modified Claude");

    // 4. 清理测试目录
    std::filesystem::remove_all(test_dir);
}
