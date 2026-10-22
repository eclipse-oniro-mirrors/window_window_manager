# 应用拉应用动效：拉起链路、启动窗与转场动画

## 文档定位

本文专项说明统一架构（`window_scene/`）下“应用拉应用”场景的窗口动效全链路。应用A处于前台，通过 `startAbility` 拉起应用B，典型表现是：B 显示启动窗（冷启动）或直接转场（热启动）、A 退场、B 打开到位。

本文回答：

- Ability Manager Service 把拉起请求送到 WMS 的哪个 IPC 入口，caller（谁拉起谁）记录在哪里；
- 冷启动与热启动在哪个函数分流，`StartUIAbilityBySCB` 的 sceneFlag 是什么；
- 统一架构下 open/close 转场动画由谁执行、动画参数是什么、作用于什么节点；
- 启动窗的类型决策（start_window.json / RDB / preload）与移除链路，退出动画开关在哪里；
- 首帧 ready（`bufferAvailableChange`）与启动窗移除、焦点推进、动画完成的衔接顺序；
- 如何用现有 Hilog、HiTrace、performanceMonitor 重建一次拉起的真实顺序。

本文只覆盖统一架构；分离架构（`wmserver/` 的 RemoteAnimation/StartingWindow）不在本文范围。沉浸式布局的窗口状态机见
[immersive-window-overview.md](./immersive-window-overview.md)，避让区与 rect 通知时序见
[immersive-avoid-area-layout-timing.md](./immersive-avoid-area-layout-timing.md)。

## 证据快照

本文按以下工作区快照核验，仓库更新后应重新搜索符号并刷新行号：

| 仓库 | 分支 | 提交 | 提交日期 |
|---|---|---|---|
| `window_window_manager` | `master` | `3e2a7b0be3ed` | 2026-09-11 |
| `window_scene_board` | `master` | `9e9e76004db6` | 2026-09-10 |

启动窗 UI 的实际绘制与退出动画执行体在 `arkui_ace_engine`（`WindowScene` ArkUI 组件的 native 实现），该仓未在本地核验，
本文对该侧只描述边界，不给出未经验证的行号。

## 结论先行

1. 统一架构下 WMS 不执行 open/close 转场动画，也不组装远程动画 target。`SetWindowAnimationController`、
   `NotifyWindowTransition`、`GetWindowAnimationTargets` 在 `SceneSessionManagerStub` 基类中是 no-op 默认实现，见
   `window_scene/session_manager/include/zidl/scene_session_manager_interface.h:369-391`。应用拉应用的转场动画由同进程的
   SceneBoard ArkTS（`window_scene_board` 仓）执行，WMS 只负责事件与状态机。
2. 拉起的 IPC 入口是 `ISession::TRANS_ID_ACTIVE_PENDING_SESSION`（Ability Manager Service → root session 或前台A的 session），不是
   `ISceneSessionManager` 的 manager 接口。热启动/mission 前台化才走
   `ISceneSessionManagerLite::PendingSessionToForeground`，见 `window_scene/session_manager/src/scene_session_manager.cpp:14682-14708`。
3. 冷/热启动分流在 `RequestSceneSessionActivationInner`：session 已存在且已连接（典型：A 拉起后台的B）时直接
   `NotifySessionForeground(1, true)`（1 是 `KEYGUARD` 枚举值，非 `ABILITY_CALL`），否则经 `StartUIAbilityBySCB` 让 Ability Manager Service
   真正拉起进程，sceneFlag 恒为 `WindowStateChangeReason::ABILITY_CALL`，带 XCollie 5 秒超时监控，见
   `window_scene/session_manager/src/scene_session_manager.cpp:4368-4381`、`:4211-4241`。
4. 应用拉应用的默认动画是 `SCBScenePanelAnimator.sceneContainerAppTransitionChange`：B 从右缘水平推入（600ms
   `Curve.Friction`）、A 向左滑出；两容器先 200ms 缩放到 0.9，再 400ms（delay 200）回弹到 1.0。同 bundle 且应用通过
   `startAnimationOptions` 指定 `FADE_IN_OUT` 时改走 300ms 淡入淡出。动画作用于 ArkUI 容器属性
   （`needRenderTranslate`/`needRenderScale`/`needRenderAlpha`），不是 RS leash、也不是 SurfaceNode bounds，见
   `window_scene_board/product/phonebase/src/main/ets/SceneBoard/scenemanager/viewanimator/SCBScenePanelAnimator.ets:219-385`。
5. A 的退后台请求先于动画发出：`sceneContainerAppTransitionChange` 开头即调用
   `requestFromContainerBackgroundOrDestruction`（`:243-244`），WMS 侧 `BackgroundTask` 存快照。所以日志时间序上
   A 的 `requestSceneSessionBackground` 早于动画 finish，属预期。
