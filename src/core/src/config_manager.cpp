/**
 * @file config_manager.cpp
 * @brief 配置管理器实现，使用 yaml-cpp 进行文件解析并提供高效缓存更新逻辑
 */

#include "config_manager.h"
#include <yaml-cpp/yaml.h>
#include <fstream>
// #include <iostream>

ConfigManager::ConfigManager(const std::string& config_dir)
    : config_dir_(config_dir) {
    // 自动创建配置目录（如果不存在）
    try {
        std::filesystem::create_directories(config_dir_);
    } catch (...) {
        // 容错处理
    }
}

StateMachineEvent ConfigManager::get_event_mapping(const std::string& tool_id, const std::string& raw_event_name) {
    std::filesystem::path filepath = config_dir_ / (tool_id + ".yaml");

    // 1. 文件不存在则自动生成默认备份模板
    if (!std::filesystem::exists(filepath)) {
        write_default_config_if_missing(tool_id);
    }

    try {
        auto current_write_time = std::filesystem::last_write_time(filepath);
        auto it = tool_configs_.find(tool_id);

        // 2. 检查更新判定：如果从未载入过，或者文件最后修改时间有更新，则触发重新解析并重置缓冲区
        if (it == tool_configs_.end() || !it->second.loaded || it->second.last_write_time != current_write_time) {
            std::printf("[ConfigManager] config for tool '%s' is missing or changed; refreshing event map cache...\n", tool_id.c_str());
            load_tool_config(tool_id);
        }
    } catch (const std::exception& e) {
        std::printf("[ConfigManager] failed to check config file timestamp: %s\n", e.what());
    }

    // 3. 在内存缓冲区中做映射查找
    auto it = tool_configs_.find(tool_id);
    if (it != tool_configs_.end() && it->second.loaded) {
        auto map_it = it->second.event_map.find(raw_event_name);
        if (map_it != it->second.event_map.end()) {
            return map_it->second;
        }
    }

    return StateMachineEvent::UNKNOWN;
}

std::string ConfigManager::get_tool_name(const std::string& tool_id) {
    auto it = tool_configs_.find(tool_id);
    if (it != tool_configs_.end() && it->second.loaded) {
        return it->second.tool_name;
    }
    return tool_id;
}

void ConfigManager::preload_tool_config(const std::string& tool_id) {
    std::filesystem::path filepath = config_dir_ / (tool_id + ".yaml");
    if (!std::filesystem::exists(filepath)) {
        write_default_config_if_missing(tool_id);
    }
    load_tool_config(tool_id);
}

void ConfigManager::load_tool_config(const std::string& tool_id) {
    std::filesystem::path filepath = config_dir_ / (tool_id + ".yaml");

    try {
        YAML::Node doc = YAML::LoadFile(filepath.string());

        // 检查顶层是否包含与 tool_id 相同的键值
        if (!doc[tool_id]) {
            std::printf("[ConfigManager] error: %s.yaml does not contain top-level key '%s'\n", tool_id.c_str(), tool_id.c_str());
            return;
        }

        YAML::Node tool_node = doc[tool_id];
        ToolConfig config;
        config.last_write_time = std::filesystem::last_write_time(filepath);
        config.loaded = true;

        if (tool_node["name"]) {
            config.tool_name = tool_node["name"].as<std::string>();
        } else {
            config.tool_name = tool_id;
        }

        // 解析并缓存事件列表
        if (tool_node["events"]) {
            YAML::Node events_node = tool_node["events"];
            for (auto it = events_node.begin(); it != events_node.end(); ++it) {
                std::string raw_ev = it->first.as<std::string>();
                std::string mapped_ev = it->second.as<std::string>();
                config.event_map[raw_ev] = parse_event_string(mapped_ev);
            }
        }

        // 更新热点缓存区 (Buffer)
        tool_configs_[tool_id] = config;
        std::printf("[ConfigManager] reloaded %zu event mappings for tool '%s'.\n", config.event_map.size(), tool_id.c_str());

    } catch (const std::exception& e) {
        std::printf("[ConfigManager] failed to parse %s.yaml: %s\n", tool_id.c_str(), e.what());
    }
}

void ConfigManager::write_default_config_if_missing(const std::string& tool_id) {
    std::filesystem::path filepath = config_dir_ / (tool_id + ".yaml");
    std::ofstream out(filepath);
    if (!out.is_open()) return;

    if (tool_id == "claude") {
        out << "claude:\n"
            << "  name: \"Claude Code\"\n"
            << "  events:\n"
            << "    # 1. Session and startup lifecycle\n"
            << "    SessionStart: \"SESSION_START\"\n"
            << "    Setup: \"SESSION_START\"\n"
            << "    SessionEnd: \"SESSION_END\"\n"
            << "    InstructionsLoaded: \"SESSION_ACTIVE\"\n"
            << "    ConfigChange: \"NOOP\"\n\n"
            << "    # 2. User interaction and input\n"
            << "    UserPromptSubmit: \"USER_PROMPT_SUBMIT\"\n"
            << "    UserPromptExpansion: \"USER_PROMPT_SUBMIT\"\n\n"
            << "    # 3. Tool invocation lifecycle\n"
            << "    PreToolUse: \"TOOL_START\"\n"
            << "    PostToolUse: \"TOOL_END\"\n"
            << "    PostToolUseFailure: \"TOOL_END\"\n"
            << "    PostToolBatch: \"AGENT_RUNNING\"\n\n"
            << "    # 4. Permission and elicitation gates (PENDING state)\n"
            << "    PermissionRequest: \"PERMISSION_REQUEST\"\n"
            << "    PermissionDenied: \"PERMISSION_DENIED\"\n"
            << "    Elicitation: \"PERMISSION_REQUEST\"\n"
            << "    ElicitationResult: \"AGENT_RUNNING\"\n\n"
            << "    # 5. Turn completion and stop signals\n"
            << "    Stop: \"AGENT_STOP\"\n"
            << "    StopFailure: \"AGENT_STOP\"\n"
            << "    TeammateIdle: \"NOOP\"\n\n"
            << "    # 6. Subtasks and concurrent subagents\n"
            << "    SubagentStart: \"TOOL_START\"\n"
            << "    SubagentStop: \"TOOL_END\"\n"
            << "    TaskCreated: \"TOOL_START\"\n"
            << "    TaskCompleted: \"TOOL_END\"\n\n"
            << "    # 7. Environment and filesystem events\n"
            << "    CwdChanged: \"NOOP\"\n"
            << "    FileChanged: \"NOOP\"\n"
            << "    WorktreeCreate: \"NOOP\"\n"
            << "    WorktreeRemove: \"NOOP\"\n\n"
            << "    # 8. Compaction and low-level adjustment events\n"
            << "    PreCompact: \"AGENT_RUNNING\"\n"
            << "    PostCompact: \"AGENT_RUNNING\"\n\n"
            << "    # 9. Auxiliary and message display events\n"
            << "    Notification: \"NOOP\"\n"
            << "    MessageDisplay: \"AGENT_RUNNING\"\n";
    } else {
        // 通用备用模版
        out << tool_id << ":\n"
            << "  name: \"" << tool_id << "\"\n"
            << "  events:\n"
            << "    session_start: \"SESSION_START\"\n"
            << "    session_end: \"SESSION_END\"\n"
            << "    tool_start: \"TOOL_START\"\n"
            << "    tool_end: \"TOOL_END\"\n"
            << "    user_input: \"PERMISSION_REQUEST\"\n"
            << "    agent_stop: \"AGENT_STOP\"\n";
    }
    out.close();
}
