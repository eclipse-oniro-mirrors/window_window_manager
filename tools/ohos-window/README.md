# ohos-window

## 概述

ohos-window 是 OpenHarmony 提供的窗口管理命令行工具，用于将指定最近任务中的主窗口（main window）恢复到前台。该工具遵循 Claw 规范，以 JSON 格式输出执行结果，并提供详细的错误码、错误原因和解决建议，ohos-window 的安装路径为 `/system/bin/cli_tool/executable/ohos-window`。

### 目录结构

```
tools/ohos-window/
├── BUILD.gn                              # GN 构建配置
├── ohos-window.json                          # Claw CLI 命令规范定义文件
├── include/
│   ├── shell_command.h                   # 移植的命令分发框架基类
│   └── ohos_window_command.h                 # 主头文件，定义命令选项、帮助信息和错误码
├── src/
│   ├── main.cpp                          # 入口函数，包含命令超时管理
│   ├── shell_command.cpp                 # 命令分发框架基类实现
│   └── ohos_window_command.cpp               # 核心命令实现（restore-session、help）
└── tests/
    ├── BUILD.gn                          # 单元测试构建配置
    └── ohos_window_command_util_test.cpp     # 纯逻辑与代理注入点单元测试
```

## CLI 子命令表

| 子命令 | 作用 | 可选参数 | 所需权限 |
|--------|------|----------|----------|
| `restore-session` | 将指定主窗口恢复到前台 | `--persistentId`、`--help` | `ohos.permission.CONTROL_DEVICE` |
| `--help` / `help` | 显示帮助信息 | 无 | 无 |

### restore-session 子命令参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `--persistentId <id>` | integer | 待恢复到前台的主窗口的 persistentId |
| `--help` | flag | 显示 restore-session 子命令帮助信息 |

> **注意**：`--persistentId` 为必填参数，且必须是非负整数。

## Claw 规范遵循情况

### 命令命名规范

- 工具名称采用 `ohos-<domain>` 格式：`ohos-window`
- 子命令使用小写英文，多词子命令以连字符分隔：`restore-session`
- 参数采用双连字符前缀的驼峰命名：`--persistentId`
- 命令规范元数据通过 JSON 配置文件 `ohos-window.json` 定义

### 输入格式规范

- 命令行参数通过位置参数列表（`argList_`）精确匹配并校验
- 输入参数定义在 `ohos-window.json` 的 `inputSchema` 字段中，采用 JSON Schema 规范

### 输出格式规范

所有命令执行结果均以 JSON 格式输出到标准输出，符合 `ohos-window.json` 中 `outputSchema` 的定义。

**成功响应：**

```json
{
  "type": "result",
  "status": "success",
  "data": {
    "message": "restore main window to foreground successfully."
  }
}
```

**失败响应：**

```json
{
  "type": "result",
  "status": "failed",
  "errCode": "ERR_INVALID_INPUT",
  "errMsg": "Invalid input parameters. The passed parameters are invalid.",
  "suggestion": "Check the passed parameters and ensure they are valid.\n"
}
```

### 错误码

ohos-window 定义了以下错误码，在命令执行失败时通过 JSON 输出返回：

| 错误码 | 说明 |
|--------|------|
| `ERR_INVALID_COMMAND` | 无效命令 |
| `ERR_INVALID_INPUT` | 无效的输入参数 |
| `ERR_NO_PERMISSION` | 权限校验失败 |
| `ERR_DEVICE_NOT_SUPPORT` | 设备不支持 |
| `ERR_IPC_FAILED` | IPC 通信或服务连接失败 |
| `ERR_INVALID_OPERATION` | 当前状态不允许该操作 |

## 使用示例

### 查看帮助信息

```bash
# 查看 ohos-window 总体帮助
ohos-window --help

# 查看 restore-session 子命令帮助
ohos-window restore-session --help
```

### 恢复主窗口到前台

```bash
# 恢复指定主窗口到前台
ohos-window restore-session --persistentId 100
```