6. 启动窗的“数据决策”在本仓（startWindowType、启动页资源四级查找与缓存、preload pixelMap、快照），"UI 绘制与退出动画执行”
   不在本仓也不在 `window_scene_board` ArkTS 代码中，而在 `arkui_ace_engine` 的 `WindowScene` 组件实现内。本仓移除链路止于
   `Session::RemoveStartingWindow` → `ILifecycleListener::OnAppRemoveStartingWindow()`，见
   `window_scene/session/host/src/session.cpp:2081-2091`；该接口在本仓与 `window_scene_board` 均无生产实现（仅测试桩）。
7. 首帧信号有两条：应用主动 `Window::NotifyRemoveStartingWindow()`（要求模块配置 `enable.remove.starting.window`），以及
   WMS→SCB 的 `bufferAvailableChange(isBufferAvailable, startWindowInvisible)` 事件；后者第二参数为 true 时 SCB 侧仅做
   “应用内容已加载”通知（`notifyApplicationLoadedWhenStartWindowInvisible`），不代表启动窗节点已被移除。
8. `RETAIN_AND_INVISIBLE` 启动窗策略会把 `SCB_START_APP` 原因的焦点请求推迟到窗口可见（`IsDelayFocusChange()` =
   `GetHidingStartingWindow()`），见 `window_scene/session_manager/src/scene_session_manager.cpp:16860-16863`。

## 职责边界：谁执行应用拉应用动画

| 层 | 职责 | 稳定入口 |
|---|---|---|
| Ability Manager Service（ability_runtime，外部） | startAbility 调度、进程拉起、经 `ISession`/`ISceneSessionManagerLite` 通知 WMS | `SessionProxy::PendingSessionActivation`（`window_scene/session/host/src/zidl/session_proxy.cpp:564`） |
| WMS native（本仓 `window_scene/`） | session 创建与状态机、启动窗数据决策与缓存、快照存取、移除通知、焦点后处理 | `SceneSessionManager`、`Session`/`SceneSession` |
| SCB ArkTS（`window_scene_board`） | 转场动画编排与执行、事件消费、动画完成后状态推进、按需回调 WMS NAPI | `SCBScenePanelViewModel`/`SCBScenePanelAnimator` |
| ace_engine `WindowScene` 组件（未本地核验） | 应用 surface 宿主、启动窗 UI 绘制、`ILifecycleListener` 实现（含 `OnAppRemoveStartingWindow` 消费与退出动画） | 关联仓 `arkui_ace_engine` |

判断“动画到底谁执行”的直接证据：no-op 接口（`scene_session_manager_interface.h:369-391`）说明统一架构没有远程动画 IPC；
`window_scene_board` 全仓检索启动窗绘制（illustration/branding/REQUIRED_SHOW 等）零命中，仅有 NAPI 消费面
（`setStartingWindowExitAnimationFlag`、`supportPreloadStartingWindow` 等）；本仓 `RSSurfaceNodeType::STARTING_WINDOW_NODE`
只存在于 `wmserver/`（分离架构）。`libscene_session.map` 导出 `SceneSession`/`ScenePersistence` 符号供进程内其它库链接，
是 ace_engine 在同进程直接消费 Session 能力的佐证，见 `window_scene/session/libscene_session.map`。

## 全链路时序（冷启动）

```text
App A --startAbility--> Ability Manager Service
Ability Manager Service --TRANS_ID_ACTIVE_PENDING_SESSION(TF_ASYNC)--> root/A 的 SceneSession
  -> SceneSession::PendingSessionActivation            (scene_session.cpp:6899)
     startMethod = START_CALL; MakeSessionInfoDuringPendingActivation (6726, caller字段)
     CalculateStartWindowType (6840, 仅PC同bundle)
  -> pendingSessionActivationFunc_ -> JsRootSceneSession::PendingSessionActivation
     (js_root_scene_session.cpp:413; GenSceneSession -> RequestSceneSession -> MainSession)
  -> JS 事件 'pendingSceneSessionActivation'
SCB ArkTS 消费
  -> SCBRootSceneSession/SCBSceneSession::onPendingSceneSessionActivation
     (SCBRootSceneSession.ts:116 / SCBSceneSession.ts:2444)
  -> caller 是普通应用 session -> startSceneTransition(to, from)   (SCBSceneSession.ts:2496)
  -> SCBScenePanelViewModel.onSceneStartTransition -> sceneStartTransitionIfNeed
     (SCBScenePanelViewModel.ets:6008: 新建/复用 toContainerSession, requestActivation, raiseSceneToTop)
  -> requestSceneSessionActivation NAPI 回调 WMS       (SCBSceneSessionManager.ts:4791)
WMS 激活分流
  -> SceneSessionManager::RequestSceneSessionActivation (SSM.cpp:4105, hitrace ssm:RequestSceneSessionActivation)
  -> RequestSceneSessionActivationInner (4280)
     冷启动: StartUIAbilityBySCBTimeoutCheck -> Ability Manager Service StartUIAbilityBySCB(sceneFlag=ABILITY_CALL) (4360/4376)
     热启动(session已连接): NotifySessionForeground(1, true) 直连 App (4379-4380)
App B 进程
  -> CreateAndConnectSpecificSession -> ConnectInner -> STATE_CONNECT
  -> WindowScene::GoForeground -> mainWindow->Show -> TRANS_ID_FOREGROUND
  -> SceneSession::Foreground -> ForegroundTask -> Session::Foreground -> STATE_FOREGROUND (session.cpp:1855-1888)
  -> sessionStateChange 事件 -> SCB OnSessionStateChange(STATE_FOREGROUND)
转场动画（与上一步并行）
  -> sceneContainerAppTransitionChange(A容器, B容器)
     先 requestFromContainerBackgroundOrDestruction(A) —— A 退后台+存快照
     600ms Friction 水平推入 + 200ms scale 0.9 + 400ms(delay 200) 回弹
     (SCBScenePanelAnimator.ets:219-385)
App B 首帧
  -> [配置了 enable.remove.starting.window 的应用] Window::NotifyRemoveStartingWindow
     -> TRANS_ID_APP_REMOVE_STARTING_WINDOW -> Session::RemoveStartingWindow
     -> ILifecycleListener::OnAppRemoveStartingWindow (ace_engine 消费, 止于本仓边界)
  -> WMS Session::SetBufferAvailable -> JS 'bufferAvailableChange'(true, startWindowInvisible)
     -> SCB onBufferAvailableChange (SCBSceneSession.ts:3416)
动画完成
  -> finishCallback: transitionOutFinish / 移除from容器 / performanceMonitor.end
焦点推进
  -> WMS focus post-process: reason==SCB_START_APP 且 IsDelayFocusChange() 则延迟 (SSM.cpp:16860)
```

