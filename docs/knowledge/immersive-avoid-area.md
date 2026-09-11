# 沉浸式窗口：避让区生产、查询、监听与 ArkUI 消费

## 文档定位

避让区是窗口与状态栏、导航区域、挖孔、键盘等系统界面元素相交后，需要应用主动规避的几何区域。
它与系统栏显隐相关但不是同一个状态：系统栏透明或隐藏时可能仍需观察缓存/策略更新，避让区为空也可能是有效结果。

本文覆盖 WindowManager 生产和通知链路，包括 UIExtensionComponent（下文简称 UEC）的宿主代理查询与通知转发，
并只在确认窗口链路到达时进入 ArkUI 的下游消费边界。
证据快照见 [immersive-window-overview.md](./immersive-window-overview.md)。

## 北向接口与准确名称

| 接口 | 版本/状态 | 语义 |
|---|---|---|
| `getWindowAvoidArea(type)` | API 9 起 | 获取指定类型当前有效避让区；API 23 起也提供静态接口 |
| `getWindowAvoidAreaIgnoringVisibility(type)` | API 22 起 | 查询时忽略相关系统 UI 的可见性，但不绕过全部窗口类型、模式和设备限制；不支持键盘类型 |
| `on('avoidAreaChange', callback)` | API 9 起 | 注册避让区变化监听；API 12 起回调参数统一为 `AvoidAreaOptions` |
| `off('avoidAreaChange', callback?)` | API 9 起 | 注销一个或全部对应监听；API 12 起回调参数统一为 `AvoidAreaOptions` |
| `setSystemAvoidAreaEnabled(enabled)` | API 18 起 | 允许系统窗口获取避让区；仅系统窗口有效 |
| `isSystemAvoidAreaEnabled()` | API 18 起 | 查询系统窗口的上述选项 |
| `OH_WindowManager_GetWindowAvoidArea` | C API 15 起 | 按 windowId 和类型同步获取避让区 |

容易写错的名称：

- 正确为 `getWindowAvoidAreaIgnoringVisibility`，不是 `getWindowAvvoidAreaIgnoringVisibility`；
- 正确为 `isSystemAvoidAreaEnabled`，不是 `isSystemAvoidAreaEndabled`。

