/**
 * @file main.cpp
 * @brief 程序入口，组装核心组件并启动 60fps 动画渲染与事件泵循环
 */

#include <cstdio>
#include <chrono>
#include <thread>
#include <memory>
#include <filesystem>

#include "tray.h"
#include "server.h"
#include "breath_engine.h"
#include "state_machine.h"
#include "icon_renderer.h"
#include "config_manager.h"

/**
 * @brief 辅助函数：将状态转换为易读的描述字符串（展示在 Tray Hover Tooltip 中）
 */
static std::string get_state_tooltip(BreathState state) {
    switch (state) {
        case BreathState::STOPPED:
            return "STOPPED (Burgundy)";
        case BreathState::IDLE:
            return "IDLE (Mint-Green)";
        case BreathState::RUNNING:
            return "RUNNING (Lavender)";
        case BreathState::PENDING:
            return "PENDING (Terracotta)";
    }
    return "UNKNOWN";
}

int run_app();

#ifdef _WIN32
#include <windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;
    return run_app();
}
#else
int main() {
    return run_app();
}
#endif

int run_app() {
    std::printf("=== MindPulse Startup ===\n");

    // 1. 初始化配置文件目录（优先检测本地 config 目录，其次检测上一级中的 config）
    std::string config_dir = "./config";
    try {
        if (!std::filesystem::exists(config_dir)) {
            auto current = std::filesystem::current_path();
            if (std::filesystem::exists(current / "../config")) {
                config_dir = (current / "../config").string();
            }
        }
    } catch (...) {
        // 容错处理
    }
    std::printf("[Main] Using config directory: %s\n", config_dir.c_str());

    // 2. 初始化核心组件
    ConfigManager config_mgr(config_dir);
    StateMachine state_machine;
    BreathEngine breath_engine;
    IconRenderer renderer(32, 32);

    // 预热 AI 工具配置
    config_mgr.preload_tool_config("claude");
    config_mgr.preload_tool_config("codex");

    // 3. 创建并启动 HTTP 后端监听服务
    HttpServer server(config_mgr, state_machine);
    const std::string host = "127.0.0.1";
    const int port = 9876;
    if (!server.start(host, port)) {
        std::printf("[Main] Failed to start HTTP server! Exiting.\n");
        return 1;
    }

    // 4. 创建系统托盘实例
    std::unique_ptr<ITray> tray = create_platform_tray();
    if (!tray || !tray->init()) {
        std::printf("[Main] Failed to initialize system tray! Exiting.\n");
        server.stop();
        return 1;
    }

    // 5. 组装右键上下文菜单项
    bool app_running = true;

    // 标题展示项 (灰色禁点击)
    TrayMenuItem title_item;
    title_item.id = 100;
    title_item.text = "MindPulse v0.1.0";
    title_item.enabled = false;
    tray->add_menu_item(title_item);

    // 分割线效果的占位
    TrayMenuItem sep_item;
    sep_item.id = 101;
    sep_item.text = "----------------";
    sep_item.enabled = false;
    tray->add_menu_item(sep_item);

    // 退出选项
    TrayMenuItem exit_item;
    exit_item.id = 102;
    exit_item.text = "Exit";
    exit_item.enabled = true;
    exit_item.callback = [&app_running]() {
        std::printf("[Main] User triggered exit menu item, exiting...\n");
        app_running = false;
    };
    tray->add_menu_item(exit_item);

    std::printf("[Main] Tray and context menu bound successfully, entering animation frame update loop...\n");

    // 6. 进入 60fps 主更新循环
    const double target_dt = 0.016; // 约 60fps
    auto last_time = std::chrono::steady_clock::now();
    BreathState last_tooltip_state = BreathState::STOPPED;
    bool tooltip_dirty = true;
    bool icon_update_failed_logged = false;

    while (app_running) {
        // a. 处理 Windows 辅助窗口的所有排队消息 (驱动右键菜单与回调事件)
        tray->pump_messages();

        // b. 状态机时间步进，并拉取当前全局聚合状态
        BreathState agg_state = state_machine.tick(target_dt);

        // c. 将全局聚合状态推进动画引擎，平滑过渡转换
        if (breath_engine.get_current_state() != agg_state) {
            breath_engine.transition_to(agg_state);
        }

        // d. 呼适应擎 Tick 步进更新亮度、颜色与角速度偏角
        breath_engine.tick(target_dt);

        // e. 图像渲染器在缓冲区中生成对应像素数据
        auto bgra_buffer = renderer.render(
            breath_engine.get_current_brightness(),
            breath_engine.get_current_rotation_angle(),
            breath_engine.get_current_color());

        // f. 将最新像素写入托盘图标，并更新悬停提示内容
        if (!tray->update_icon(bgra_buffer, 32, 32)) {
            if (!icon_update_failed_logged) {
                std::printf("[Main] Failed to update tray icon; will keep retrying.\n");
                icon_update_failed_logged = true;
            }
        } else {
            icon_update_failed_logged = false;
        }

        BreathState tooltip_state = breath_engine.get_current_state();
        if (tooltip_dirty || tooltip_state != last_tooltip_state) {
            std::string tooltip = "MindPulse - Status: " + get_state_tooltip(tooltip_state);
            tray->set_tooltip(tooltip);
            last_tooltip_state = tooltip_state;
            tooltip_dirty = false;
        }

        // g. 进行帧率节流，防止占用过多 CPU
        auto current_time = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration<double>(current_time - last_time).count();
        if (elapsed < target_dt) {
            double sleep_sec = target_dt - elapsed;
            std::this_thread::sleep_for(std::chrono::duration<double>(sleep_sec));
        }
        last_time = std::chrono::steady_clock::now();
    }

    // 7. 清理并退出后台服务
    server.stop();
    std::printf("=== MindPulse Clean Exit ===\n");

    return 0;
}