热启动差异：B 的 session 已存在且连接时，`RequestSceneSessionActivationInner` 走 `NotifySessionForeground(1, true)`
直接通知 App B `window->Show(reason, withAnimation)`（`wm/src/window_scene_session_impl.cpp:7486-7501`），无
`StartUIAbilityBySCB`；另有 mission 前台化路径由 Ability Manager Service 调 `PendingSessionToForeground`（lite stub），WMS 把事件投给当前聚焦
主窗口 session 或目标 session 的 JS 监听（`SSM.cpp:14698-14703`）。

## 阶段一：拉起入口（Ability Manager Service → WMS → SCB）

### IPC 入口与 caller 记录

Ability Manager Service 持有 root session 的 token（`RegisterRootSceneSession` 时经 `SetRootSceneSession` 交给 Ability Manager Service），拉起时调
`SessionProxy::PendingSessionActivation`（TF_ASYNC，`session_proxy.cpp:564-578`），IPC code
`TRANS_ID_ACTIVE_PENDING_SESSION`（`session_ipc_interface_code.h:30`）。事件会落在 root session 或前台A的 session 上，
两者都注册了监听。

`SceneSession::PendingSessionActivation`（`scene_session.cpp:6899-6955`）要点：

- 校验 `PERMISSION_MANAGE_MISSION`，仅 Ability Manager Service 等系统调用者可入；
- `sessionInfo_.startMethod = StartMethod::START_CALL`（`:6923`）标记本次是“拉起”；
- `MakeSessionInfoDuringPendingActivation`（`:6726-6800`）从 `AAFwk::SessionInfo` 提取 caller 信息：
  `callerToken_`、`requestCode`、`callingTokenId_`（`:6744-6747`），`callerBundleName_`/`callerAbilityName_` 来自 want 的
  `PARAM_RESV_CALLER_BUNDLE_NAME/ABILITY_NAME`（`:6738-6739`）；注意 `callerPersistentId_` 此处填的是接收 IPC 的
  session（root 或A）自己的 id（`:6737`），真实拉起者的修正见下；
- `CalculateStartWindowType(info, hideStartWindow)`（`:6796`，实现 `:6840-6855`）仅在 PC/PC 模式且同 bundle 时按
  `OPTIONAL_SHOW` + Ability Manager Service 的 `hideStartWindow` 决定 `RETAIN_AND_INVISIBLE`；
- 最后 `pendingSessionActivationFunc_(info)`（`:6949-6950`）把事件交给 NAPI 桥。

### NAPI 桥与“真实拉起者”

root 侧 `JsRootSceneSession::PendingSessionActivation`（`js_root_scene_session.cpp:413-467`）先 `GenSceneSession` 建session
（内部经 `RequestSceneSession` → `CreateSceneSession`，主窗口创建 `MainSession`），再处理 caller：

```cpp
bool isNeedBackToOther = info.want->GetBoolParam(AAFwk::Want::PARAM_BACK_TO_OTHER_MISSION_STACK, false);
if (isNeedBackToOther) {
    info.callerPersistentId_ = GetRealCallerSessionId(sceneSession);   // 当前聚焦session，即前台A
    VerifyCallerToken(info);
} else {
    info.callerPersistentId_ = INVALID_SESSION_ID;                     // 常见取值
}
```

