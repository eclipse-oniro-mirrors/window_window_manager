# 沉浸式窗口：系统栏显隐、样式与 SceneBoard 策略

## 文档定位

本文覆盖状态栏、导航栏/导航指示条的显隐、颜色、图标内容色与动画。系统栏是沉浸体验的一部分，
但与窗口模式和沉浸式布局是独立维度。布局入口与定义见
[immersive-window-overview.md](./immersive-window-overview.md)。

证据快照见总览文档；行号按 `window_window_manager@dd8e1ae076`、
`window_scene_board@7797266343` 核验。

## 北向接口

| 接口 | 版本/状态 | 作用与注意点 |
|---|---|---|
| `setWindowSystemBarEnable(names)` | API 9 起 | 数组表示要显示的栏；空数组隐藏全部三类。Promise 版本继续使用，callback 重载在 API 12 标记废弃 |
| `setSpecificSystemBarEnabled(name, enable, enableAnimation?)` | API 11 起；API 12 扩展能力 | 单独控制 `status`、`navigation` 或 `navigationIndicator` 的显隐与动画 |
| `setWindowSystemBarProperties(properties)` | API 9 起 | 部分更新背景色、内容色与动画，不设置显隐；callback 重载在 API 12 标记废弃 |
| `getWindowSystemBarProperties()` | API 12 起 | 查询客户端记录的背景色、内容色、亮图标和动画，不含显隐 |
| `setStatusBarColor(color)` | API 18 起 | 设置状态栏内容色；不要与背景色混淆 |
| `getStatusBarProperty()` | API 18 起 | 查询状态栏有效属性 |

