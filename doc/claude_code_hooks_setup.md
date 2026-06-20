# Claude Code Hooks 配置指南

本文说明如何把 Claude Code 的 hook 事件接入 MindPulse，使任务栏托盘指示灯能够反映 Claude Code 的真实工作状态：

| 指示灯状态 | 含义 |
|---|---|
| `STOPPED` | Claude Code 会话未运行或已退出 |
| `IDLE` | Claude Code 已启动，但当前无需用户操作，也没有正在处理的任务 |
| `RUNNING` | Claude Code 正在处理 prompt、调用工具、展示回复或执行子任务 |
| `PENDING` | Claude Code 正在等待用户确认权限或补充输入 |

官方参考：<https://code.claude.com/docs/en/hooks>

## 推荐方式

推荐使用 Claude Code 原生 HTTP hook：

```json
{ "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
```

原因：

| 方式 | 建议 | 说明 |
|---|---|---|
| `type: "http"` | 推荐 | Claude Code 会直接把官方 JSON payload POST 到 MindPulse，包含 `hook_event_name`、`session_id`、`cwd` 和事件专属字段；不需要处理 Windows 命令行转义 |
| `type: "command"` + `curl --data-binary @-` | 可用作降级方案 | 可以把 Claude Code 写入 stdin 的官方 payload 原样转发给 MindPulse |
| `type: "command"` + 手写 JSON | 不推荐 | 容易出现引号转义问题，并且通常只转发事件名，丢失官方 payload 中的上下文字段 |

MindPulse 服务端会优先读取官方字段 `hook_event_name`，并兼容旧配置中的 `event` 字段。成功处理 HTTP hook 后，MindPulse 返回空 JSON 对象 `{}`，满足 Claude Code 对 HTTP hook 响应体的要求。

## 前置条件

1. 启动 MindPulse。
2. 确认本机 HTTP 服务正在监听：

```powershell
curl.exe http://127.0.0.1:9876/status/claude
```

如果 MindPulse 正常运行，会返回类似内容：

```json
{
  "tool_id": "claude",
  "state": "STOPPED",
  "aggregate_state": "STOPPED",
  "active_tool_count": 0,
  "last_event": "UNKNOWN",
  "received_event_count": 0
}
```

## Claude Code Settings 位置

把下面的 `hooks` 配置合并到 Claude Code 的 settings 文件中。

| 平台 | 常见路径 |
|---|---|
| Windows | `%USERPROFILE%\.claude\settings.json` |
| macOS / Linux | `~/.claude/settings.json` |

修改后建议重启 Claude Code，或用 Claude Code 的 `/hooks` 命令确认配置已经被加载。

## 推荐配置

以下配置只注册 MindPulse 当前需要的事件。`ConfigChange`、`CwdChanged`、`FileChanged`、`WorktreeCreate`、`WorktreeRemove`、`Notification`、`TeammateIdle` 在默认映射中是 `NOOP`，通常不需要转发。

```json
{
  "hooks": {
    "SessionStart": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "Setup": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "SessionEnd": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "InstructionsLoaded": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "UserPromptSubmit": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "UserPromptExpansion": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "PreToolUse": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "PostToolUse": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "PostToolUseFailure": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "PostToolBatch": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "PermissionRequest": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "PermissionDenied": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "Elicitation": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "ElicitationResult": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "Stop": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "StopFailure": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "SubagentStart": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "SubagentStop": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "TaskCreated": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "TaskCompleted": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "PreCompact": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "PostCompact": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ],
    "MessageDisplay": [
      {
        "matcher": "",
        "hooks": [
          { "type": "http", "url": "http://127.0.0.1:9876/hook/claude" }
        ]
      }
    ]
  }
}
```

说明：

- `matcher: ""` 对工具类事件表示匹配全部工具。
- 对不使用 matcher 的事件，保留空 matcher 是为了让配置结构一致。
- `InstructionsLoaded` 建议保留。部分环境中 `SessionStart` 不稳定触发时，它可以作为会话活跃兜底，让指示灯从 `STOPPED` 进入 `IDLE`。
- MindPulse 只监听本机地址 `127.0.0.1:9876`，不会对外暴露 hook 接口。

## 状态映射

MindPulse 的 Claude Code 默认映射位于 `config/claude.yaml`。

| Claude Code hook 事件 | MindPulse 内部事件 | 指示灯行为 |
|---|---|---|
| `SessionStart`、`Setup` | `SESSION_START` | 进入 `IDLE`，清空活动工具计数 |
| `InstructionsLoaded` | `SESSION_ACTIVE` | 如果当前是 `STOPPED`，进入 `IDLE`；不会打断 `RUNNING` 或 `PENDING` |
| `SessionEnd` | `SESSION_END` | 进入 `STOPPED` |
| `SessionEnd` 且 `reason` 为 `resume` 或 `clear` | `SESSION_START` | Claude Code 仍处于交互中，按会话活跃处理，进入 `IDLE` |
| `UserPromptSubmit`、`UserPromptExpansion` | `USER_PROMPT_SUBMIT` | 进入 `RUNNING`，标记一轮响应开始 |
| `PreToolUse` | `TOOL_START` | 进入 `RUNNING`，活动工具计数加一 |
| `PostToolUse`、`PostToolUseFailure` | `TOOL_END` | 活动工具计数减一；本轮未结束时仍保持 `RUNNING` |
| `PostToolBatch`、`PreCompact`、`PostCompact`、`MessageDisplay` | `AGENT_RUNNING` | 表示 Claude Code 仍在本轮处理中 |
| `PermissionRequest`、`Elicitation` | `PERMISSION_REQUEST` | 进入 `PENDING`，等待用户确认或输入 |
| `PermissionDenied` | `PERMISSION_DENIED` | 回到 `RUNNING`，由 Claude Code 继续处理 |
| `ElicitationResult` | `AGENT_RUNNING` | 用户输入已返回，回到 `RUNNING` |
| `Stop`、`StopFailure` | `AGENT_STOP` | 本轮响应结束，进入 `IDLE` |
| `SubagentStart`、`TaskCreated` | `TOOL_START` | 进入 `RUNNING`，活动工具计数加一 |
| `SubagentStop`、`TaskCompleted` | `TOOL_END` | 活动工具计数减一 |
| 其它默认 `NOOP` 事件 | `NOOP` | 不改变指示灯状态 |

