/**
 * @file server.cpp
 * @brief HTTP 服务器实现，分发并映射接收到的 Hooks 事件
 */
#include "server.h"

#include <cstdio>

#include <nlohmann/json.hpp>

#include "config_manager.h"
#include "state_machine.h"

HttpServer::HttpServer(ConfigManager& config_mgr, StateMachine& state_machine)
    : config_mgr_(config_mgr), state_machine_(state_machine), port_(9876) {

    // 注册核心 Hook 接收端路由 (使用正则路径捕获 tool_id)
    svr_.Post(R"(/hook/([a-zA-Z0-9_\-]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string tool_id = req.matches[1].str();

        // 1. 解析请求 JSON 载荷
        nlohmann::json body;
        try {
            body = nlohmann::json::parse(req.body);
        } catch (const std::exception& e) {
            std::printf("[HttpServer] 解析 JSON 载荷失败 (来自工具: '%s'): %s\n", tool_id.c_str(), e.what());
            res.status = 400;
            res.set_content("Invalid JSON body", "text/plain");
            return;
        }

        // 2. 检查必需字段。Claude Code 官方 HTTP hook 使用 hook_event_name；
        // event 保留为手工测试和旧配置兼容字段。
        const char* event_field = nullptr;
        if (body.contains("hook_event_name") && body["hook_event_name"].is_string()) {
            event_field = "hook_event_name";
        } else if (body.contains("event") && body["event"].is_string()) {
            event_field = "event";
        }

        if (event_field == nullptr) {
            std::printf("[HttpServer] 缺少必需的 'hook_event_name' 字符串字段 (来自工具: '%s')\n", tool_id.c_str());
            res.status = 400;
            res.set_content("Missing or invalid 'hook_event_name' parameter", "text/plain");
            return;
        }

        std::string raw_event = body[event_field].get<std::string>();

        // 3. 通过 ConfigManager 将原始 Hook 事件转换为标准化状态机事件
        StateMachineEvent mapped_event = config_mgr_.get_event_mapping(tool_id, raw_event);

        if (mapped_event == StateMachineEvent::UNKNOWN) {
            std::printf("[HttpServer] 工具 '%s' 触发了未定义/未映射的事件 '%s'\n", tool_id.c_str(), raw_event.c_str());
        } else {
            std::printf("[HttpServer] 收到工具 '%s' 事件 '%s'，已映射为标准事件\n", tool_id.c_str(), raw_event.c_str());
        }

        // 4. 发送转换后的事件至状态机（状态机会返回全局聚合状态）
        state_machine_.handle_event(tool_id, mapped_event);

        // 5. 响应 OK
        res.status = 200;
        res.set_content("ok", "text/plain");
    });
}

HttpServer::~HttpServer() {
    stop();
}

bool HttpServer::start(const std::string& host, int port) {
    if (is_running_) {
        return false;
    }

    host_ = host;
    port_ = port;
    is_running_ = true;

    // 创建后台工作线程启动 listen 阻塞监听
    worker_thread_ = std::make_unique<std::thread>(&HttpServer::run, this);

    std::printf("[HttpServer] 服务线程启动，正在监听 %s:%d...\n", host_.c_str(), port_);
    return true;
}

void HttpServer::stop() {
    if (!is_running_) {
        return;
    }

    svr_.stop();

    if (worker_thread_ && worker_thread_->joinable()) {
        worker_thread_->join();
    }
    worker_thread_.reset();
    is_running_ = false;

    std::printf("[HttpServer] 服务已停止并清理线程。\n");
}

bool HttpServer::is_running() const {
    return is_running_;
}

void HttpServer::run() {
    try {
        if (!svr_.listen(host_.c_str(), port_)) {
            std::printf("[HttpServer] 监听 %s:%d 失败！端口可能已被占用。\n", host_.c_str(), port_);
            is_running_ = false;
        }
    } catch (const std::exception& e) {
        std::printf("[HttpServer] 服务运行时异常: %s\n", e.what());
        is_running_ = false;
    }
}