旧 `setSystemBarEnable`、`setSystemBarProperties` 在 API 9 标记废弃，分别迁移到带 `Window` 前缀的新接口。
具体版本以[窗口 API 官方参考](https://developer.huawei.com/consumer/cn/doc/harmonyos-references/arkts-apis-window-window)为准。

`setWindowSystemBarEnable()`的名称数组不是“要隐藏的栏”：解析时先关闭 status、navigation indicator 和旧 navigation，
再按 `status`（同时打开 status 与 navigation indicator）或 `navigation`（打开旧 navigation）恢复，见
`interfaces/kits/napi/window_runtime/window_napi/js_window_utils.cpp:1166-1204`。需要精确区分三类时使用
`setSpecificSystemBarEnabled()`。

官方沉浸式指南还约束：控制系统界面元素显隐的接口主要面向非自由窗口状态下的主窗口；辅助窗口或自由窗口状态下
可能不生效，主窗口在非全屏/非最大化时设置的属性也可能在之后进入对应模式才呈现。产品问题必须结合 SceneBoard
最终策略验证，不能仅看到 API 返回成功就认为 UI 已改变。

## 核心数据结构

`SystemBarProperty`定义在 `interfaces/innerkits/wm/wm_common.h:1251-1268`，主要字段：

- `enable_`：显隐；
- `backgroundColor_`：背景色；
- `contentColor_`：文字/图标内容色；
- `enableAnimation_`：显隐动画；
- `settingFlag_`：应用是否显式设置显隐或颜色等字段。

部分更新掩码 `SystemBarPropertyFlag` 位于 `interfaces/innerkits/wm/wm_common.h:1281-1297`。
统一客户端的 owner 位于 `:1319-1324`：

1. `APPLICATION`；
2. `ARKUI_NAVIGATION`；
3. `ATOMIC_SERVICE`；
4. `ABILITY_RUNTIME`。

## 统一架构属性链路

```text
ArkTS 参数解析
  -> WindowSceneSessionImpl::UpdateSystemBarProperties
  -> UpdateSystemBarPropertyForPage
  -> SetOwnSystemBarProperty(owner = APPLICATION)
  -> GetCurrentActiveSystemBarProperty（逐字段仲裁）
  -> NotifySpecificWindowSessionProperty / UpdateProperty
  -> SceneSession::HandleSpecificSystemBarProperty
  -> SceneSession::SetSystemBarProperty
  -> JsSceneSession "systemBarPropertyChange"
  -> SCBSceneSession 保存属性和 setting flag
  -> SCBSceneSessionManager::updateSystemBarProperty
  -> 对应产品的状态栏/导航 UI 回调
```

当前 WindowManager 证据：

- NAPI `setSpecificSystemBarEnabled` 注册：
  `interfaces/kits/napi/window_runtime/window_napi/js_window.cpp:12384`；
- 部分字段合并：`wm/src/window_scene_session_impl.cpp:4129-4165`；
- 页面/application owner 写入：`wm/src/window_scene_session_impl.cpp:4168-4185`；
- owner 插入顺序：`wm/src/window_scene_session_impl.cpp:4313-4345`；
- 逐字段有效值选择：`wm/src/window_scene_session_impl.cpp:4377-4425`。

### owner 仲裁不是整结构体覆盖

`GetCurrentActiveSystemBarProperty()`分别选择背景色、内容色、显隐和动画。列表通常是最新写入优先，
但 `ABILITY_RUNTIME` 始终保留在头部。因此一个最终属性的不同字段可能来自不同 owner。

排查时应逐项记录 `SystemBarPropertyFlag`，不能因为显隐来自应用，就推断图标颜色也来自应用。

`setSpecificSystemBarEnabled()`省略 animation 参数时沿用原值，显式传入时才更新动画位，见
`interfaces/kits/napi/window_runtime/window_napi/js_window.cpp:3959-4005`。target API >= 18 时，客户端才为这一路径补
`ENABLE_SETTING`，再由 `UpdateSystemBarPropertyForPage()`派生隔离属性，见
`wm/src/window_session_impl.cpp:8751-8760`、`wm/src/window_scene_session_impl.cpp:4168-4185`。

`setWindowSystemBarProperties()`的内容色字段优先于对应的 light-icon 布尔字段，且不更新 `enable_`，见
`interfaces/kits/napi/window_runtime/window_napi/js_window_utils.cpp:1369-1486`。

## SceneBoard 产品策略

SceneBoard 的 `SCBSceneSessionManager.updateSystemBarProperty()` 是统一架构下的中央策略入口，见关联仓：

- `window_scene_board/staticcommon/basecommon/windowscene/src/main/ets/scene/session/SCBSceneSessionManager.ts:6897-6929`：
  强制属性、顶层容器会话和产品分支选择；
- 同文件 `:6937-6989`：顶层会话分类；
- 同文件 `:7209-7275`：全屏子会话；
- 同文件 `:7278-7395`：PC hover、Dock、全屏应用属性与默认色策略。

`SCBSceneSession.layoutFullScreen`还可能根据 Dock 自动隐藏或最大化全屏派生有效状态，
而 `getRealLayoutFullScreen()`保留应用真实设置，见
`window_scene_board/staticcommon/basecommon/windowscene/src/main/ets/scene/session/SCBSceneSession.ts:960-975`。

最终消费者示例：

- Phone 状态栏：`window_scene_board/product/phonebase/src/main/ets/SceneBoard/statusbar/SCBStatusBar.ets:765-804`；
- 通用状态栏：`window_scene_board/feature/commonscbscreen/src/main/ets/views/SCBGeneralStatusBar.ets:61-76`；
- Phone 导航区：`window_scene_board/product/phonebase/src/main/ets/SceneBoard/naviBar/NAVIBarModel/NAVIBarViewModel.ets:799-802`；
- PC 手势栏：`window_scene_board/product/pcbase/src/main/ets/SceneBoard/statusbar/SCBGestureBar.ets:110-136`。

状态栏在下拉、横幅等 SystemUI 临时状态中可以继续显示；这属于最终消费者策略，不代表 WindowManager 属性没有送达。

## 查询值不是最终展示值

`getWindowSystemBarProperties()`只返回 status/navigation 的背景色、内容色、亮图标和动画；
`getStatusBarProperty()`只返回状态栏 `contentColor`。两者读取客户端保存的 `SystemBarProperty`，不查询 SceneBoard
最终选中的 owner、产品默认值或 SystemUI 临时保护状态，见
`interfaces/kits/napi/window_runtime/window_napi/js_window_utils.cpp:909-932`、`:1207-1216`。
因此不能用这两个 getter 证明栏当前可见，也不能用它们证明屏幕上的最终颜色。

## C API

准确名称如下，三者均在 `interfaces/kits/ndk/wm/oh_window.h` 自 API 15 声明：

| C API | 作用 | 实现入口 |
|---|---|---|
| `OH_WindowManager_SetWindowStatusBarEnabled` | 状态栏显隐与动画 | `wm/src/oh_window.cpp:660-693` |
| `OH_WindowManager_SetWindowStatusBarColor` | 状态栏内容色，ARGB | `wm/src/oh_window.cpp:695-727` |
| `OH_WindowManager_SetWindowNavigationBarEnabled` | 导航栏显隐与动画 | `wm/src/oh_window.cpp:729-765` |

名称中没有 `OH_Window_Manager_SetWindowStatusBarColor` 这一额外下划线写法。C API 先按 `windowId` 找窗口，
再复用 Rosen `Window` 的系统栏属性能力；应沿同一 owner/SceneBoard 链路继续定位。

## 症状路由

| 症状 | 先检查 | 再检查 |
|---|---|---|
| 内容没有画到栏下 | `isImmersiveLayout` / `isIgnoreSafeArea_` | 总览和避让区文档，不要先改系统栏显隐 |
| 栏仍然可见 | 有效 `enable_` 与 `ENABLE_SETTING` | owner 仲裁、原生事件、SceneBoard 顶层会话和消费者临时保护 |
| 仅图标颜色错误 | `contentColor_` 与 `COLOR_SETTING` | owner 来源、深浅色/横屏默认策略 |
| 仅 PC/Pad 异常 | real/effective `layoutFullScreen`、自由多窗、Dock | PC 分支与 hover 回调 |
| 首帧错误后恢复 | 回调注册时机和 last-property replay | SceneBoard 默认属性选择和去重 |
