/**
 * @file config_manager.h
 * @brief 系统配置管理器声明，使用 yaml-cpp 管理和动态重载 AI 工具各自的配置文件
 */

#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>
#include "core/yaml_parser.h"

/**
 * @brief 存储单个工具的缓存结构体，保存了文件最后修改时间以及已建立的映射表
 */
struct ToolConfig {
    std::string tool_name;                                        ///< 托盘及显示所使用的工具名
    std::filesystem::file_time_type last_write_time;              ///< 配置文件最后修改时间
    std::unordered_map<std::string, StateMachineEvent> event_map; ///< 该工具的事件映射缓冲区
    bool loaded = false;                                          ///< 是否加载成功
};

/**
 * @brief 配置管理器，动态加载和缓存各 AI 工具的单独 YAML 配置文件
 */
class ConfigManager {
public:
    /**
     * @brief 构造函数
     * @param config_dir 配置文件所在的目录路径（例如 "./config"）
     */
    ConfigManager(const std::string& config_dir);

    /**
     * @brief 核心映射获取方法：在内存缓冲区中检索映射，并自动按需检测更新重载
     * @param tool_id 工具唯一标识 (例如 "claude", "codex")
     * @param raw_event_name 传来的原生生命周期事件名 (例如 "PreToolUse")
     * @return StateMachineEvent 对应的状态机标准处理动作
     */
    StateMachineEvent get_event_mapping(const std::string& tool_id, const std::string& raw_event_name);

    /**
     * @brief 获取指定工具的托盘显示名称
     * @param tool_id 工具标识
     * @return std::string
     */
    std::string get_tool_name(const std::string& tool_id);

    /**
     * @brief 手动为某个工具预加载配置（用于在 HTTP 后端启动前进行映射表预热）
     * @param tool_id 工具标识
     */
    void preload_tool_config(const std::string& tool_id);

private:
    /**
     * @brief 使用 yaml-cpp 解析特定工具的 yaml 文件并刷新缓冲区
     */
    void load_tool_config(const std::string& tool_id);

    /**
     * @brief 当文件被误删或不存在时，写入一套默认的全量配置，保证系统健壮性
     */
    void write_default_config_if_missing(const std::string& tool_id);

    std::filesystem::path config_dir_;                         ///< 配置文件根路径
    std::unordered_map<std::string, ToolConfig> tool_configs_; ///< 运行时工具配置映射缓存区 (Buffer)
};
