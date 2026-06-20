/**
 * @file server.h
 * @brief HTTP 服务器声明，接收客户端 Hook 并与状态机对接
 */

#pragma once

#include <string>
#include <memory>
#include <thread>
#include <atomic>
#include <httplib.h>

class ConfigManager;
class StateMachine;

/**
 * @brief 接收 Hook 的 HTTP 服务端，封装后台线程的运行与生命周期管理
 */
class HttpServer {
public:
    /**
     * @brief 构造函数
     * @param config_mgr 引用系统的配置管理器
     * @param state_machine 引用系统的状态机
     */
    HttpServer(ConfigManager& config_mgr, StateMachine& state_machine);

    /**
     * @brief 析构函数，保证服务器在销毁时被停止并 join 线程
     */
    ~HttpServer();

    /**
     * @brief 启动服务器（异步，会创建独立 worker 线程）
     * @param host 监听的网卡 IP (如 "127.0.0.1" 或 "0.0.0.0")
     * @param port 监听的端口 (如 9876)
     * @return bool 启动是否成功
     */
    bool start(const std::string& host, int port);

    /**
     * @brief 停止服务器并回收 worker 线程
     */
    void stop();

    /**
     * @brief 获取服务器当前的运行状态
     * @return bool
     */
    [[nodiscard]] bool is_running() const;

private:
    /**
     * @brief 线程体，在 worker 线程中调用 blocking 的 listen 接口
     */
    void run();

    ConfigManager& config_mgr_;                  ///< 系统配置管理器引用
    StateMachine& state_machine_;                ///< 系统状态机引用

    std::string host_;                           ///< 监听 host
    int port_;                                   ///< 监听 port

    httplib::Server svr_;                        ///< cpp-httplib 服务实例
    std::unique_ptr<std::thread> worker_thread_; ///< 工作线程指针
    std::atomic<bool> is_running_{false};        ///< 标识服务器是否正在运行
};
