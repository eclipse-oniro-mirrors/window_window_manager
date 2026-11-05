# ohos-window

## 概述

ohos-window 是 OpenHarmony 提供的窗口管理命令行工具，用于操控窗口或查询窗口信息。该工具遵循 Claw 规范，以 JSON 格式输出执行结果，并提供详细的错误码、错误原因和解决建议，ohos-window 的安装路径为 `/system/bin/cli_tool/executable/ohos-window`。

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
│   └── ohos_window_command.cpp               # 核心命令实现（restore-window、list-windows、help）
└── tests/
    ├── BUILD.gn                          # 单元测试构建配置
    └── ohos_window_command_util_test.cpp     # 纯逻辑与代理注入点单元测试
```

## CLI 子命令表

| 子命令 | 作用 | 可选参数 | 所需权限 |
|--------|------|----------|----------|
| `restore-window` | 将指定主窗口恢复到前台 | `--windowId`、`--help` | `ohos.permission.CONTROL_DEVICE` |
| `list-windows` | 查询窗口信息 | `--windowId`、`--displayId`、`--filter`、`--type`、`--help` | `ohos.permission.CONTROL_DEVICE` |
| `screenshot` | 屏幕截图（全屏/指定矩形区域/全屏排除指定窗口） | `--displayId`、`--outputPath`、`--rect`、`--excludeWindowIds`、`--windowId`、`--format`、`--help` | `ohos.permission.CAPTURE_SCREEN` |
| `--help` / `help` | 显示帮助信息 | 无 | 无 |

### restore-window 子命令参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `--windowId <id>` | integer | 待恢复到前台的主窗口的 windowId |
| `--help` | flag | 显示 restore-window 子命令帮助信息 |

> **注意**：`--windowId` 为必填参数，且必须是非负整数。

### list-windows 子命令参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `--windowId <id>` | integer | 仅查询指定 windowId 的窗口 |
| `--displayId <id>` | integer | 仅查询指定 displayId 的窗口 |
| `--filter <visible\|excludeSystem\|foreground>` | string（可多次） | 窗口过滤条件 |
| `--type <ui\|display\|layout\|meta>` | string（可多次） | 返回的窗口信息类别 |
| `--help` | flag | 显示 list-windows 子命令帮助信息 |

> **注意**：`--filter` 与 `--type` 均可多次指定，以累加过滤条件与返回信息类别。

### screenshot 子命令参数说明

| 参数 | 类型 | 说明 |
|------|------|------|
| `--displayId <id>` | integer | 待截图的 displayId（缺省为默认屏） |
| `--outputPath <path>` | string | 输出文件路径（缺省自动生成带时间戳文件名，位于 `/data/local/tmp` 下）；指定时写入任意有权限访问的路径 |
| `--rect <x,y,width,height>` | string | 仅截取屏幕指定矩形区域 |
| `--excludeWindowIds <id1,id2,...>` | string（逗号分隔） | 全屏截图时排除指定 windowId（windowId 语义），仅支持全屏，不可与 `--rect` 组合 |
| `--windowId <id>` | integer | 截图窗口的ID，不能与`--displayId`/`--rect`/`--excludeWindowIds`一起使用 |
| `--format <jpeg\|png>` | string | 图片格式，支持jpeg和png，默认jpeg |
| `--help` | flag | 显示 screenshot 子命令帮助信息 |

> **注意**：
> - 该子命令的能力对齐系统能力 `@ohos.screenshot`：全屏截图、区域截图（`screenRect`）、排除窗口截图（`blackWindowIds`）。
> - 不带任何参数时默认全屏截图，走老接口 `GetScreenshot`（服务端 foundation 内置实现），**所有设备类型（phone/PC 等）均支持**。
> - `--rect` 区域截图走 `GetScreenshotwithConfig`（老接口 `GetScreenshot` + 本地裁剪），**所有设备类型均支持**。
> - `--excludeWindowIds` 走 `GetScreenCapture`（blackWindowIdList_，windowId 语义），其服务端实现仅存在于部分设备类型（如 phone）的镜像中，**在 PC 等设备上会返回失败**。
> - `--excludeWindowIds` 与 `--rect` 互斥。
> - 不指定 `--outputPath` 时默认保存到 `/data/local/tmp/screenshot_<时间戳>.jpeg`（走 `SnapShotUtils` 的默认保存逻辑）；指定 `--outputPath` 时允许写入任意有权限访问的路径（走 `ImagePacker` 直接打包，不再受 `/data/local/tmp` 限制），目标目录需已存在且进程对该路径有写权限。

## Claw 规范遵循情况

### 命令命名规范

- 工具名称采用 `ohos-<domain>` 格式：`ohos-window`
- 子命令使用小写英文，多词子命令以连字符分隔：`restore-window`
- 参数采用双连字符前缀的驼峰命名：`--windowId`
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

# 查看 restore-window 子命令帮助
ohos-window restore-window --help
```

### 恢复主窗口到前台

```bash
# 恢复指定主窗口到前台
ohos-window restore-window --windowId 100
```

### 查询窗口信息

```bash
# 查询所有可见窗口
ohos-window list-windows --filter visible

# 查询指定窗口的元信息
ohos-window list-windows --type meta --windowId 100

# 查询指定显示设备上的窗口布局与元信息
ohos-window list-windows --displayId 0 --type layout --type meta
```

**list-windows 成功响应示例：**

```json
{
  "type": "result",
  "status": "success",
  "data": {
    "windows": [
      {
        "displayInfo": {
          "displayId": 0,
          "displayName": "mainDisplay"
        },
        "metaInfo": {
          "windowId": 100,
          "windowName": "mainWindow",
          "bundleName": "com.example.app",
          "windowType": 1,
          "isFocused": true
        }
      }
    ]
  }
}
```