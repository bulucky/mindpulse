/**
 * @file server.cpp
 * @brief HTTP 服务器实现，分发并映射接收到的 Hooks 事件
 */
#include "server.h"

#include <cstdio>

#include <nlohmann/json.hpp>

#include "config_manager.h"
#include "state_machine.h"

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

static const char* event_to_string(StateMachineEvent event) {
    switch (event) {
        case StateMachineEvent::SESSION_START:
            return "SESSION_START";
        case StateMachineEvent::SESSION_ACTIVE:
            return "SESSION_ACTIVE";
        case StateMachineEvent::SESSION_END:
            return "SESSION_END";
        case StateMachineEvent::USER_PROMPT_SUBMIT:
            return "USER_PROMPT_SUBMIT";
        case StateMachineEvent::AGENT_RUNNING:
            return "AGENT_RUNNING";
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
        case StateMachineEvent::NOOP:
            return "NOOP";
        case StateMachineEvent::UNKNOWN:
            return "UNKNOWN";
    }
    return "UNKNOWN";
}

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
            std::printf("[HttpServer] failed to parse JSON payload from tool '%s': %s\n", tool_id.c_str(), e.what());
            {
                std::lock_guard<std::mutex> lock(diagnostics_mutex_);
                HookDiagnostics& diagnostics = diagnostics_[tool_id];
                diagnostics.last_error = "Invalid JSON body";
            }
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
            std::printf("[HttpServer] missing required string field 'hook_event_name' from tool '%s'\n", tool_id.c_str());
            {
                std::lock_guard<std::mutex> lock(diagnostics_mutex_);
                HookDiagnostics& diagnostics = diagnostics_[tool_id];
                diagnostics.last_error = "Missing hook_event_name";
            }
            res.status = 400;
            res.set_content("Missing or invalid 'hook_event_name' parameter", "text/plain");
            return;
        }

        std::string raw_event = body[event_field].get<std::string>();
        std::string reason;
        if (body.contains("reason") && body["reason"].is_string()) {
            reason = body["reason"].get<std::string>();
        }
        {
            std::lock_guard<std::mutex> lock(diagnostics_mutex_);
            HookDiagnostics& diagnostics = diagnostics_[tool_id];
            diagnostics.received_event_count++;
            diagnostics.last_raw_event = raw_event;
            diagnostics.last_event_field = event_field;
            diagnostics.last_reason = reason;
            diagnostics.last_error.clear();
        }

        // 3. 通过 ConfigManager 将原始 Hook 事件转换为标准化状态机事件
        StateMachineEvent mapped_event = config_mgr_.get_event_mapping(tool_id, raw_event);
        if (raw_event == "SessionEnd" && (reason == "resume" || reason == "clear")) {
            mapped_event = StateMachineEvent::SESSION_START;
        }

        if (mapped_event == StateMachineEvent::UNKNOWN) {
            std::printf("[HttpServer] tool '%s' sent unknown or unmapped event '%s'\n", tool_id.c_str(), raw_event.c_str());
        } else {
            std::printf("[HttpServer] received event '%s' from tool '%s' and mapped it to %s\n",
                        raw_event.c_str(),
                        tool_id.c_str(),
                        event_to_string(mapped_event));
        }

        // 4. 发送转换后的事件至状态机（状态机会返回全局聚合状态）
        state_machine_.handle_event(tool_id, mapped_event);

        // 5. Claude Code HTTP hooks 会解析非空响应体，成功时返回空 JSON 对象。
        res.status = 200;
        res.set_content("{}", "application/json");
    });

    svr_.Get(R"(/status/([a-zA-Z0-9_\-]+))", [this](const httplib::Request& req, httplib::Response& res) {
        std::string tool_id = req.matches[1].str();
        BreathState state = state_machine_.get_tool_state(tool_id);
        StateMachineEvent last_event = state_machine_.get_tool_last_event(tool_id);

        nlohmann::json body = {
            {"tool_id", tool_id},
            {"state", breath_state_to_string(state)},
            {"active_tool_count", state_machine_.get_tool_active_count(tool_id)},
            {"last_event", event_to_string(last_event)},
            {"aggregate_state", breath_state_to_string(state_machine_.get_aggregate_state())}};
        {
            std::lock_guard<std::mutex> lock(diagnostics_mutex_);
            auto it = diagnostics_.find(tool_id);
            if (it != diagnostics_.end()) {
                body["received_event_count"] = it->second.received_event_count;
                body["last_raw_event"] = it->second.last_raw_event;
                body["last_event_field"] = it->second.last_event_field;
                body["last_reason"] = it->second.last_reason;
                body["last_error"] = it->second.last_error;
            } else {
                body["received_event_count"] = 0;
                body["last_raw_event"] = "";
                body["last_event_field"] = "";
                body["last_reason"] = "";
                body["last_error"] = "";
            }
        }

        res.status = 200;
        res.set_content(body.dump(), "application/json");
    });

    svr_.Get("/status", [this](const httplib::Request&, httplib::Response& res) {
        nlohmann::json body = {
            {"aggregate_state", breath_state_to_string(state_machine_.get_aggregate_state())}};

        res.status = 200;
        res.set_content(body.dump(), "application/json");
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

    std::printf("[HttpServer] service thread started; listening on %s:%d...\n", host_.c_str(), port_);
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

    std::printf("[HttpServer] service stopped and worker thread cleaned up.\n");
}

bool HttpServer::is_running() const {
    return is_running_;
}

void HttpServer::run() {
    try {
        if (!svr_.listen(host_.c_str(), port_)) {
            std::printf("[HttpServer] failed to listen on %s:%d; the port may already be in use.\n", host_.c_str(), port_);
            is_running_ = false;
        }
    } catch (const std::exception& e) {
        std::printf("[HttpServer] runtime exception: %s\n", e.what());
        is_running_ = false;
    }
}
