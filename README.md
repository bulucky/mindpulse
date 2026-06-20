# MindPulse

> Windows 任务栏里的呼吸灯 — 一眼看清 AI 编程助手是空闲、运行中，还是在等你的输入。

MindPulse 是一个 **Windows 系统托盘指示器**，以呼吸灯动画图标实时反映 [Claude Code](https://claude.ai/code)、[Codex](https://github.com/openai/codex) 等 AI 编程工具的工作状态。任务栏图标以颜色、亮度与呼吸节奏编码状态信息 — 无需切换窗口，余光即可感知。

---

## 四种状态

| 状态 | 颜色 | 动画 | 含义 |
|:---|:---|:---|:---|
| **静息** Stopped | <sub>⬤</sub> 暗酒红 `#792740` | 静态微光，无旋转 | 无 AI 工具运行 |
| **就绪** Idle | <sub>⬤</sub> 薄荷绿 `#69ad9b` | 深慢呼吸 4.0s 周期，缓慢旋转 | 工具空闲，等待指令 |
| **处理中** Running | <sub>⬤</sub> 薰衣草紫 `#61649f` | 活跃呼吸 1.8s 周期，快速旋转 | Agent 正在执行任务 |
| **等待中** Pending | <sub>⬤</sub> 陶土橙 `#e59e67` | 三次脉冲 + 暂停，3.0s 周期 | 需要用户输入或批准 |

**多工具优先级**: Pending > Running > Idle > Stopped（同时追踪多个 AI 工具时，取最高优先级状态）

---

## 架构

```
Claude Code ──POST──┐
                    ├── HttpServer (:9876) ── ConfigManager ── StateMachine
Codex       ──POST──┘                                          │
                                                    优先级聚合 + 嵌套计数
                                                               │
Windows 任务栏 ◀── TrayWin ◀── IconRenderer ◀── BreathEngine ◀┘
                  Shell_         32×32 BGRA       动画曲线
                NotifyIcon       金比例圆环      亮度/颜色/旋转
```

| 模块 | 库 | 职责 |
|:---|:---|:---|
| **core** | `libcore.a` | 呼吸动画、状态机、YAML 配置、图标渲染 — 无平台依赖 |
| **server** | `libserver.a` | HTTP 服务，POST `/hook/<tool_id>` 接收事件 |
| **ui** | `libui.a` | Win32 系统托盘 (Shell_NotifyIcon)；非 Windows 降级为 stdout |

---

## 特性

- **余光感知** — 状态通过颜色节奏直接传达，无需看终端
- **多工具共存** — 嵌套计数 + 优先级聚合，同时追踪 Claude Code 与 Codex
- **过程化渲染** — 32×32 图标由数学公式生成，三缺口金比例旋转圆环，零图片依赖
- **零运行时依赖** — C++17 → 单个 `mindpulse.exe`，仅链接 Windows 系统 DLL
- **配置热重载** — 修改 YAML 即时生效，文件缺失自动生成默认配置
- **本地 HTTP** — `127.0.0.1:9876`，兼容 Claude Code 与 Codex 钩子格式

---

## 快速开始

### 1. 构建

```bash
git clone https://github.com/bulucky/mindpulse.git
cd mindpulse
cmake -B build -G Ninja
cmake --build build
```

> 需要 CMake ≥ 3.20、C++17 编译器、Ninja。依赖项（httplib、nlohmann/json、yaml-cpp）通过 FetchContent 自动下载。

或从 [Releases](https://github.com/bulucky/mindpulse/releases) 下载预编译 `mindpulse.exe`。

### 2. 启动

双击 `mindpulse.exe` — 任务栏出现暗酒红图标，HTTP 服务开始监听。

### 3. 配置 AI 工具钩子

**Claude Code** — 将以下配置加入 `.claude/settings.json`:

```json
{
  "hooks": {
    "PostToolUse": [
      {
        "type": "http",
        "url": "http://127.0.0.1:9876/hook/claude",
        "format": "json",
        "body": {
          "hook_event_name": "PostToolUse"
        }
      }
    ]
  }
}
```

事件映射定义在 `config/claude.yaml`:

```yaml
claude:
  name: "Claude Code"
  events:
    SessionStart:       "SESSION_START"
    PreToolUse:         "TOOL_START"
    PostToolUse:        "TOOL_END"
    PermissionRequest:  "PERMISSION_REQUEST"
    Stop:               "AGENT_STOP"
    SessionEnd:         "SESSION_END"
```

可修改此文件调整映射或添加新工具（文件缺失时自动生成默认配置）。

---

## HTTP API

所有端点位于 `127.0.0.1:9876`:

| 方法 | 路由 | 说明 |
|:---|:---|:---|
| POST | `/hook/<tool_id>` | 接收钩子事件（`hook_event_name` 或 `event` 字段） |
| GET | `/status/<tool_id>` | 查询单工具状态与诊断 |
| GET | `/status` | 查询聚合状态 |

POST 响应为 `{}`（空 JSON 对象），满足 Claude Code 对 HTTP 钩子返回 JSON 的要求。

---

## 技术栈

| 技术 | 用途 |
|:---|:---|
| C++17 | 主语言 |
| CMake + Ninja | 构建系统 |
| cpp-httplib | HTTP 服务（FetchContent，头文件库） |
| nlohmann/json | JSON 解析（FetchContent，头文件库） |
| yaml-cpp | YAML 配置解析（FetchContent） |
| Win32 API | 系统托盘（Shell_NotifyIcon, CreateDIBSection） |

---

## 许可

[MIT](LICENSE) · bulucky · 2026

---

<p align="center">
  <sub>状态可见，注意力不可分割。</sub>
</p>