更旧的异步 `getAvoidArea()`和 `systemAvoidAreaChange`事件自 API 9 废弃；新代码使用同步
`getWindowAvoidArea()`与 `avoidAreaChange`。版本以
[OpenHarmony SDK 接口声明](https://raw.githubusercontent.com/openharmony/interface_sdk-js/master/api/@ohos.window.d.ts)为准。

官方指南说明，全局悬浮窗、模态窗口和系统窗口默认不具备普通应用窗口相同的避让区获取能力；需要系统窗口能力时，
先使用 `setSystemAvoidAreaEnabled()`，并继续接受产品、模式和设备门禁。

## 数据类型

`AvoidAreaType` 位于 `interfaces/innerkits/wm/wm_common.h:1557-1566`：

- `TYPE_SYSTEM`：状态栏等系统栏区域；
- `TYPE_CUTOUT`：挖孔/刘海；
- `TYPE_SYSTEM_GESTURE`：系统手势；
- `TYPE_KEYBOARD`：输入法；
- `TYPE_NAVIGATION_INDICATOR`：底部导航指示区域；
- `TYPE_FLOAT_NAVIGATION`：悬浮导航区域。

一个 `AvoidArea`包含上下左右四个矩形。方向由相对窗口的几何位置决定；坐标转换还会受到窗口矩形、显示 ID、
旋转、缩放和产品形态影响。ArkTS `AvoidArea.visible`不应被解释为任一 SystemUI 的真实可见性。

## 当前获取规格（统一架构）

必须先区分两个问题：接口能否调用，以及 producer 能否算出非空区域。统一服务端的窗口类型/模式门禁失败时，
`GetAvoidAreaByTypeInner()`返回默认空 `AvoidArea`，客户端通常仍返回 `WM_OK`；因此“不支持该场景”经常表现为
成功取得空对象，而不是 BusinessError。当前门禁集中在
`window_scene/session/host/src/scene_session.cpp:3547-3623`、`:3876-3913`。

### 按窗口类型和模式

下表的“允许”指通过服务端 capability gate；仍需继续满足后续按类型约束。

| 查询窗口/场景 | 非键盘类型规格 | 结果与额外约束 |
|---|---|---|
| 应用主窗：非 floating、非 PC 产品 | 通常允许 | cutout 的特定分屏+旋转组合仍会被提前拒绝；PIP 的 navigation indicator 固定为空 |
| 应用主窗：Phone/Pad 普通 floating | 仅 `TYPE_SYSTEM`允许 | 非 Phone/Pad 的 floating 默认连 system 也不允许；自由多窗 floating 默认全部非键盘类型不允许 |
| 应用主窗：PC 产品 | 默认不允许 | `IsAdaptToImmersive()`兼容沉浸应用例外，所有非键盘类型可继续进入 producer |
| 应用子窗 | 有条件允许 | 内部 `ENABLE_APP_SUB_WINDOW` option 可放行；否则要求非 PC、非自由多窗 floating、存在 parent、子窗 rect 与 parent rect 完全相等，并递归继承 parent 对该 type 的资格 |
| 系统窗口 | 有条件允许 | 必须设置 `ENABLE_SYSTEM_WINDOW`，且产品为 Phone/Pad；普通查询还受 target API 18 门槛约束 |
| 既非主窗、子窗也非系统窗的普通 `SceneSession` | 非键盘类型不允许 | `TYPE_KEYBOARD`在窗口类型判断前被统一放行；UEC 不套用此行，而走宿主代理特例 |
| UEC provider 扩展窗口 | 委托宿主 | 以宿主窗口类型、模式、产品和 provider target API version 判定；计算使用宿主窗口 rect，不使用 UEC 节点 rect |

主窗逻辑、子窗继承与 option 旁路见
`window_scene/session/host/src/scene_session.cpp:3547-3586`；系统窗口 gate 见同文件 `:3588-3595`。
`ENABLE_APP_SUB_WINDOW`当前是 native `AvoidAreaOption`能力，公开 ArkTS 只暴露系统窗口的
`setSystemAvoidAreaEnabled()`，不能把子窗 option 当成普通应用公开 API，枚举见
`interfaces/innerkits/wm/wm_common.h:606-609`，公开桥见
`interfaces/kits/napi/window_runtime/window_napi/js_window.cpp:11325-11420`。

> 当前源码细节：rotation 为 `ROTATION_0`时，cutout 前置门禁检查
> `WindowMode::WINDOW_MODE_SPLIT`和 `WINDOW_MODE_SPLIT_PRIMARY`；条件中 `SPLIT_PRIMARY`重复出现，
> 没有显式列出 `SPLIT_SECONDARY`。本文记录当前实现，不将其扩展解释为设计意图，见
> `window_scene/session/host/src/scene_session.cpp:3603-3609`。

### 按避让区类型

| 类型 | 通过窗口 gate 后仍需满足 | 明确不支持/合法空值场景 |
|---|---|---|
| `TYPE_SYSTEM` | App 未因旧兼容设置 `WINDOW_FLAG_NEED_AVOID`；存在可见状态栏并与窗口相交，或满足 Phone/Pad 非自由多窗 floating/split 标题栏合成条件 | `NEED_AVOID` app 直接为空；状态栏不可见且未 ignore visibility、无状态栏、无交集时为空；HiCar 不走 floating/split 标题栏合成 |
| `TYPE_CUTOUT` | 对应 display 存在 cutout info/bounding rect，并与窗口相交 | 无 cutout 数据/区域、无交集，或命中上述 rotation-0 split gate 时为空 |
| `TYPE_SYSTEM_GESTURE` | 无 | 当前统一 producer switch 直接返回默认空区域 |
| `TYPE_KEYBOARD` | 窗口侧 keyboard avoid-area active；存在前台/活跃、非 float gravity 且自身 avoid-area active 的 IME/panel，并与窗口相交 | capability gate 不限制窗口类型/模式，但 floating 等策略可通过 `CheckEmptyKeyboardAvoidAreaIfNeeded()`强制为空 |
| `TYPE_NAVIGATION_INDICATOR` | 非 PIP；存在对应 display 的导航指示条数据、可见或使用 ignore visibility，并与窗口相交 | PIP 固定为空；无上报数据、不可见、无交集时为空；兼容沉浸应用使用 hook 值 |
| `TYPE_FLOAT_NAVIGATION` | 客户端必须先 opt-in；server 有对应 display 的横/竖屏矩形和 display，且 visible 或使用 ignore visibility，并与窗口相交 | opt-in 默认 false；未启用时 native 返回 `WM_DO_NOTHING`；无数据/display、不可见或无交集时为空 |

各 producer 的附加条件见 `window_scene/session/host/src/scene_session.cpp:3223-3308`（system）、
`:3310-3349`（keyboard）、`:3377-3411`（cutout）、`:3429-3448`（navigation indicator）和
`:3487-3510`（float navigation）。浮动导航客户端开关默认关闭，只对该 type 生效，见
`wm/include/window_session_impl.h:635-638`、`:1119-1120` 和 `wm/src/window_session_impl.cpp:7347-7370`。

### API、生命周期与返回行为

| 条件 | `getWindowAvoidArea(type)` | `getWindowAvoidAreaIgnoringVisibility(type)` |
|---|---|---|
| type 越界 | NAPI 抛参数错误 | NAPI 抛参数错误 |
| window/session 已失效 | 抛窗口状态异常 | 抛窗口状态异常 |
| 系统窗口 target API < 18 | 客户端直接 `WM_OK + empty` | 没有同一客户端短路，但仍受服务端 system option/Phone/Pad gate |
| `TYPE_KEYBOARD` | 允许查询 | 明确返回非法参数并由 NAPI 抛错 |
| float navigation 未 opt-in | native `WM_DO_NOTHING`；普通同步 NAPI 转为空对象返回 | native `WM_DO_NOTHING`，NAPI 按错误码抛出 |
| producer 不可用或没有交集 | 通常 `WM_OK + empty` | 通常 `WM_OK + empty` |

普通同步 NAPI 对 native 非 `WM_OK`会把四边置空后仍返回对象，见
`interfaces/kits/napi/window_runtime/window_napi/js_window.cpp:4434-4491`；忽略可见性接口则会抛出 native 错误，
见同文件 `:4494-4551`。客户端 API 18、float-navigation 和 keyboard 门禁见
`wm/src/window_scene_session_impl.cpp:3781-3830`。

`ignoreVisibility`不是“忽略所有约束”：当前只有 system、navigation indicator 和 float navigation producer
消费该参数；cutout 没有可见性输入，system gesture 仍为空，keyboard 在客户端被拒绝。窗口类型、模式、产品、
option、数据存在性和几何相交条件全部保留。

## 查询链路

下面先描述普通窗口。UEC 的查询不会直接进入自身对应的普通 `SceneSession` producer，而是委托宿主窗口计算，
详见“UEC 特例”一节。

```text
ArkTS getWindowAvoidArea(type)
  -> JsWindow 参数校验
  -> WindowSceneSessionImpl::GetAvoidAreaByType
  -> ISession::GetAvoidAreaByType IPC
  -> SceneSession::GetAvoidAreaByTypeInner
       -> CheckGetAvoidAreaAvailable
       -> 按类型调用具体 producer
```

客户端入口见 `wm/src/window_scene_session_impl.cpp:3781-3806`。统一服务的类型分发见
`window_scene/session/host/src/scene_session.cpp:3876-3913`：

| 类型 | 当前 producer |
|---|---|
| system | `GetSystemAvoidArea` |
| cutout | `GetCutoutAvoidArea` |
| system gesture | 当前 switch 返回空区域 |
| keyboard | `GetKeyboardAvoidArea` |
| navigation indicator | `GetAINavigationBarArea` |
| float navigation | `GetFloatNavigationAvoidArea` |

窗口类型、模式和产品门禁在 `window_scene/session/host/src/scene_session.cpp:3547-3623`。
键盘类型在该门禁中总是允许继续查询；主窗、子窗、系统窗、自由多窗、分屏和挖孔分别还有条件。
此外，target API < 18 的系统窗口在客户端直接得到成功和空区域，见
`wm/src/window_scene_session_impl.cpp:3784-3789`。

### 忽略可见性查询

`WindowSceneSessionImpl::GetAvoidAreaByTypeIgnoringVisibility()`拒绝 `TYPE_KEYBOARD`，然后仍执行浮动导航和 host session
校验，见 `wm/src/window_scene_session_impl.cpp:3809-3830`。服务端只是把 `ignoreVisibility`传给支持该参数的 producer；
它不会绕过 `CheckGetAvoidAreaAvailable()`。

因此“IgnoringVisibility”只表示不因目标系统 UI 当前不可见而丢弃几何区域，不表示无条件返回区域。

## 主要 producer

### 系统栏

`SceneSession::GetSystemAvoidArea()`位于 `window_scene/session/host/src/scene_session.cpp:3223-3308`：

- 旧应用 `WINDOW_FLAG_NEED_AVOID` 可直接短路应用窗口结果；
- 兼容模式沉浸使用 hook 路径；
- Phone/Pad 的悬浮或分屏场景可合成标题栏避让区；
- 普通路径枚举状态栏 SceneSession，结合可见性/常驻显示策略后与应用窗口求交；
- `ignoreVisibility`只影响可见性过滤。

### 键盘与挖孔

- 键盘：枚举前台/活跃输入法和键盘面板，见
  `window_scene/session/host/src/scene_session.cpp:3310-3349`；
- 挖孔：读取目标显示的 cutout bounding rect 并映射到窗口坐标，见同文件 `:3377-3409`。

## `setSystemAvoidAreaEnabled`

NAPI 仅允许系统窗口调用。实现读取 `AvoidAreaOption`，设置或清除
`ENABLE_SYSTEM_WINDOW`，再写回窗口属性，见
`interfaces/kits/napi/window_runtime/window_napi/js_window.cpp:11325-11420`。

服务端仍只在 Phone/Pad 产品形态接受该系统窗口选项，见
`window_scene/session/host/src/scene_session.cpp:3588-3595`。因此 enabled 为 true 不保证任意设备、任意类型都有非空结果。

## 刷新与通知总览（统一架构）

避让区通知不是单一的“生产者变化 -> callback”链路。从 IPC 传输看有两条数据通道；从服务端刷新粒度看，
则形成用户通常观察到的三种通知机制。

避让区通知和窗口布局通知发生在相邻帧、同一帧或不同异步队列时，优先阅读
[避让区通知与布局通知时序定位](./immersive-avoid-area-layout-timing.md)。该文档说明 RootScene VSync门禁、
dirty合并、direct/map先后、客户端handler、JS callback和ArkUI最终布局之间的边界。

| 通道 | 服务端入口 | IPC | 适用情形 |
|---|---|---|---|
| 单类型直接推送 | `SceneSession::UpdateAvoidArea(area, type)` | `SessionStage::UpdateAvoidArea` | 系统栏/键盘等避让源变化，以及导航指示条、悬浮导航的定向更新 |
| 随窗口矩形携带 | `NotifyClientToUpdateRect` -> `GetAllAvoidAreas` | `SessionStage::UpdateRect(..., avoidAreas)` | 普通窗口自身矩形/显示/旋转等变化后，需要用新窗口坐标系重算全部类型 |

| 通知机制 | 服务端计算与发送 | 典型场景 | 客户端观察 |
|---|---|---|---|
| 跟随布局的通路全部通知 | `GetAllAvoidAreas(map)`后随一次 `UpdateRect`发送 | 窗口 rect/旋转/显示等布局变化；普通窗口首次订阅或 `AVOID_AREA` dirty 后由 manager 请求刷新 | 一次 IPC 携带新 rect 和多类型 map；listener 仅对 map 中发生值变化的 type 回调 |
| 逐个全量通知 | `HandleLayoutAvoidAreaUpdate(TYPE_END)`遍历类型，每个 type 各调一次 `UpdateAvoidArea` | 锁屏状态、displayId、scale 变化；RootScene 全量刷新也采用逐类型推送 | 多次单类型 IPC；每个 type 独立去重、回调并触发一次 viewport 更新，不构成原子 map 快照 |
| 单个通知 | `HandleLayoutAvoidAreaUpdate(type)`或 manager 计算指定 type 后调一次 `UpdateAvoidArea` | 状态栏、IME、导航指示条、悬浮导航等单一 producer 变化，以及本窗口 status-bar visibility 变化 | 一次单类型 IPC；只可能产生该 type 的 listener callback |

因此“三种通知”不是三种独立协议：后两种共用 `SessionStage::UpdateAvoidArea`，只是在进入该 IPC 前是遍历
`TYPE_START..TYPE_END`还是只计算指定 type。两条通道最终都会到达
`WindowSessionImpl::UpdateViewportConfig()`和本地 `IAvoidAreaChangedListener`，但计算时机、批量粒度和
UIContent 更新次数不同。排查时必须同时确认“通道”和“刷新粒度”。

### 监听注册、首帧与注销

```text
Window.on('avoidAreaChange')
  -> JsWindowRegisterManager
  -> WindowSessionImpl::RegisterAvoidAreaChangeListener
  -> 本窗口第一个 listener 时
     WindowAdapter::UpdateSessionAvoidAreaListener(id, true)
  -> SceneSessionManagerProxy/Stub
  -> SceneSessionManager::UpdateSessionAvoidAreaListener
  -> avoidAreaListenerSessionSet_.insert(id)
  -> UpdateAvoidArea(id)               # 异步触发初始刷新
```

- NAPI 注册桥在
  `interfaces/kits/napi/window_runtime/window_napi/js_window_register_manager.cpp:141-169`。
- 客户端只在第一个 listener 加入时订阅服务端，在最后一个 listener 移除后取消订阅，见
  `wm/src/window_session_impl.cpp:7188-7239`。会话恢复时，`RecoverSessionListener()`会为仍有本地 listener
  的窗口重建服务端订阅，见同文件 `:5441-5450`。
- 服务端校验 IPC calling PID 必须与 session calling PID 一致，再更新
  `avoidAreaListenerSessionSet_`，见
  `window_scene/session_manager/src/scene_session_manager.cpp:14816-14839`。
- 注册调用本身是同步返回，但首次 `UpdateAvoidArea(id)`在服务端通过 task scheduler 异步执行，
  且后续 `SessionStage::UpdateAvoidArea` 与客户端 handler 也是异步。因此不应把 `on()`返回当成
  “初始避让区已到达 callback”的时序保证。

RootScene 不进入 `avoidAreaListenerSessionSet_`。它在每个新的、未重复的 listener 加入时会调用
`updateRootSceneAvoidAreaCallback_`，再由 `SceneSessionManager::UpdateRootSceneAvoidArea()`重算所有类型；结果通过
`RootSceneSession::UpdateAvoidArea()`的本进程 callback 扩散给 RootScene listener。见
`wm/src/root_scene.cpp:454-500`、
`window_scene/interfaces/kits/napi/scene_session_manager/js_scene_session_manager.cpp:918-955` 和
`window_scene/session/host/src/root_scene_session.cpp:223-241`。

### 触发源：什么时候会要求刷新

| 变化源 | 标记/调用 | 刷新类型与说明 |
|---|---|---|
| 状态栏或传统导航栏的矩形/显示变化 | 系统会话 `RECT`/`VISIBLE` dirty；非 SCB Core 路径直接调 `onUpdateAvoidArea_` | `IsImmersiveType()`将 status bar、navigation bar 和 IME float 视为避让源；前两者刷新 `TYPE_SYSTEM` |
| 状态栏可见性写入 app session | `SetIsStatusBarVisibleInner()` -> `HandleLayoutAvoidAreaUpdate(TYPE_SYSTEM)` | 只在值变化时计算；若上一帧布局未完成，先标记 `AVOID_AREA` dirty，延后到 FlushUI 后处理 |
| IME float 的矩形/显示/状态变化 | `KeyboardSession::CalculateOccupiedAreaAfterUIRefresh()` | 定向刷新 `TYPE_KEYBOARD`；键盘 occupied-area 是相关但独立的通知语义 |
| AI 导航指示条显隐或矩形上报 | `NotifyAINavigationBarShowStatus()` | manager 先比较缓存；仅变化时定向刷新 `TYPE_NAVIGATION_INDICATOR` |
| 悬浮导航显隐或横竖屏矩形上报 | `NotifyFloatNavigationInfo()` | manager 先比较缓存；仅变化时定向刷新 `TYPE_FLOAT_NAVIGATION` |
| 应用会话前后台/显示、矩形、旋转、scale、displayId 变化 | `MarkAvoidAreaAsDirty()`、`UpdateRotationAvoidArea()` 或 `onUpdateAvoidArea_` | 重算该窗口在新坐标系下的避让区；常走全类型/矩形携带通道 |
| 锁屏状态变化 | `UpdateAvoidAreaForLSStateChange()` | 对所有可见前台 session 执行 `TYPE_END`，即遍历刷新全部类型 |
| RootScene 矩形或状态栏常驻显示策略变化 | `SetRootSessionRect()` / `NotifyStatusBarConstantlyShow()` | 调用 `UpdateRootSceneAvoidArea()`刷新 RootScene |

SceneBoard 还会调用 `notifyNextAvoidRectInfo()`上报状态栏/导航指示条的“下一横竖屏”矩形，
例如 `window_scene_board/product/phonebase/src/main/ets/SceneBoard/statusbar/SCBStatusBar.ets:1080-1099`。
WindowManager 的 `NotifyNextAvoidRectInfo()`只写入 `nextAvoidRectInfoMap_`，本身不遍历 listener，见
`window_scene/session_manager/src/scene_session_manager.cpp:15178-15200`；该缓存在旋转预计算中由
`GetSystemBarAvoidAreaByRotation()`读取，见 `window_scene/session/host/src/scene_session.cpp:3974-4053`。
因此它是“更新后续计算输入”，不是“立即向应用发 avoidAreaChange”。相比之下，
SceneBoard 导航条调用 `notifyAINavigationBarShowStatus()`后会进入上述定向刷新，见
`window_scene_board/product/phonebase/src/main/ets/SceneBoard/naviBar/NAVIBarModel/NAVIBarViewModel.ets:575-589`。

SCB Core 的批处理入口是 `FlushUIParams()`。各 session 的 dirty flags 汇总后，
`PostProcessProperty()`遍历 session 调用 `PostProcessNotifyAvoidArea()`；
`PipelineNeedNotifyClientToUpdateAvoidArea()`在两类条件下为 true：

1. status bar/navigation bar/IME float 这些避让源发生 `VISIBLE` 或 `RECT` dirty；
2. 普通 session 带 `AVOID_AREA` dirty 且当前可见。

见 `window_scene/session/host/src/scene_session.cpp:10051-10074`、`:10141-10146`与
`window_scene/session_manager/src/scene_session_manager.cpp:16636-16669`、`:16837-16899`。隐藏 session 的
`AVOID_AREA` dirty 会在 `ResetDirtyFlags()`中保留，以便后续可见时再处理，见
`window_scene/session/host/src/session.cpp:5993-5999`。

### 服务端中央分流

`SceneSessionManager::UpdateAvoidArea(persistentId)`异步执行后按“触发者是否为避让源”分流，见
`window_scene/session_manager/src/scene_session_manager.cpp:14970-14984`：

```text
UpdateAvoidArea(changedSessionId)
  |-- changed session 是 status/navigation bar/IME float
  |     -> UpdateAvoidSessionAvoidArea(windowType)
  |     -> IME => TYPE_KEYBOARD，其余 => TYPE_SYSTEM
  |     -> 重算 RootScene
  |     -> 遍历已订阅且可见前台的 session
  |     -> 校验缩放状态，重算并单类型直接推送
  |
  `-- changed session 是普通窗口
        |-- RootScene -> 遍历 TYPE_START..TYPE_END 后直接推送
        `-- 普通 session -> 要求已订阅且可见前台
              -> SizeChangeReason::AVOID_AREA_CHANGE
              -> NotifyClientToUpdateRect("AvoidAreaChange")
              -> GetAllAvoidAreas()
              -> 随 UpdateRect 一次携带全类型 map
```

`UpdateAvoidSessionAvoidArea()`见同文件 `:14882-14903`；普通 session 的可见性与订阅过滤、
`AVOID_AREA_CHANGE` 矩形通知见 `:14905-14931`。单类型直接推送还要求
`SceneSession::GetForegroundInteractiveStatus()`，否则在 `SceneSession::UpdateAvoidArea()`拒绝，见
`window_scene/session/host/src/scene_session.cpp:4096-4105`。

导航指示条和悬浮导航不经由 `IsImmersiveType()`的 window-type 映射区分。它们由独立上报入口保存
显隐/矩形，识别值变化后遍历订阅集合，对每个可见前台 session 调用
`HandleLayoutAvoidAreaUpdate(type)`。见
`window_scene/session_manager/src/scene_session_manager.cpp:15145-15175`、`:15203-15232` 和 `:15247-15255`。

### 直接推送：单个通知与逐个全量通知

#### 单个通知

```text
SceneSession::HandleLayoutAvoidAreaUpdate(specificType)
  -> 确认上一帧布局已完成；否则 MarkAvoidAreaAsDirty()
  -> GetAvoidAreaByType(specificType)
  -> SceneSession::UpdateAvoidArea(area, specificType)
  -> SessionStageProxy::UpdateAvoidArea()        # async IPC
  -> SessionStageStub::HandleUpdateAvoidArea()
  -> WindowSessionImpl::UpdateAvoidArea()        # client handler task
  -> IsFloatNavigationAvoidAreaEnabled(specificType)
  -> UpdateLastAvoidAreaIfChanged(specificType, area)
  -> NotifyAvoidAreaChange(area, specificType)
  -> UpdateViewportConfig(currentRect, AVOID_AREA_CHANGE)
```

计算和延后逻辑见 `window_scene/session/host/src/scene_session.cpp:3026-3078`；IPC 序列化见
`window_scene/session/container/src/zidl/session_stage_proxy.cpp:761-785` 和
`window_scene/session/container/src/zidl/session_stage_stub.cpp:589-602`；客户端处理见
`wm/src/window_session_impl.cpp:7306-7344`。

去重在客户端按 `AvoidAreaType` 进行：值不变不会再通知 listener，初始未记录的空
`TYPE_CUTOUT` 也不通知。悬浮导航还有本窗口 enable 开关过滤。因此“服务端执行了刷新”
并不等于“应用必然收到一次 callback”。
直接推送仅携带一个 type，但它更新 `lastAvoidAreaMap_`后，`UpdateViewportConfig()`会将该窗口的
最新完整缓存传给 UIContent，见 `wm/src/window_session_impl.cpp:2371-2384`。

单个通知的主要入口和类型映射如下：

- status bar/navigation bar/IME float 这些避让源变化时，
  `SceneSessionManager::UpdateAvoidSessionAvoidArea()`分别映射为 `TYPE_SYSTEM`或 `TYPE_KEYBOARD`，
  再向 RootScene 和已订阅、可见前台的窗口逐个发送，见
  `window_scene/session_manager/src/scene_session_manager.cpp:14882-14902`；
- 本窗口记录的 status-bar visibility 变化只刷新 `TYPE_SYSTEM`，见
  `window_scene/session/host/src/scene_session.cpp:3014-3024`；
- AI 导航指示条和悬浮导航的上报值变化后，分别定向刷新
  `TYPE_NAVIGATION_INDICATOR`、`TYPE_FLOAT_NAVIGATION`，见
  `window_scene/session_manager/src/scene_session_manager.cpp:15145-15175`、`:15203-15255`；
- 窗口首次启用悬浮导航避让时，立即单独刷新 `TYPE_FLOAT_NAVIGATION`，见
  `window_scene/session/host/src/scene_session.cpp:7214-7227`。

#### 逐个全量通知

```text
SceneSession::HandleLayoutAvoidAreaUpdate(TYPE_END)
  -> 确认上一帧布局已完成；否则只标记 AVOID_AREA dirty
  -> for type = TYPE_START; type < TYPE_END; ++type
       -> GetAvoidAreaByType(type)
       -> navigation-indicator 几何有效性校验
       -> SceneSession::UpdateAvoidArea(area, type)
       -> 一次 SessionStage::UpdateAvoidArea async IPC
       -> 客户端按 type 去重、回调、UpdateViewportConfig
```

`TYPE_END`在这里是“遍历所有类型”的哨兵，不是可发送的避让区类型，见
`window_scene/session/host/src/scene_session.cpp:3048-3078`。它与跟随布局的 map 通路有本质差异：

- 服务端先后发出多次单类型异步 IPC，而不是一次携带多类型 map；
- 客户端每收到一个 type 都可能调用一次 listener 和一次 `UpdateViewportConfig()`；
- 每个 type 独立受到 foreground-interactive、navigation-indicator 有效性、float-navigation enable 和
  客户端值去重影响，因此“全量”表示尝试遍历，不保证出现固定数量的回调；
- 各次 UIContent 更新读取的是当时的 `lastAvoidAreaMap_`完整缓存，缓存会随逐类型到达逐步收敛，
  不应把任一次更新视为与 `UpdateRect(..., avoidAreas)`相同的原子快照。

当前明确采用逐个全量通知的场景包括：

- 锁屏状态发生变化：manager 对所有可见前台 session 调用 `TYPE_END`，见
  `window_scene/session_manager/src/scene_session_manager.cpp:15083-15105`；
- session displayId 变化：已安装 `sessionDisplayIdChangeFunc_`时，`NotifySessionDisplayIdChange()`在通知显示 ID 后
  调用默认参数为
  `TYPE_END`的 `HandleLayoutAvoidAreaUpdate()`，见
  `window_scene/session/host/src/scene_session.cpp:4583-4596`和
  `window_scene/session/host/include/scene_session.h:622`；
- 前台 session scale 变化：`UpdateScaleInner()`在 transform 下发后调用 `TYPE_END`，见
  `window_scene/session/host/src/scene_session.cpp:10077-10101`；
- RootScene 初次订阅、root rect 或常驻状态栏策略等触发 `UpdateRootSceneAvoidArea()`时，
  `UpdateRootSceneSessionAvoidArea()`自行遍历所有 type 并逐个调用 RootScene callback，见
  `window_scene/session_manager/src/scene_session_manager.cpp:14933-14947` 和
  `window_scene/session/host/src/root_scene_session.cpp:223-241`。RootScene 只有相同的“逐类型全量”粒度，
  走的是本进程 callback，不经过 `SessionStage` IPC。

### 跟随布局的通路全部通知（单次 map）

普通窗口的避让区依赖它自身矩形。服务端因此会在更新 client rect 时调用
`GetAllAvoidAreas()`，将 `TYPE_START..TYPE_END` 的 map 与 rect 在同一个异步
`SessionStage::UpdateRect` IPC 中下发：

```text
SceneSession::NotifyClientToUpdateRectTask()
  -> GetAllAvoidAreas(avoidAreas)
  -> Session::UpdateRectWithLayoutInfo(..., avoidAreas)
  -> SessionStageProxy::UpdateRect(..., avoidAreas)
  -> WindowSessionImpl::UpdateRect(..., avoidAreas)
  -> WindowSessionImpl::UpdateViewportConfig(..., avoidAreas)
       -> 逐类型更新 lastAvoidAreaMap_
       -> 仅对值变化项 NotifyAvoidAreaChange()
       -> UIContent::UpdateViewportConfig(..., avoidAreas)
```

服务端采集见 `window_scene/session/host/src/scene_session.cpp:2052-2105`、`:3940-3970`和
`window_scene/session/host/src/session.cpp:1525-1555`；IPC map 序列化/校验见
`window_scene/session/container/src/zidl/session_stage_proxy.cpp:174-236` 和
`window_scene/session/container/src/zidl/session_stage_stub.cpp:369-395`；客户端去重与扩散见
`wm/src/window_session_impl.cpp:2288-2304`。

这条路径的重要目的是让“窗口新几何”与“相对新窗口坐标计算的避让区”成组下发。
不要仅在单类型 `UpdateAvoidArea` 断点观察，否则会漏掉随 rect 到达的刷新。

它不只用于 rect 肉眼可见地变化：普通窗口第一次注册 listener 时，manager 会把该窗口设置为
`AVOID_AREA_CHANGE`并调用 `NotifyClientToUpdateRect()`，同样用这条 map 通路发送初始状态；普通窗口的
`AVOID_AREA` dirty 刷新也会经 manager 进入该路径。见
`window_scene/session_manager/src/scene_session_manager.cpp:14816-14839`、`:14905-14931`。
SCB Core 布局 flush 中，只要 session 的 rect/dirty window 确实需要同步且当前可见、可交互，
`UpdateRectInner()`也会调用 `NotifyClientToUpdateRect("WMSPipeline")`，见
`window_scene/session/host/src/scene_session.cpp:9966-9992`。

“全部通知”同样不是“必有所有 type callback”：`GetAllAvoidAreas()`虽遍历
`TYPE_START..TYPE_END`，但 scale 非法会失败，navigation-indicator 无效时会跳过该项；客户端还只对 map 中
值变化的 type 调用 listener，且 `OCCUPIED_AREA_CHANGE` reason 会跳过这里的 listener 更新。见
`window_scene/session/host/src/scene_session.cpp:3940-3971` 和
`wm/src/window_session_impl.cpp:2288-2304`。

### 客户端扩散和回调语义

`WindowSessionImpl` 在 `lastAvoidAreaMap_` 发现值变化后，将同一份 `{ type, area }`扩散给该窗口的所有
native listener。旧 `systemAvoidAreaChange`与新 `avoidAreaChange`共用此 native listener，但旧事件只回传 area，
新事件回传 `{ type, area }`；NAPI 封装见
`interfaces/kits/napi/window_runtime/window_napi/js_window_listener.cpp:195-228`。

API < 18 的系统窗口或 system-host UIExtension 在客户端回调扩散前会被隔离为空区域，见
`wm/src/window_session_impl.cpp:7269-7292`。这个隔离发生在 listener 回调层，不应误判为服务端 producer
未计算出区域。

### UEC 特例：查询委托给宿主，通知由宿主转发

这里的 UEC 指 ArkUI `UIExtensionComponent` 对应的 provider 扩展窗口。它仍向 provider 暴露
`getWindowAvoidArea(type)` 和 `on/off('avoidAreaChange')`；NAPI 查询入口见
`interfaces/kits/napi/extension_window/js_extension_window.cpp:1086-1147`，监听注册桥见
`interfaces/kits/napi/extension_window/js_extension_window_register_manager.cpp:111-130`。

#### 获取：provider 发起，宿主窗口实际计算

```text
UEC provider WindowProxy.getWindowAvoidArea(type)
  -> JsExtensionWindow::OnGetWindowAvoidArea
  -> ExtensionWindowImpl::GetAvoidAreaByType
  -> WindowExtensionSessionImpl::GetAvoidAreaByType
       -> 取 provider target API version
       -> IExtensionSession::GetAvoidAreaByType(type, rect, apiVersion)
  -> host 进程 ExtensionSession::GetAvoidAreaByType
       -> notifyGetAvoidAreaByTypeFunc_(type, apiVersion)
  -> ArkUI SessionWrapperImpl 安装的 callback
  -> 宿主 AceContainer::GetAvoidAreaByType(type, providerApiVersion)
  -> 宿主 uiWindow_->GetAvoidAreaByType(type, area, emptyRect, providerApiVersion)
  -> 宿主 WindowSceneSessionImpl / SceneSession producer
```

窗口侧两级委托见 `wm/src/extension_window_impl.cpp:36-40`、
`wm/src/window_extension_session_impl.cpp:1437-1452` 和
`window_scene/session/host/src/extension_session.cpp:509-515`；宿主 ArkUI callback 的安装与回调见
`arkui_ace_engine/frameworks/core/components_ng/pattern/ui_extension/ui_extension_component/session_wrapper_impl.cpp:513-530`、
`:601-618`，最终调用宿主窗口见
`arkui_ace_engine/adapter/ohos/entrance/ace_container.cpp:3531-3540`。

这条链有三个需要单独记住的语义：

1. `ExtensionSession::GetAvoidAreaByType()`没有使用收到的 `rect`，callback 也只携带 `type + apiVersion`；
   宿主 `AceContainer`传给宿主窗口的是空 `Rect`，服务端再以宿主 `SceneSession` 自身矩形计算，见
   `window_scene/session/host/src/scene_session.cpp:3876-3886`。因此返回的是宿主窗口原始避让区，
   该桥没有按 UEC 节点矩形再次裁剪或坐标转换。
2. 传递的是 provider 的 target API version。若宿主 `uiWindow_`是系统窗口且 provider API < 18，
   宿主 `WindowSceneSessionImpl`会成功返回空区域，见
   `wm/src/window_scene_session_impl.cpp:3781-3801`。API 门禁通过后，当前
   `SceneSession::GetAvoidAreaByType()`并未消费 `apiVersion`，仍按宿主窗口类型、模式、
   `AvoidAreaOption`和产品形态判断，见
   `window_scene/session/host/src/scene_session.cpp:3588-3623`、`:3916-3925`。
3. callback/session 缺失时该委托返回默认空区域；NAPI 对 native 非 `WM_OK` 也构造空的四边矩形后返回。
   排查“UEC 查询为空”时要先区分空值、能力门禁和链路错误，不能只看 ArkTS 对象是否成功返回。

UEC 的 ArkUI 初始化也会主动查询 system、cutout、navigation indicator 和 float navigation，建立初始
safe-area 状态，见 `arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:2882-2904` 和
`arkui_ace_engine/adapter/ohos/entrance/ace_container.cpp:3497-3517`。因此初始状态依靠同步查询；
provider 后续注册 `avoidAreaChange`本身不触发服务端首帧刷新。

#### 通知：宿主收到后跨 UEC session 转发

```text
宿主窗口避让区刷新
  -> 宿主 WindowSessionImpl 更新 lastAvoidAreaMap_
  -> 宿主 UIContentImpl::UpdateViewportConfig(..., avoidAreas)
  -> AvoidAreasUpdateOnUIExtension
  -> 宿主 PipelineContext::UpdateOriginAvoidArea
  -> UIExtensionManager::TransferOriginAvoidArea
  -> UIExtensionPattern::DispatchOriginAvoidArea
  -> SessionWrapperImpl::NotifyOriginAvoidArea
  -> host 侧 ExtensionSession::UpdateAvoidArea
  -> provider 的 SessionStage::UpdateAvoidArea
  -> provider WindowSessionImpl::UpdateAvoidArea
       -> provider lastAvoidAreaMap_ 按 type 去重
       -> provider native/ArkTS avoidAreaChange listener
       -> provider UIContent::UpdateViewportConfig
```

宿主批量入口无论 viewport 配置是否提前判等，都会把 `avoidAreas` 转给 UEC：普通 task 经
`UpdateSafeArea()`，提前返回分支则显式调用 helper，见
`arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:600-640`、`:4230-4237`、`:4252-4259`。
`PipelineContext`和 manager 会把同一份原始 `{ area, type }`广播给存活的普通 UEC 与 Security UEC，见
`arkui_ace_engine/frameworks/core/pipeline_ng/pipeline_context.cpp:2987-2992` 和
`arkui_ace_engine/frameworks/core/components_ng/pattern/ui_extension/ui_extension_manager.cpp:236-250`。
普通 UEC 的最后一段转发见
`arkui_ace_engine/frameworks/core/components_ng/pattern/ui_extension/ui_extension_component/ui_extension_pattern.cpp:1979-1983`、
`arkui_ace_engine/frameworks/core/components_ng/pattern/ui_extension/ui_extension_component/session_wrapper_impl.cpp:1358-1372` 和
`window_scene/session/host/src/extension_session.cpp:501-506`。
Security UEC 和 Preview UEC 的宿主查询 callback/下行通知采用同样结构，分别见
`arkui_ace_engine/frameworks/core/components_ng/pattern/ui_extension/security_ui_extension_component/security_session_wrapper_impl.cpp:310-317`、
`:800-806` 和
`arkui_ace_engine/frameworks/core/components_ng/pattern/ui_extension/preview_ui_extension_component/preview_session_wrapper_impl.cpp:305-312`、
`:738-744`。

宿主收到单类型直接通知时也可进入这条桥：`UIContentImpl::AvoidAreaChangedListener`对所有 type 调用
`UpdateOriginAvoidArea()`，见 `arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:663-727`。
普通应用/UIExtension 的 `InitializeSafeArea()`注册的是 no-op `PretendChangedListener`；对普通应用宿主，
该注册会建立 WindowManager 订阅，主要数据仍随 viewport map 更新；对 provider UIExtension，它只落到下述本地
listener map。SceneBoard 使用有实际处理逻辑的 listener。

与普通窗口注册不同，provider 的
`WindowExtensionSessionImpl::RegisterAvoidAreaChangeListener()`只进入
`RegisterExtensionAvoidAreaChangeListener()`，后者只维护 provider 本地 listener map，并不调用
`WindowAdapter::UpdateSessionAvoidAreaListener()`，见
`wm/src/window_extension_session_impl.cpp:1455-1462` 和 `wm/src/window_session_impl.cpp:7242-7255`。
所以不要期待在 `SceneSessionManager::avoidAreaListenerSessionSet_`里看到 UEC session，也不要从那里排查
UEC 的初始回调；真正被订阅和重算的是宿主窗口，随后由上述 ArkUI/ExtensionSession 桥转发。

provider 收到转发后复用普通客户端的按类型去重与 viewport 更新，见
`wm/src/window_session_impl.cpp:7306-7344`。若它是 system-host UEC 且 provider target API < 18，
`NotifyAvoidAreaChange()`会把公开 listener 的 payload 隔离为空，但此前保存的原始缓存和随后的
`UpdateViewportConfig()`仍继续执行；root host type 随 extension config 传入 provider，见
`wm/src/window_extension_session_impl.cpp:2252-2267` 和 `wm/src/window_session_impl.cpp:7269-7292`。
因此此兼容规则只说明公开 callback 的值，不能据此推断宿主没有生产数据或 provider UIContent 没有收到 viewport 更新。

### 按类型理解刷新覆盖

| 类型 | 主要主动触发 | 去重/特殊门禁 |
|---|---|---|
| `TYPE_SYSTEM` | status/navigation system session 的矩形或显隐、app 上的 status-bar visibility、常驻显示策略 | 订阅、前台可见和 foreground-interactive 门禁；客户端按值去重 |
| `TYPE_KEYBOARD` | IME float/panel 的矩形、显示与 state 变化 | 避让区与 `OccupiedAreaChangeInfo` 是两种数据，不能只查其中一条 |
| `TYPE_CUTOUT` | 注册首帧、窗口矩形/旋转/display 变化等全类型刷新 | 当前 manager 中没有与 AI/悬浮导航类似的独立 cutout 发布入口；初始空值在客户端去重 |
| `TYPE_NAVIGATION_INDICATOR` | `NotifyAINavigationBarShowStatus()` | manager 先对 visibility+rect 去重；还校验区域是否到达窗口底部 |
| `TYPE_FLOAT_NAVIGATION` | `NotifyFloatNavigationInfo()`、首次 enable | manager 先对 visibility+横竖屏 rect 去重；客户端还校验本窗口 enable |
| `TYPE_SYSTEM_GESTURE` | 仅会被全类型刷新遍历 | 当前统一服务 producer 返回空区域；首次批量刷新可记录/回调空值，后续未变则被客户端去重 |

## 分离架构对照

分离架构的获取规格不能套用上述 `SceneSession::CheckGetAvoidAreaAvailable()`矩阵：

- `WindowImpl::GetAvoidAreaByType()`只校验窗口有效性，然后按 windowId 请求 WMS，见
  `wm/src/window_impl.cpp:486-498`；
- `AvoidAreaController`对非 keyboard 类型仅允许 `FULLSCREEN`、`SPLIT_PRIMARY`、`SPLIT_SECONDARY`模式；
  keyboard 不受该 mode gate 限制；
- producer 仅实现 `TYPE_SYSTEM`、`TYPE_KEYBOARD`、`TYPE_CUTOUT`，其他 type 返回空；
- keyboard 还要求当前窗口是 IME calling window 或 focused window；cutout 只使用 bounding rect 列表第一项；
- `WindowImpl`没有覆盖 ignoring-visibility 接口，基类返回 `WM_ERROR_DEVICE_NOT_SUPPORT`。

见 `wmserver/src/avoid_area_controller.cpp:279-328`、`:330-370`和
`interfaces/innerkits/wm/window.h:1731-1743`。因此同一窗口在统一/分离架构下，允许的 mode、支持的 type、
cutout 合成数量及 ignore-visibility 能力都可能不同。

分离架构不使用 `SceneSessionManager`/`SessionStage`链路。`WindowImpl`在第一/最后一个 listener 时调用
`WindowAdapter::UpdateAvoidAreaListener()`，WMS 将 `WindowNode`加入/移出
`AvoidAreaController::avoidAreaListenerNodes_`。避让系统窗加入、移除或更新时，controller 重算受影响的监听窗口，
通过 `lastUpdatedAvoidArea_` 在服务端去重，然后调用 `IWindow::UpdateAvoidArea`。见
`wm/src/window_impl.cpp:2870-2889`、`wmserver/src/window_manager_service.cpp:1597-1613` 和
`wmserver/src/avoid_area_controller.cpp:30-43`、`:45-217`。
与统一架构不同，这个服务端注册函数只加入集合，没有在该处立即重算和推送初始值。

分离架构的 `WindowAgent::UpdateAvoidArea()`最终进入 `WindowImpl::UpdateAvoidArea()`，它直接同步
`UIContent::UpdateViewportConfig(..., {{type, area}})` 并扩散 listener，见
`wm/src/window_agent.cpp:72-79` 和 `wm/src/window_impl.cpp:4036-4042`。这些类、缓存和触发规则都与统一架构不同，
不能将某一边的断点或修复直接套到另一边。

## C API

`OH_WindowManager_GetWindowAvoidArea()`在 `interfaces/kits/ndk/wm/oh_window.h:101-103` 声明，
实现位于 `wm/src/oh_window.cpp:622-658`。它按 `windowId`查找窗口、校验类型和输出指针，再复用
`Window::GetAvoidAreaByType()`；错误码与 ArkTS 异常模型不同，调用方必须检查返回值。
NDK 枚举当前支持 system、cutout、system gesture、keyboard 和 navigation indicator，不含 float navigation，见
`interfaces/kits/ndk/wm/oh_window_comm.h:151-167`。

## ArkUI 下游消费边界

进入 ArkUI 源码前应先阅读其 `arkui_ace_engine/docs/kb/capabilities/safe-area.md` 与
`arkui_ace_engine/docs/kb/architecture/window-mechanism.md`。与窗口侧的两条下发通道对应，ArkUI 也有两个入口：

```text
直接 listener 入口
Rosen IAvoidAreaChangedListener
  -> UIContentImpl::AvoidAreaChangedListener
  -> UI task
  -> PipelineContext::UpdateSystemSafeArea / UpdateCutoutSafeArea / UpdateNavSafeArea
  -> SafeAreaManager

viewport 批量入口
WindowSessionImpl::UpdateViewportConfig(..., lastAvoidAreaMap_)
  -> UIContentImpl::UpdateViewportConfig(..., avoidAreas)
  -> ParseAvoidAreasUpdate / ParseAvoidAreasToMap
  -> PipelineContext::FlushSafeArea
  -> SafeAreaManager
  -> SyncSafeArea(SYNC_TYPE_AVOID_AREA)
```

直接 listener 在 `arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:663-727`处将 Rosen callback
切到 ArkUI UI 线程。它对 system、navigation indicator 和 cutout 分别更新对应 safe area，并对
所有类型调用 `UpdateOriginAvoidArea()`供 UIExtension 链路使用。不要由此推断 float navigation 不会到达
ArkUI：viewport 批量入口在同文件 `:554-579`明确处理 `TYPE_FLOAT_NAVIGATION`。

viewport 路径在 `UIContentImpl::UpdateViewportConfig()`中即使发现窗口 config 未变，也会单独执行
`ParseAvoidAreasUpdate()`，防止“矩形未变但避让区已变”被早退逻辑丢掉，见
`arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:4230-4237`。常规 viewport task 则调用
`PipelineContext::FlushSafeArea()`，见同文件 `:4312-4318` 和
`arkui_ace_engine/frameworks/core/pipeline_ng/pipeline_context.cpp:2353-2376`。`SafeAreaManager` 再对值变化判断，
只在至少一项 inset 改变时调度 `SyncSafeArea()`。

因此“ArkUI listener 断点未命中”不能单独证明避让区未刷新：还要检查
`UIContentImpl::UpdateViewportConfig()`/`ParseAvoidAreasUpdate()`这条批量通道。反之，同一次服务端变化可能经本地
去重或 `SafeAreaManager` 去重后不再触发布局。

组件的 `expandSafeArea`、`ignoreLayoutSafeArea`、`safeAreaPadding`和键盘避让模式是此状态的下游消费者，
不是 WindowManager 沉浸式北向接口本身。只有问题已经证明进入 ArkUI 布局消费阶段时才继续追踪。

## 定向断点和测试入口

按现象选断点：

1. “注册后没有首次回调”：
   `WindowSessionImpl::RegisterAvoidAreaChangeListener` ->
   `SceneSessionManager::UpdateSessionAvoidAreaListener` ->
   `SceneSessionManager::UpdateAvoidArea`。
2. “栏/键盘变了但应用没收到”：
   `PostProcessNotifyAvoidArea` -> `UpdateAvoidSessionAvoidArea` ->
   `SceneSession::UpdateAvoidArea` -> `WindowSessionImpl::UpdateAvoidArea`。
3. “窗口移动/旋转后数值不对”：
   `NotifyClientToUpdateRectTask` -> `GetAllAvoidAreas` ->
   `SessionStageProxy::UpdateRect` -> `WindowSessionImpl::UpdateViewportConfig`。
4. “回调已到但布局不变”：
   `UpdateLastAvoidAreaIfChanged` -> `UIContentImpl::UpdateViewportConfig`/
   `AvoidAreaChangedListener` -> `PipelineContext::FlushSafeArea` -> `SafeAreaManager`。
5. “UEC 查询为空”：
   `WindowExtensionSessionImpl::GetAvoidAreaByType` -> `ExtensionSession::GetAvoidAreaByType` ->
   `SessionWrapperImpl::InitNotifyGetAvoidAreaByTypeFunc` -> 宿主 `AceContainer::GetAvoidAreaByType`；
   同时核对 provider API version、宿主窗口类型和 `AvoidAreaOption`。
6. “宿主已更新但 UEC 没有回调”：
   宿主 `UIContentImpl::UpdateViewportConfig` -> `AvoidAreasUpdateOnUIExtension` ->
   `UIExtensionManager::TransferOriginAvoidArea` -> `SessionWrapperImpl::NotifyOriginAvoidArea` ->
   `ExtensionSession::UpdateAvoidArea` -> provider `WindowSessionImpl::UpdateAvoidArea`。
7. “先判断走了三种通知中的哪一种”：
   命中 `SessionStageProxy::UpdateRect`且 parcel 带 avoid-area map，是跟随布局的全部通知；命中
   `HandleLayoutAvoidAreaUpdate(TYPE_END)`后连续多次进入 `SessionStageProxy::UpdateAvoidArea`，是逐个全量通知；
   只以一个具体 type 进入 `UpdateAvoidArea`，是单个通知。

现有定向用例入口：

- 服务端订阅集合、避让源/普通 session 分流、锁屏全量刷新和导航单类型上报：
  `window_scene/test/unittest/window_immersive/scene_session_manager_immersive_test.cpp`；
- `HandleLayoutAvoidAreaUpdate(TYPE_END/specificType)`、`GetAllAvoidAreas`、dirty 与布局后刷新：
  `window_scene/test/unittest/window_immersive/scene_session_immersive_test.cpp`；
- 客户端单类型推送、float-navigation 门禁和 `lastAvoidAreaMap_` 去重：
  `wm/test/unittest/immersive/window_session_impl_immersive_test.cpp` 与
  `wm/test/unittest/immersive/window_scene_session_impl_immersive_test.cpp`；
- RootScene listener 初始刷新：`wm/test/unittest/root_scene_test.cpp`。
- UEC provider 查询委托、本地监听注册：
  `wm/test/unittest/ui_extension/window_extension_session_impl_test.cpp` 与
  `window_scene/test/unittest/ui_extension/extension_session_test.cpp`；对应 GN targets 为
  `wm_window_extension_session_impl_test`、`ws_extension_session_test`；
- ArkUI UEC 查询 callback 和宿主原始避让区广播：
  `arkui_ace_engine/test/unittest/core/pattern/ui_extension/session_wrapper_impl_test_new_ng.cpp`、
  `arkui_ace_engine/test/unittest/core/pattern/ui_extension/ui_extension_manager_test.cpp` 与
  `arkui_ace_engine/test/unittest/core/pattern/ui_extension/ui_extension_component/ui_extension_component_test.cpp`；
  对应 GN targets 为 `session_wrapper_impl_test_new_ng`、`ui_extension_manager_test`、
  `ui_extension_pattern_test_ng`。

## 常见误判

- 空避让区可能是窗口类型、模式、设备、可见性、无交集或兼容标志共同作用的正确结果。
- 通知既可以经 `SessionStage::UpdateAvoidArea`单类型下发，也可以随 `SessionStage::UpdateRect`的 map 下发；只查一条会漏判。
- 刷新请求、IPC 下发、listener callback 和 ArkUI 布局更新是四个阶段；中间的可见性、交互状态、类型 enable 和值去重都可以终止后续阶段。
- `getWindowAvoidAreaIgnoringVisibility()`只绕过可见性过滤，不绕过能力门禁。
- 状态栏、导航指示条、旧导航栏、悬浮导航、键盘和挖孔是不同 producer。
- `notifyNextAvoidRectInfo()`只更新旋转预计算输入，不是立即通知 listener 的接口。
- SystemUI 几何上报与应用设置系统栏样式是两条链路。
- RootScene/SceneBoard 自身安全区与普通应用窗口安全区不能混为一谈。
- UEC 查询计算的是宿主窗口原始避让区；UEC listener 是本地注册，变化由宿主 ArkUI 跨 extension session 转发。
- system-host UEC 的 API < 18 callback 空值隔离不等于宿主 producer、provider 缓存或 viewport 更新为空。
- 三种通知机制只对应两条 IPC：逐个全量和单个通知都使用单类型 `UpdateAvoidArea`。
- “全量”表示服务端遍历全部类型，不表示应用必定收到六次回调；类型校验、状态门禁和客户端值去重都会减量。
- 跟随布局的 map 是与新 rect 成组的一次快照；逐个全量通知会让客户端缓存逐项收敛，二者不能按相同原子性理解。