即 `callerPersistentId_` 只有 want 携带 `PARAM_BACK_TO_OTHER_MISSION_STACK` 时才是真实拉起者 session id，否则是
`INVALID_SESSION_ID`；排查“谁拉起谁”应优先看 `callerBundleName_`/`callerToken_`。前台A的 session 侧
`JsSceneSession::PendingSessionActivation`（`js_scene_session.cpp:5366-5380`）在 `GenSceneSession` 后，若
`startWindowType_ == RETAIN_AND_INVISIBLE` 且 session 处于 `STATE_DISCONNECT`，置 `SetHidingStartingWindow(true)`。

### SCB 消费与激活回调

SCB 侧 `SCBRootSceneSession.ts:116-168` / `SCBSceneSession.ts:2444-2498` 处理 `pendingSceneSessionActivation`，关键分叉：

- `callState != UNKNOWN` → `startSceneByCall`（startAbilityForResult 链路）；
- PC/PC 模式且 `startupVisibility == STARTUP_HIDE` → 隐藏启动；
- caller 是普通应用 session（`SCBSceneSession` 实例且非悬浮）→ `startSceneTransition(toInfo, callerSession.sceneInfo)`
  （`SCBSceneSession.ts:2496`），即“应用拉应用”主路径；caller 非应用（桌面/通知等）→ `startSceneFromOther`。

面板侧 `SCBScenePanelViewModel.sceneStartTransitionIfNeed`（`SCBScenePanelViewModel.ets:6008-6094`）按序执行：
获取/新建 B 的容器会话（`:6042-6046`）、`requestActivation`（`:6050`）、`raiseSceneToTop`（`:6051`）、若干无动画/同容器
跳过分支（`:6053-6061`）、浮窗分支不退后台 A（`:6063-6075`）、顶层活跃容器按需退后台（`:6076-6080`），最后
`startSceneContainerTransition`（`:6092`）进入动画执行器。

B 的激活经 NAPI `requestSceneSessionActivation`（`SCBSceneSessionManager.ts:4791-4812`）回到 WMS
`RequestSceneSessionActivation`（`SSM.cpp:4105-4151`，hitrace `ssm:RequestSceneSessionActivation(%d )`），进入
`RequestSceneSessionActivationInner`（`:4280-4417`）分流（见“结论先行”第 3 条）。

### App B 前台化与 App A 退后台

App B 建窗后 `WindowScene::GoForeground`（`wm/src/window_scene.cpp:173-187`）以 `Show(reason, false, true)` 触发
`TRANS_ID_FOREGROUND`；服务端 `SceneSession::Foreground`（`scene_session.cpp:495-536`，含锁屏/ShowWhenLocked 与
pid/identityToken 校验）→ `ForegroundTask`（`:538-580`）→ 基类 `Session::Foreground`（`session.cpp:1855-1888`）置
`STATE_FOREGROUND` 并 `NotifyForeground()`；`UpdateSessionState` → `sessionStateChange` 事件通知 SCB。

A 的退后台由 SCB 在动画开始前发起（见“阶段三”），WMS 侧 `RequestSceneSessionBackground` → `BackgroundTask` 为主窗保存
快照（`SaveSnapshot`）后置 `STATE_BACKGROUND`；此后按设备形态分流：PC/Pad 场景生命周期由 WMS 直接
`NotifySessionBackground(1, true, true)` 通知 App（`SSM.cpp:4774-4777`），手机场景经 `MinimizeUIAbilityBySCB`
（sceneFlag 仍为 `ABILITY_CALL`，`SSM.cpp:4778-4784`）驱动 A 进程走 ability onBackground。

## 阶段二：启动窗

### 类型与配置决策

`StartWindowType`（`ws_common.h:428-432`）三值：`DEFAULT`（显示启动窗）、`RETAIN_AND_INVISIBLE`（保留但不可见）、
`REMOVE_NODE_INVISIBLE`。应用侧 start_window.json 的 `startWindowType` 字符串（`REQUIRED_SHOW` 默认 /
`REQUIRED_HIDE` / `OPTIONAL_SHOW`）经 `CONVERT_STRING_TO_START_WINDOW_TYPE_MAP` 映射（`SSM.cpp:230-234`）。

启动页资源（`StartingWindowInfo`，`ws_common.h:1290-1301`：背景色/图标/插画/品牌/背景图）由
`SceneSessionManager::GetStartupPage`（`:7364-7434`）按“桌面图标 want 参数 → 内存 `startingWindowMap_` → RDB → Bundle Manager Service 资源”
四级查找；内存缓存与 RDB 均未命中时才经 Bundle Manager Service 查询并解析应用资源，成功后回写内存缓存与 RDB（深浅色分别缓存）。

PC/PC 模式下 `SetSessionInfoStartWindowType`（`:7786-7804`）从 RDB 读取字符串配置回写 sessionInfo；
`RETAIN_AND_INVISIBLE` 且 `STATE_DISCONNECT` 时置 `SetHidingStartingWindow(true)`，该标志会：