`PENDING` 是阻塞态。进入 `PENDING` 后，普通运行事件不会把指示灯抢回 `RUNNING`；只有明确的恢复、拒绝、工具结束、回合结束或会话结束事件才会离开该状态。这样用户可以通过任务栏快速判断是否需要操作 Claude Code。

## 验证方法

启动 MindPulse 和 Claude Code 后，可以用状态接口检查 hook 是否到达：

```powershell
curl.exe http://127.0.0.1:9876/status/claude
```

常见字段含义：

| 字段 | 含义 |
|---|---|
| `state` | `claude` 工具自身状态 |
| `aggregate_state` | 多工具聚合后的全局指示灯状态 |
| `active_tool_count` | 当前未结束的工具/子任务计数 |
| `last_event` | MindPulse 内部映射后的最后事件 |
| `last_raw_event` | Claude Code 原始 hook 事件名 |
| `last_event_field` | 本次事件来源字段，通常为 `hook_event_name` |
| `last_reason` | Claude Code payload 中的 `reason` 字段，常用于判断 `SessionEnd` 是否是真退出 |
| `received_event_count` | MindPulse 已收到的事件总数 |
| `last_error` | 最近一次 hook 解析错误 |

正常流程通常是：

1. Claude Code 启动后进入 `IDLE`。
2. 用户提交 prompt 后进入 `RUNNING`。
3. 出现权限请求或用户输入请求时进入 `PENDING`，并保持到用户处理。
4. 用户处理完成后回到 `RUNNING`。
5. 本轮响应结束后进入 `IDLE`。
6. Claude Code 退出后进入 `STOPPED`。

也可以手工发送一个测试事件：

```powershell
curl.exe -s -X POST http://127.0.0.1:9876/hook/claude -H "Content-Type: application/json" -d "{\"hook_event_name\":\"UserPromptSubmit\"}"
```

## 常见问题

### `ECONNREFUSED`

Claude Code 无法连接 `127.0.0.1:9876`。通常表示 MindPulse 没有运行、服务启动失败，或端口被其它进程占用。

处理方式：

1. 启动或重启 MindPulse。
2. 用 `curl.exe http://127.0.0.1:9876/status/claude` 检查服务。
3. 如果仍失败，查看 MindPulse 控制台日志是否有端口占用提示。

### `HTTP hook must return JSON`

Claude Code 的 HTTP hook 要求非空响应体必须是 JSON。MindPulse 当前成功响应为 `{}`。

如果看到类似错误：

```text
HTTP hook must return JSON, but got non-JSON response body: ok
```

说明正在运行的 MindPulse 还是旧版本，请重新构建并重启。

### `received_event_count` 一直是 `0`

MindPulse 没有收到 Claude Code hook。

优先检查：

1. `settings.json` 是否是有效 JSON。
2. 配置是否写入了 Claude Code 实际加载的 settings 文件。
3. Claude Code `/hooks` 页面是否能看到这些 hook。
4. MindPulse 是否正在监听 `127.0.0.1:9876`。

### 启动后仍然是 `STOPPED`

如果 `received_event_count` 为 `0`，说明启动事件没有到达 MindPulse，按上一节检查配置加载情况。

如果 `last_raw_event` 是 `SessionEnd` 且 `last_reason` 是 `resume` 或 `clear`，MindPulse 会把它视作会话仍然活跃并进入 `IDLE`。如果没有进入 `IDLE`，请确认运行的是包含该修复的新版本。

### 运行中短暂变成 `IDLE`

Claude Code 的工具调用可能出现 `PostToolUse` 后继续模型推理的情况。MindPulse 使用 `turn_active` 跟踪一轮响应，`PostToolUse` 不会直接结束本轮 `RUNNING`，只有 `Stop` 或 `StopFailure` 才表示本轮响应结束。

如果仍能稳定复现误判，请记录 `/status/claude` 中的 `last_raw_event`、`last_reason`、`active_tool_count` 和 `state`，再补充对应 Claude Code hook 事件序列。

## Command Hook 降级方案

如果当前 Claude Code 版本或运行环境不能使用 `type: "http"`，可以使用 command hook 转发 stdin：

```json
{
  "hooks": {
    "SessionStart": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -sS -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" --data-binary @-"
          }
        ]
      }
    ]
  }
}
```

这种写法会把 Claude Code 提供给 hook 的完整 JSON 原样转发给 MindPulse。相比手工构造 `{"event":"SessionStart"}`，它更接近官方 payload，也更容易保留未来新增字段。

## 相关文件

| 文件 | 作用 |
|---|---|
| `config/claude.yaml` | Claude Code hook 事件到 MindPulse 内部事件的默认映射 |
| `src/server/src/server.cpp` | HTTP hook 接收、诊断字段和 `SessionEnd(reason=resume/clear)` 特殊处理 |
| `src/core/src/state_machine.cpp` | 状态机逻辑，包括 `RUNNING`、`PENDING`、`IDLE`、`STOPPED` 的转换 |
| `tests/test_server.cpp` | HTTP hook 行为测试 |
| `tests/test_state_machine.cpp` | 状态机转换测试 |
