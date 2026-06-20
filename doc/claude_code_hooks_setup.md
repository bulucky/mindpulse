# Claude Code Hooks 配置指南

本指南提供了将 `mindpulse` 指示灯状态机与 **Claude Code** 进行并联联调的 Hook 配置。你可以直接复制本页面的 JSON 配置，并粘贴进 Claude Code 的全局配置文件中。

---

## 1. 配置文件路径

根据你的操作系统，打开或创建以下路径的 `settings.json` 文件：

* **Windows**: `%USERPROFILE%\.claude\settings.json`  
  (例如：`C:\Users\<你的用户名>\.claude\settings.json`)
* **macOS / Linux**: `~/.claude/settings.json`

---

## 2. 方案 A：全量 Curl 配置方案 (推荐，直接复制可用)

根据 Claude Code 的官方设计，所有的 Hook 结构必须符合 **`Event -> Matcher[] -> Hook[]`** 的层级（而不是直接配字符串）。

你可以直接复制以下完整的配置块粘贴到 `settings.json` 中。该配置使用原生 `curl` 命令，不依赖外部脚本：

```json
{
  "hooks": {
    "SessionStart": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"SessionStart\\\"}\""
          }
        ]
      }
    ],
    "Setup": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"Setup\\\"}\""
          }
        ]
      }
    ],
    "SessionEnd": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"SessionEnd\\\"}\""
          }
        ]
      }
    ],
    "InstructionsLoaded": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"InstructionsLoaded\\\"}\""
          }
        ]
      }
    ],
    "ConfigChange": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"ConfigChange\\\"}\""
          }
        ]
      }
    ],
    "UserPromptSubmit": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"UserPromptSubmit\\\"}\""
          }
        ]
      }
    ],
    "UserPromptExpansion": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"UserPromptExpansion\\\"}\""
          }
        ]
      }
    ],
    "PreToolUse": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"PreToolUse\\\"}\""
          }
        ]
      }
    ],
    "PostToolUse": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"PostToolUse\\\"}\""
          }
        ]
      }
    ],
    "PostToolUseFailure": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"PostToolUseFailure\\\"}\""
          }
        ]
      }
    ],
    "PostToolBatch": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"PostToolBatch\\\"}\""
          }
        ]
      }
    ],
    "PermissionRequest": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"PermissionRequest\\\"}\""
          }
        ]
      }
    ],
    "PermissionDenied": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"PermissionDenied\\\"}\""
          }
        ]
      }
    ],
    "Elicitation": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"Elicitation\\\"}\""
          }
        ]
      }
    ],
    "ElicitationResult": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"ElicitationResult\\\"}\""
          }
        ]
      }
    ],
    "Stop": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"Stop\\\"}\""
          }
        ]
      }
    ],
    "StopFailure": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"StopFailure\\\"}\""
          }
        ]
      }
    ],
    "TeammateIdle": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"TeammateIdle\\\"}\""
          }
        ]
      }
    ],
    "SubagentStart": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"SubagentStart\\\"}\""
          }
        ]
      }
    ],
    "SubagentStop": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"SubagentStop\\\"}\""
          }
        ]
      }
    ],
    "TaskCreated": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"TaskCreated\\\"}\""
          }
        ]
      }
    ],
    "TaskCompleted": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"TaskCompleted\\\"}\""
          }
        ]
      }
    ],
    "CwdChanged": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"CwdChanged\\\"}\""
          }
        ]
      }
    ],
    "FileChanged": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"FileChanged\\\"}\""
          }
        ]
      }
    ],
    "WorktreeCreate": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"WorktreeCreate\\\"}\""
          }
        ]
      }
    ],
    "WorktreeRemove": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"WorktreeRemove\\\"}\""
          }
        ]
      }
    ],
    "PreCompact": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"PreCompact\\\"}\""
          }
        ]
      }
    ],
    "PostCompact": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"PostCompact\\\"}\""
          }
        ]
      }
    ],
    "Notification": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"Notification\\\"}\""
          }
        ]
      }
    ],
    "MessageDisplay": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "curl -s -X POST http://127.0.0.1:9876/hook/claude -H \"Content-Type: application/json\" -d \"{\\\"event\\\":\\\"MessageDisplay\\\"}\""
          }
        ]
      }
    ]
  }
}
```

---

## 3. 方案 B：Node.js 中转脚本方案 (推荐，转义更清爽)

如果在 Windows `cmd.exe` 环境下运行 `curl` 产生转义引号冲突，可使用中转脚本方案：

### A. 创建中转脚本 `~/.claude/send_hook.js`
```javascript
const http = require('http');
const event = process.argv[2];

if (!event) process.exit(0);

const data = JSON.stringify({ event: event });
const req = http.request({
  hostname: '127.0.0.1',
  port: 9876,
  path: '/hook/claude',
  method: 'POST',
  headers: {
    'Content-Type': 'application/json',
    'Content-Length': data.length
  }
}, (res) => {
  res.on('data', () => {});
});

req.on('error', () => {});
req.write(data);
req.end();
```

### B. 配置 `settings.json`
```json
{
  "hooks": {
    "SessionStart": [
      {
        "matcher": "",
        "hooks": [
          {
            "type": "command",
            "command": "node %USERPROFILE%/.claude/send_hook.js SessionStart"
          }
        ]
      }
    ]
    // ... 其他事件依此类推，同样使用 node %USERPROFILE%/.claude/send_hook.js <EventName> 包裹
  }
}
```