- 推迟 `SCB_START_APP` 焦点（`SSM.cpp:16860-16863`）；
- 使可见性统计强制可见、无障碍窗口列表被过滤；
- `SetLeashWindowAlpha(true)` 把 leash SurfaceNode alpha 置 0 并禁触摸（`session.cpp:5003-5014`）。

### preload 启动窗

开关由 SCB 一次性使能：phone `MainAbility.ets` 初始化时调
`SCBSceneSessionManager.supportPreloadStartingWindow()`（`window_scene_board/product/phone/src/main/ets/MainAbility/MainAbility.ets:545-546`）→ WMS
`ConfigSupportPreloadStartingWindow` 置 `systemConfig_.supportPreloadStartingWindow_`。

`SceneSessionManager::PreLoadStartingWindow`（`SSM.cpp:7543-7619`，ffrt 异步，hitrace `ssm:PreLoadStartingWindow`）：

1. 后台 session 且有持久化快照 → 改为 `PreloadSnapshot()`（解码快照文件，`:7563-7567`）；
2. 同步加载开关（`const.window.sync_startingWindow`）打开时跳过（`:7569-7571`）；
3. 仅预载“早版本图标”（未配置 start_window.json 的应用），解码 pixelMap/SVG 后
   `SetPreloadStartingWindow` 存入 Session（`:7589-7612`），必要时裁剪后落盘 `SaveStartWindow`。

预载的 pixelMap 不进入 ArkTS 层，由 native 侧（ace_engine 绘制启动窗时）直接消费；SCB 结束通知经
`NotifyPreLoadStartingWindowFinished` → `ILifecycleListener::OnPreLoadStartingWindowFinished`。

### 移除链路与退出动画

应用主动移除（要求模块 metadata `enable.remove.starting.window=true`，由
`GetEnableRemoveStartingWindowFromBMS` 读入 `SetEnableRemoveStartingWindow`）：App 调
`Window::NotifyRemoveStartingWindow()` → IPC `TRANS_ID_APP_REMOVE_STARTING_WINDOW`
（`session_ipc_interface_code.h:34`）→ `Session::RemoveStartingWindow`（`session.cpp:2081-2091`）遍历
`ILifecycleListener` 调 `OnAppRemoveStartingWindow()`——本仓到此为止，消费实现在 ace_engine `WindowScene` 组件（未本地核验）。

退出动画开关 `NeedStartingWindowExitAnimation()`（`session.h:550` 默认 true）由 SCB 写入：仅手表设备或通话类应用
（`com.ohos.callui.MainAbility` / meetime 通话 UI，见 `SCBSceneInfo.isPhoneCall`）会
`setStartingWindowExitAnimationFlag(false)` 走直接移除，其余保持默认动画移除，见
`SCBSceneSession.ts:1693-1706`（构造函数 `:1182` 触发）。退出动画参数来自 WMS XML 配置
`startWindowTransitionAnimation` 解析出的 `StartingWindowAnimationConfig`（`ws_common.h:1325-1331`：
enabled/200ms/linear/opacity 1→0），解析见 `SSM.cpp:1699-1722`（`ConfigStartingWindowAnimation`）。

预启动（prelaunch/game prelaunch）场景另有专门移除：App 侧刷满 `frameNum` 帧 vsync 后
`NotifyRemovePrelaunchStartingWindow`（`wm/src/window_session_impl.cpp:1799-1830`），服务端
`SceneSession::NotifyRemovePrelaunchStartingWindow`（`scene_session.cpp:754-768`）同样止于
`ILifecycleListener::OnRemovePrelaunchStartingWindow`。

### 边界：启动窗 UI 在哪里

启动窗的绘制（消费背景色/图标/插画/品牌、快照与启动页二选一）与退出动画的执行体不在本仓，也不在
`window_scene_board` ArkTS 代码中：全仓检索 `StartingWindowInfo`/`startWindowType`/`illustration`/`branding`/
`REQUIRED_SHOW` 主代码零命中，仅有 `module.json5` 里 SCB 自身 UIAbility 的 `startWindowIcon/startWindowBackground`
声明与 NAPI 封装。应用 surface 的宿主是 ace_engine 提供的 `WindowScene(persistentId)` ArkUI 组件，由 SCBScene 挂载：

```ts
WindowScene(this.sceneSession?.session.persistentId) { ... }   // SCBScene.ets:869
```

见 `window_scene_board/product/phonebase/src/main/ets/SceneBoard/scenemanager/SCBScene.ets:867-877`。诊断启动窗显示/
消失问题时应以本仓日志（`WMS_STARTUP_PAGE` 等）+ ace_engine 侧断点为边界，不要在本仓或 scene_board 找绘制代码。

## 阶段三：转场动画执行（SceneBoard）

### 默认推入动画

`SCBScenePanelAnimator.sceneContainerAppTransitionChange`（`SCBScenePanelAnimator.ets:219-385`，注释即“窗口转换动效，
应用拉应用、应用返回拉起方”）：

