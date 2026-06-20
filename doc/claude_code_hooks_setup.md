# Claude Code Hooks 接入说明

本文说明如何把 **Claude Code** 的 hook 事件接入 **MindPulse**，让任务栏托盘指示灯反映 Claude Code 当前是否空闲、运行中，或正在等待用户操作。

官方文档：<https://code.claude.com/docs/en/hooks>

## 结论

推荐使用 Claude Code 原生的 `type: "http"` hook。

你当前 `settings.json` 中使用的是 `type: "command" + curl`，并手动发送：

```json
{ "event": "PermissionRequest" }
```

这种方式现在可以工作，因为 MindPulse 服务端兼容读取 `event` 字段。但从官方文档和当前源码看，它不是最合理的长期配置：

| 方式 | 结论 | 原因 |
|---|---|---|
| `type: "http"` | 推荐 | Claude Code 会自动把官方 hook JSON 作为 POST body 发送，包含 `hook_event_name`；不需要 `curl`，没有 Windows 引号转义问题 |
| `type: "command" + curl --data-binary @-` | 可作为降级方案 | 依赖外部 `curl`，但可以原样转发 Claude Code 传给 hook 的官方 JSON |
| `type: "command" + curl` 手工拼 JSON | 不推荐 | JSON 转义冗长，且你当前写法只转发事件名，丢弃了官方 payload |
| `curl` 发送 `event` | 可兼容，不推荐 | `event` 是 MindPulse 的兼容字段；官方字段是 `hook_event_name` |
| `curl` 手工发送 `hook_event_name` | 可兼容 | 比 `event` 更接近官方语义，但仍不如直接转发 stdin |

## MindPulse 如何解析事件

MindPulse 在本机启动 HTTP 服务：

```text
http://127.0.0.1:9876/hook/claude
```

当前服务端逻辑：

1. 优先读取 JSON body 中的 `hook_event_name`。
2. 如果没有 `hook_event_name`，再兼容读取 `event`。
3. 用 `config/claude.yaml` 把 Claude Code hook 事件映射为内部状态机事件。
4. 成功处理后返回 `{}`，响应类型为 `application/json`。

因此：

- 原生 HTTP hook 是最干净的方式，因为 Claude Code 会自动发送 `hook_event_name`。
- 旧的 `curl -d "{\"event\":\"...\"}"` 可以继续用于手工测试或旧配置迁移。
- 新写的命令式 hook 如果还要用 `curl`，应优先用 `--data-binary @-` 原样转发 stdin。
- 如果看到 `HTTP hook must return JSON, but got non-JSON response body: ok`，说明正在运行的 MindPulse 还是旧版本，需要重新构建并重启。

## 指示灯状态

| 指示灯状态 | 含义 | 典型来源 |
|---|---|---|
| `STOPPED` | Claude Code 会话未运行 | `SessionEnd` |
| `IDLE` | 会话存在，但当前无任务 | `SessionStart`、`Stop`、工具计数归零 |
| `RUNNING` | Claude 正在处理 prompt、展示回复或调用工具 | `UserPromptSubmit`、`PreToolUse`、`MessageDisplay` |
| `PENDING` | 正在等待用户确认或输入 | `PermissionRequest`、`Elicitation` |

`PENDING` 是阻塞态。进入 `PENDING` 后，普通活动事件不会把指示灯抢回 `RUNNING`；只有明确的恢复或结束事件才会离开该状态。

## 推荐配置：原生 HTTP Hook

把下面的 `hooks` 合并到 Claude Code 的 settings 文件中。

常见位置：

| 平台 | 文件 |
|---|---|
| Windows | `%USERPROFILE%\.claude\settings.json` |
| macOS / Linux | `~/.claude/settings.json` |

推荐只注册对状态指示有意义的事件。`InstructionsLoaded` 建议注册：它会在会话启动加载 CLAUDE.md 或规则文件时触发，可作为 `SessionStart` 在某些环境中不触发时的会话活跃兜底。`ConfigChange`、`CwdChanged`、`FileChanged`、`WorktreeCreate`、`WorktreeRemove`、`Notification`、`TeammateIdle` 在当前 MindPulse 映射里是 `NOOP`，不需要转发。

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

- 对 `PreToolUse`、`PostToolUse`、`PermissionRequest` 等工具相关事件，`matcher: ""` 表示匹配全部工具。
- 对 `UserPromptSubmit`、`PostToolBatch`、`Stop`、`TaskCreated`、`TaskCompleted`、`MessageDisplay` 等不支持 matcher 的事件，Claude Code 会忽略 matcher；保留空 matcher 只是为了配置结构一致。
- 原生 HTTP hook 会把完整官方 payload 发给 MindPulse，包括 `session_id`、`cwd`、`hook_event_name` 以及事件专属字段。MindPulse 当前只需要 `hook_event_name`。
- Claude Code 会解析 HTTP hook 的非空响应体；MindPulse 成功时返回空 JSON 对象 `{}`。

## 降级配置：Command Hook + curl

如果当前 Claude Code 版本不支持 `type: "http"`，或者你的环境必须通过命令 hook 转发，可以继续使用 `curl`。

更合理的 command hook 写法是转发 stdin，而不是手工拼接事件名。官方 command hook 会把完整 hook JSON 写入子进程 stdin，下面的命令会原样转发它：