1. **先发退后台请求**：`requestFromContainerBackgroundOrDestruction(fromContainerSession, ...)`（`:243-244`），A 的
   `requestBackground` 与动画并行，不等动画结束；
2. **初始态**：B 容器先平移出屏（`setTranslateXWithDfx(px2vp(screenWidth))`，`:252-253`）；返回转场（isBackTransition）
   则从左缘进入（`:264-266`）；
3. **主位移**：`animateTo({ duration: 600, curve: Curve.Friction, onFinish: finishCallback })` 内 A 左移出屏
   （`-width - DISTANCE_APP_TRANSITION`）、B 落位（`:335-357`）；
4. **缩放节奏**：200ms 缩放到 0.9（`:358-370`），随后 400ms（delay 200）回弹 1.0（`:371-384`）；
5. **自定义分支**：同 bundle 且 B 的 `startAnimationOptions.type == FADE_IN_OUT` 时改走
   `WindowAnimationUtils` 的 fadeIn/fadeOut（各 300ms），日志 `Start fadeInOut animation`（`:317-333`）。

动画属性挂在 `SCBSceneContainerSession` 的渲染状态对象上（`needRenderTranslate/needRenderScale/needRenderAlpha`），
绑定到 `SCBSceneContainer`/`SCBScene` ArkUI 组件属性。冷启动期间 B 容器的内容就是启动窗 surface，因此
**open 动画不依赖应用首帧**；依赖首帧的场景（如通话 fadeIn）用 `setBufferAvailableCallback` 等待。

### 动画完成处理

`finishCallback`（`:286-315`）：`performanceMonitor.end('APP_TRANSITION_TO_OTHER_APP')`、恢复 B 容器圆角、
A 容器 `transitionOutFinish`（清 `isDisappearing`、清 transition 计数、清理桌面图标动效控制器）、按需移除 A 容器。
`SCBSceneContainerSession.transitionOutFinishIfNeed` 在动画计数归零后发 `SCENE_CONTAINER_TRANSITION_OUT_END` 事件，
转入侧对应 `transitionInFinishIfNeed` 通知桌面图标动效复位。

### 容器 z 序与尺寸动画通道

容器列表变化后 `SCBSceneSessionManager.refreshZOrder` 逐个 `setZOrder` 并 `checkSceneZOrder`。窗口尺寸类动画
（最大化/还原等）不走本通道，而是 SCB 先 `updateSizeChangeReason` 设置 reason，WMS 在
`Session::UpdateClientRectInfo`（`session.cpp:6296-6312`）按 reason 组装 `SceneAnimationConfig`，客户端
`WindowSessionImpl::UpdateRect`（`wm/src/window_session_impl.cpp:1354-1401`）对 `SCENE_WITH_ANIMATION` 走
`UpdateRectForResizeAnimation`，在 App 进程内用 WMS 传来的曲线/时长执行 RS 隐式动画。应用拉应用（全屏推入）不改变
窗口 rect，通常不触发该通道。

## 阶段四：完成与首帧衔接

### bufferAvailableChange

WMS `Session::SetBufferAvailable(bool, bool startWindowInvisible)`（`session.cpp:5561-5568`）把首帧 buffer 状态经
`bufferAvailableChangeFunc_` 发给 SCB JS 事件 `bufferAvailableChange`。SCB `onBufferAvailableChange`
（`SCBSceneSession.ts:3416-3443`）依次：

1. `setSessionAliveStatus` 更新存活状态；
2. 执行并清空 `bufferAvailableCallbackList`（依赖首帧的动画回调，如通话 fadeIn）；
3. 第一个 `bufferAvailableChange` 事件时应用 systemBarProperty（`_isSystemBarPropertyApplied` 置位后不再重复）；
4. `startWindowInvisible` 为 true 时 `notifyApplicationLoadedWhenStartWindowInvisible`（phone 的 mission processor
   为空实现，PC 侧打点）。

注意第二参数只驱动“内容已加载”通知，启动窗节点的移除由 native 侧（ace_engine）完成，两者不是同一事件。

### 焦点推进

前台化后 WMS 的 focus post-process（`SSM.cpp:16848-16872`）：`reason == FocusChangeReason::SCB_START_APP` 时，
若 `IsDelayFocusChange()`（= `GetHidingStartingWindow()`，`scene_session.cpp:6319-6322`）则本帧跳过，等窗口可见后
`IsFocusedOnShow` 补偿；否则 `RequestSessionFocusImmediately`。SCB 侧通过 `shiftFocus` 事件回推同步聚焦状态。

### CUSTOM 动画延迟 Background

子窗/系统窗设置 `WindowAnimation::CUSTOM` 动画 flag 时，`SubSession::Hide` 只
`NotifyIsCustomAnimationPlaying(true)` 便返回，不真正 Background（surfaceNode 会随 Background 移除，自定义动画需要
它存在），见 `sub_session.cpp:190-197`；SCB 动画完成后调 `UpdateWindowSceneAfterCustomAnimation(false)`，WMS 才执行
真正的 `Background()` 并复位标志，见 `scene_session.cpp:6332-6360`。主窗口应用拉应用不经过该分支，但排查“动画中
窗口提前消失/黑屏”时应知道该机制。

### 最近任务热启动的 blank

从最近任务热启动时 SCB 可能用 `setBlank(true)` + `setBufferAvailableCallbackEnable(true)` 显示白/黑blank 层，动画
结束后 `removeBlank()`（`SCBScenePanelViewModel.ets:2423-2443`、`:6747-6770`）。这是 SCB 侧行为，与启动窗
（native 绘制）是两条不同的“过渡画面”通路，诊断“热启动闪白/闪黑”时先区分是 blank 还是快照/启动窗。

## 日志与 Trace 关联表

按链路顺序（WMS 域 `0xD004200` 系；SCB 经 `LogHelper`/`WinLog`，域/tag 如 `WMS_MAIN=0x04202`、
`WMS_ANIMATION=0x0421E`）：

| 阶段 | Hilog/Trace 关键字 | 位置 |
|---|---|---|
| Ability Manager Service 拉起 IPC | `set reuseDelegatorWindow`（WMS_LIFE） | scene_session.cpp:6925 |
| caller 修正 | `isNeedBackToOther`（WMS_LIFE） | js_root_scene_session.cpp:429 |
| 建 session | `Create MainSession, id:`（WMS_MAIN） | SSM.cpp:3384（经 GenSceneSession → RequestSceneSession） |
| pending 激活（SCB） | `onPendingSceneSessionActivation sceneInfo.persistentId:` | SCBRootSceneSession.ts:117 |
| pending 激活（A 的 session） | `onPendingSessionActivation to:` | SCBSceneSession.ts:2450 |
| 面板接管 | `onSceneTransition Failed to get fromContainerSession` / `without animation` / `skip, scenePanelState is FULLSCENE` | SCBScenePanelViewModel.ets:6013/6054/6059 |
| 激活请求 | `[RequestId:..][SCBRequestId:..]Request active id:..`（WMS_MAIN） | SSM.cpp:4129 |
| 激活请求（SCB） | `[SCBMain][id:..]requestSceneSessionActivation.` | SCBSceneSessionManager.ts:4797 |
| 冷启动分流 | `Begin StartUIAbility` / `Background switch on, isNewActive..` | SSM.cpp:4352/4364 |
| 热启动分流 | `NotifySessionForeground: %{public}d` | SSM.cpp:4379 |
| Ability Manager Service 拉起 | `StartUIAbilityBySCB: persistentId:..` / `StartUIAbility sceneFlag:.. retCode:..` / `Timeout, currentUserId:` | SSM.cpp:4214/4230/4235 |
| 前台化 | `[id:..] state:.., isTerminating:..`（WMS_LIFE） | session.cpp:1859 |
| 转场动画开始 | `sceneContainerAppTransitionChange from .. to ..`（WMSMain） | SCBScenePanelAnimator.ets:223 |
| 自定义动画 | `startAnimationOptions:` / `Start fadeInOut animation`（WMSAnimation） | SCBScenePanelAnimator.ets:229/318 |
| 转场动画结束 | `sceneContainerAppTransitionChange animation finish.` | SCBScenePanelAnimator.ets:287 |
| 启动窗移除 | `success id:..`（WMS_STARTUP_PAGE，App侧） | window_scene_session_impl.cpp:2586 |
| 首帧 | `onBufferAvailableChange, isBufferAvailable:..`（SCB） | SCBSceneSession.ts:3417 |
| 内容加载通知 | `start window invisible, notify app content loaded.` | SCBSceneSession.ts:3440 |
| preload | `change to preloadSnapshot` / `check no need preLoad` / `sync load starting window`（WMS_PATTERN） | SSM.cpp:7564/7578/7570 |
| 焦点延迟 | `delay focus change until the window is visible`（WMS_FOCUS） | SSM.cpp:16861 |
| CUSTOM 延迟 | `background after custom animation id`（WMS_LAYOUT） | scene_session.cpp:6351 |
| 退后台（SCB 发起） | `requestSceneSessionBackground, isDelegator:.. isToDeskTop:.. isSaveSnapshot:..` | SCBSceneSessionManager.ts:4873（经 RequestSceneSessionBackground） |

HiTrace（`HITRACE_TAG_WINDOW_MANAGER`）：`ssm:RequestSceneSession(%d )`（SSM.cpp:3885）、
`ssm:RequestSceneSessionActivation(%d )`（SSM.cpp:4128）、`ssm:PreLoadStartingWindow`（SSM.cpp:7551）、
`WMS::WindowRectUpdate::ClientRecv::UpdateRect`（window_session_impl.cpp:1380-1382）。SCB 侧 HiTraceChain：
`PendingSceneSessionActivation`（SCBSceneSession.ts:1009）、`RequestSceneSessionActivation`
（SCBSceneSessionManager.ts:4799）；性能打点 `APP_TRANSITION_TO_OTHER_APP`（SCBScenePanelAnimator.ets:257/291）。