```text
curl -sS -X POST http://127.0.0.1:9876/hook/claude -H "Content-Type: application/json" --data-binary @-
```

单个事件示例：

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

你当前的写法：

```json
{
  "type": "command",
  "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"SessionStart\\\"}\""
}
```

仍然可以运行，但只建议作为旧配置保留。后续新增事件时应优先改成 stdin 转发，或直接迁移到原生 HTTP hook。

如果必须手工构造 JSON，至少使用官方字段：

```json
{
  "type": "command",
  "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"hook_event_name\\\":\\\"SessionStart\\\"}\""
}
```

## 事件映射

MindPulse 的 Claude Code 映射在 `config/claude.yaml` 中维护，并支持运行时热重载。

| Claude Code 事件 | MindPulse 映射 | 指示灯效果 |
|---|---|---|
| `SessionStart`、`Setup` | `SESSION_START` | 进入 `IDLE`，清空工具计数 |
| `InstructionsLoaded` | `SESSION_ACTIVE` | 如果当前是 `STOPPED`，进入 `IDLE`；不会打断 `RUNNING/PENDING` |
| `SessionEnd` | `SESSION_END` | 进入 `STOPPED` |
| `SessionEnd(reason=resume/clear)` | `SESSION_START` | Claude Code 切换/清空会话但仍在交互中，重置为 `IDLE` |
| `UserPromptSubmit`、`UserPromptExpansion` | `USER_PROMPT_SUBMIT` | 进入 `RUNNING` |
| `PreToolUse` | `TOOL_START` | 进入 `RUNNING`，工具计数 +1 |
| `PostToolUse`、`PostToolUseFailure` | `TOOL_END` | 工具计数 -1；归零后进入 `IDLE` |
| `PostToolBatch` | `AGENT_RUNNING` | 进入 `RUNNING`，不改工具计数 |
| `PermissionRequest`、`Elicitation` | `PERMISSION_REQUEST` | 进入 `PENDING` |
| `PermissionDenied` | `PERMISSION_DENIED` | 进入 `RUNNING` |
| `ElicitationResult` | `AGENT_RUNNING` | 用户输入已返回，进入运行态，不改工具计数 |
| `Stop`、`StopFailure` | `AGENT_STOP` | 进入 `IDLE`，清空工具计数 |
| `SubagentStart`、`TaskCreated` | `TOOL_START` | 进入 `RUNNING`，工具计数 +1 |
| `SubagentStop`、`TaskCompleted` | `TOOL_END` | 工具计数 -1 |
| `PreCompact`、`PostCompact`、`MessageDisplay` | `AGENT_RUNNING` | 进入 `RUNNING`，不改工具计数 |
| `ConfigChange`、`TeammateIdle`、`CwdChanged`、`FileChanged`、`WorktreeCreate`、`WorktreeRemove`、`Notification` | `NOOP` | 不改变指示灯 |

## 验证步骤

1. 启动 MindPulse，确认托盘图标可见。
2. 启动 Claude Code，新会话应进入 `IDLE`。
3. 提交 prompt，应进入 `RUNNING`。
4. 触发需要用户确认的工具调用，应进入 `PENDING`。
5. 在用户未确认前，即使出现 `MessageDisplay`、`PostCompact` 等活动事件，也应保持 `PENDING`。
6. 用户处理权限请求，并收到恢复或结束类事件后，状态才应离开 `PENDING`。
7. Claude 完成本轮响应后，应回到 `IDLE`。
8. 会话结束后，应进入 `STOPPED`。

## 故障排查

如果指示灯没有变化：

- 确认 MindPulse 正在运行，并监听 `127.0.0.1:9876`。
- 确认 Claude Code 的 `settings.json` 是合法 JSON。
- 查询 MindPulse 内部状态，判断 hook 是否真的到达：

  ```powershell
  curl.exe http://127.0.0.1:9876/status/claude
  ```

  典型响应：

  ```json
  {
    "tool_id": "claude",
    "state": "IDLE",
    "aggregate_state": "IDLE",
    "active_tool_count": 0,
    "last_event": "SESSION_START"
  }
  ```

  如果 Claude Code 启动后这里仍显示 `STOPPED` 和 `UNKNOWN`，说明 `SessionStart`/`InstructionsLoaded` 没有触发到 MindPulse。优先检查 Claude Code 是否加载了正确的 `settings.json`。如果 `last_event` 是 `SESSION_END`，查看 `last_reason`：`resume` 或 `clear` 应被视为仍处于会话中，其它 reason 才表示停止。
- 如果使用 `curl`，先在终端手工执行一条请求：

  ```powershell
  curl.exe -s -X POST http://127.0.0.1:9876/hook/claude -H "Content-Type: application/json" -d "{\"hook_event_name\":\"UserPromptSubmit\"}"
  ```

- 如果使用原生 HTTP hook，优先查看 Claude Code 的 hook 调试输出，确认事件确实触发。
- 如果持续卡在 `PENDING`，检查是否缺少 `PermissionDenied`、`ElicitationResult`、`PostToolUse`、`Stop` 等恢复或结束事件。

## 参考

- Claude Code Hooks Reference: <https://code.claude.com/docs/en/hooks>
- Claude Code Settings Reference: <https://code.claude.com/docs/en/settings>
- MindPulse 设计文档: [DESIGN.md](../DESIGN.md)