## 常见误判

- 在统一架构里搜 `RemoteAnimation`/`RSWindowAnimationTarget`/`OnStartApp` 找应用拉应用动画——找不到；那是分离架构
  `wmserver/` 的实现，统一架构对应接口是 no-op。
- “启动窗由 WMS C++ 画”或“由 scene_board ArkTS 画”——都不对；绘制与退出动画执行在 ace_engine `WindowScene` 组件，
  本仓与 scene_board 都只有数据、开关和通知链。
- “open 动画要等应用首帧”——默认水平推入作用在 SCB 容器属性上，冷启动期间内容就是启动窗，不等首帧。
- 把 `callerPersistentId_` 当可靠拉起者——只有 want 带 `PARAM_BACK_TO_OTHER_MISSION_STACK` 时才填真实拉起者，
  否则是 `INVALID_SESSION_ID`；拉起者身份看 `callerBundleName_`/`callerToken_`。
- 把 `NotifySessionForeground(1, true)` 的 1 读成 `ABILITY_CALL`——1 是 `KEYGUARD`；`ABILITY_CALL(4)` 只出现在
  SCB→Ability Manager Service 的 sceneFlag 上。
- 期望 A 的 Background 晚于动画结束——实际请求在动画开始前发出，快照保存与动画并行。
- 以为 `withAnimation` 参数控制主窗口转场——它只影响系统窗的动画 flag 属性，主窗口转场由 SCB 编排决定。
- 把 `bufferAvailableChange` 第二参数当成“启动窗已移除”——它只触发内容加载通知，节点移除在 native 侧独立完成。
- 在 phone 上排查 `setSystemAnimatedScenes`——该链路仅 PC 模式窗口分合/最小化场景使用，phonebase 无调用。

## 最短定位决策树

1. **拉起无反应**：查 WMS `PendingSessionActivation` 是否收到（权限/`reuseDelegatorWindow` 日志）→ SCB
   `onPendingSessionActivation to:` → `requestSceneSessionActivation.` → WMS `Request active id:` →
   `StartUIAbilityBySCB` 的 `retCode`；XCollie 超时会打 `Timeout, currentUserId:`。
2. **无转场动画**：查 SCB `onSceneTransition without animation`（want 指定无动画）→ `skip, scenePanelState is
   FULLSCENE same container`（同容器跳过）→ `startAnimationOptions` 是否走了自定义/fadeInOut 分支 → 容器是否是
   float（浮窗不退后台 A）。
3. **启动窗不消失/闪黑**：应用是否配置 `enable.remove.starting.window` 且调用了 `removeStartingWindow`（App 侧
   `success id:` 日志）→ `onBufferAvailableChange` 是否到达 → 手表/通话场景确认 `setStartingWindowExitAnimationFlag(false)`
   是否预期关闭了退出动画；继续深入需转 ace_engine `WindowScene` 组件断点。
4. **动画中 A 提前消失/黑屏**：确认是否误入 CUSTOM 动画延迟 Background 分支（`background after custom animation id`）；
   普通主窗口不应出现。
5. **焦点异常**：`RETAIN_AND_INVISIBLE` 下 `delay focus change until the window is visible` 属预期；确认
   `IsFocusedOnShow` 后续补偿是否执行。
6. **热启动闪白/黑**：区分 SCB blank 层（`setBlank`/`removeBlank`）、快照启动窗与启动页三条“过渡画面”来源。

## 当前观测性与测试缺口

- `Session::RemoveStartingWindow` 成功路径无携带 persistentId 的日志，仅 stub 入口有 `Called!`；跨进程只能靠 App 侧
  `success id:` 与 SCB `onBufferAvailableChange` 推断。
- 启动窗“显示时刻”没有本仓可查日志（绘制在 ace_engine），WMS 侧只能通过 `PendingSessionActivation` 大日志中的
  `startWindowType`、preload 日志间接确认输入。
- `bufferAvailableChange` 与启动窗移除没有共同 generation ID，时序只能按时间戳关联。
- 转场动画的 600ms/200ms/400ms 参数硬编码在 `SCBScenePanelAnimator`，无配置化开关，性能劣化只能靠
  `APP_TRANSITION_TO_OTHER_APP` 打点对比。
- 单测覆盖：拉起入口与生命周期见 `window_scene/test/unittest/scene_session_manager_stub_lifecycle_test.cpp`、
  `session_lifecycle_test.cpp`；启动窗数据决策见 `window_scene/test/unittest/attribute/` 相关用例；SCB 动画工具类有
  `window_scene_board/staticcommon/basecommon/windowscene/src/ohosTest/ets/test/ets/WindowScene/animation/`。缺少
  “WMS 事件 → SCB 动画 → 完成回执 → 焦点推进”的跨仓顺序断言。
