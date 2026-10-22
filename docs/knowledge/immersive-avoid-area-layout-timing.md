# 避让区通知与布局通知时序定位

## 文档定位

本文专项说明统一架构下，避让区通知、窗口布局通知和 ArkUI 最终布局之间的时序。重点回答：

- 为什么同一次状态栏、键盘、旋转、缩放或窗口矩形变化，可能先看到避让区 callback，也可能先看到
  `windowSizeChange`；
- 为什么 callback 已到达，画面仍可能晚一帧才正确；
- 如何区分随 `UpdateRect` 成组下发、单类型直接通知和逐类型全量通知；
- 如何用现有 Hilog、Trace、窗口 ID、type、reason、rect 和 area 重建一次问题的真实顺序。

避让区的获取规格、三种通知机制和 UEC 链路见
[避让区](./immersive-avoid-area.md)；完整日志入口和测试清单见
[调试与测试](./immersive-debugging-and-tests.md)。本文只展开两类通知之间的时序，不把“通知发出”、
“应用 callback 执行”、“ArkUI 安全区写入”和“最终布局完成”合并为同一个事件。

## 证据快照

本文于 2026-09-08（Asia/Shanghai）按以下源码快照核对：

| 仓库 | 分支 | commit | 最近提交日期 |
|---|---|---|---|
| `window_window_manager` | `master` | `dd8e1ae07646` | 2026-09-07 |
| `window_scene_board` | `master` | `779726634368` | 2026-09-02 |
| `arkui_ace_engine` | `master` | `37c17d14a8ac` | 2026-09-04 |

源码变化后应重新检索本文中的符号和日志文本；行号只对应上述快照。

## 结论先行

1. 只有“跟随布局的 map 通路”在一次 `SessionStage::UpdateRect` IPC 载荷中同时携带新 rect 和按该
   session 当前几何计算的多类型 avoid-area map。它是传输层面的成组快照，但应用 callback、ArkUI
   UI/Platform 任务和最终渲染仍不是一个原子步骤。
2. 单类型通知和逐类型全量通知走独立的 `SessionStage::UpdateAvoidArea` 异步 IPC，不携带 rect，也没有
   layout/avoid generation ID。客户端收到后使用当时的 `WindowSessionImpl::GetRect()`刷新 viewport。
3. `HandleLayoutAvoidAreaUpdate()`的“上一帧布局完成”门禁来自 RootScene 的待处理 VSync 计数；它不证明
   应用进程已经处理 `UpdateRect`，更不证明 ArkUI 已完成 Measure/Layout。
4. 门禁失败时只保留一个 `AVOID_AREA` dirty bit，不保存原请求 type。多个单类型或 `TYPE_END`请求可能被
   合并，并在下次 `FlushUIParams`中改走普通窗口的 map 通路或避让源的单类型通路。
5. 客户端 map 通路的 C++调用顺序是：窗口尺寸 listener -> avoid-area cache/listener ->
   `UIContent::UpdateViewportConfig`。但两个 NAPI listener 会再次投递 JS 事件，UIContent 也会投递或同步执行
   UI/Platform 任务，因此 JS callback 顺序不能代表 ArkUI 布局完成顺序。
6. `AceLayout`的 `UVC ...`日志在 ArkUI 选择并执行后续 UI/Platform 任务前打印。看到它只能证明 UIContent
   已接到参数，不能证明 `SafeAreaManager`已写入或页面已重排。

## 必须分开的六个时钟

| 时钟/队列 | 代表事件 | 不能据此推断 |
|---|---|---|
| SceneBoard RootScene VSync | `RequestVsync`、`FlushUIParams` | 应用窗口的布局已经生效 |
| WMS `OS_sceneSession`任务队列 | `ssm:FlushUIParams`、`ssm:UpdateAvoidArea:PID:*` | 异步 IPC 已被客户端处理 |
| `SessionStage`异步 IPC | `UpdateRect`、`UpdateAvoidArea`、occupied-area | 客户端 handler、JS、UI 三个队列已执行 |
| `WindowSessionImpl` handler | `WMS_WindowSessionImpl_UpdateRectForOtherReason`、`UpdateAvoidArea` | NAPI callback 或 ArkUI UI/Platform task 已执行 |
| NAPI JS 事件队列 | `JsWindowListener::OnSizeChange`、`OnAvoidAreaChanged` | `SafeAreaManager`和页面 Geometry 已更新 |
| ArkUI UI/Platform/VSync 管线 | `ArkUIUpdateViewportConfig`、`SyncSafeArea`、`UITaskScheduler::FlushTask` | RenderService 已显示该帧 |

定位时至少用 `persistentId/windowId + displayId + AvoidAreaType + WindowSizeChangeReason + rect + area`关联事件。
仅比较“两个 callback 谁先打印”不足以确定服务端发送顺序。

## SceneBoard Core 一帧内的服务端顺序

`SceneSessionManager::FlushUIParams()`先通知 RootScene 消耗一次 VSync 请求，再把真正的 session 更新投递到
WMS task scheduler：

```cpp
if (onFlushUIParamsFunc_ != nullptr) {
    onFlushUIParamsFunc_();
}
taskScheduler_->PostAsyncTask([this, screenId, uiParams = std::move(uiParams)]() {
    HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER, "SceneSessionManager::FlushUIParams");
    TLOGND(WmsLogTag::WMS_PIPELINE, "FlushUIParams");
    // ... update every session, keyboard, post process ...
}, __func__);
```

来源：`window_scene/session_manager/src/scene_session_manager.cpp:16597-16607`。

异步任务内部的固定阶段是：

```text
遍历本屏 session，执行 UpdateUIParam
  -> interactive / visible
  -> scale
  -> rect，并可能立即发送携带 avoidAreas map 的 UpdateRect
遍历结束
  -> KeyboardSession::CalculateOccupiedAreaAfterUIRefresh
结束 processingFlushUIParams
  -> focus/property post-process
  -> 每个 session 的 PostProcessNotifyAvoidArea
  -> ResetDirtyFlags
```

对应代码在
`window_scene/session_manager/src/scene_session_manager.cpp:16613-16645`、`:16647-16669`和
`:16837-16899`。跨 session 的先后取决于 manager 遍历顺序，不应把某个 persistentId 的偶然顺序写成产品契约；
但“所有 `UpdateUIParam` -> keyboard after-refresh -> property/avoid post-process”是当前函数内的阶段顺序。

单个 session 内，scale 明确早于 rect：

```cpp
dirtyFlags_ |= UpdateScaleInner(...) ? SessionUIDirtyFlag::SCALE : 0;
bool isUpdateRectDirty = UpdateRectInner(uiParam, GetSizeChangeReason());
if (isUpdateRectDirty) {
    dirtyFlags_ |= static_cast<uint32_t>(SessionUIDirtyFlag::RECT);
}
```

来源：`window_scene/session/host/src/scene_session.cpp:9837-9854`。这会形成下文“scale 与 rect 同帧”这一条
高风险时序。

## 上一帧布局门禁的真实语义

`SceneSession::HandleLayoutAvoidAreaUpdate()`在直接计算前查询 RootScene：

```cpp
bool isLayoutFinished = false;
WSError ret = isLastFrameLayoutFinishedFunc_(isLayoutFinished);
if (ret != WSError::WS_OK) {
    TLOGE(WmsLogTag::WMS_IMMS, "isLastFrameLayoutFinishedFunc failed, ret %{public}d", ret);
    return ret;
}
if (!isLayoutFinished) {
    MarkAvoidAreaAsDirty();
    return WSError::WS_OK;
}
```

来源：`window_scene/session/host/src/scene_session.cpp:3026-3047`。

RootScene 的判断只是 `requestVsyncTimes_ <= 0`：

```cpp
bool RootScene::IsLastFrameLayoutFinished()
{
    int32_t requestTimes = vsyncStation_->GetRequestVsyncTimes();
    TLOGD(WmsLogTag::WMS_LAYOUT, "vsync request times: %{public}d", requestTimes);
    return requestTimes <= 0;
}

void RootScene::OnFlushUIParams()
{
    vsyncStation_->DecreaseRequestVsyncTimes();
}
```

来源：`wm/src/root_scene.cpp:346-355`；VSync 请求加一和安全减一见
`utils/src/vsync_station.cpp:139-162`、`:353-363`。

这意味着：

- `FlushUIParams()`入口先减计数，服务端异步 task 随后才真正处理 session；
- 门禁回答的是“RootScene 是否还有未消费的布局 flush 请求”，不是“客户端/ArkUI 是否完成布局”；
- 门禁为 false 时没有专用 Hilog。当前只能通过此前的 `vsync request times: N`、后续 dirty/flush 行为和断点
  推断 `MarkAvoidAreaAsDirty()`分支；该函数本身也不记录 type，见
  `window_scene/session/host/src/scene_session.cpp:10427-10430`。

### dirty 延后的通路变化

下一次 `FlushUIParams`中，`PostProcessNotifyAvoidArea()`只检查 dirty flags：

```cpp
return ((dirty & (SessionUIDirtyFlag::VISIBLE | SessionUIDirtyFlag::RECT)) && IsImmersiveType()) ||
    ((dirty & SessionUIDirtyFlag::AVOID_AREA) && isVisible_.load());
// ...
if ((IsImmersiveType() || !IsDirtyWindow()) && specificCallback_->onUpdateAvoidArea_) {
    specificCallback_->onUpdateAvoidArea_(GetPersistentId());
}
```

来源：`window_scene/session/host/src/scene_session.cpp:10051-10074`。

- 普通窗口没有 rect dirty 时，延后的更新进入 manager，再以 `AVOID_AREA_CHANGE`触发携带 map 的
  `UpdateRect`；有 rect dirty 时，该帧自身的 `UpdateRect`已经携带 map，post-process 不重复发。
- status/navigation bar 或 IME float 这类避让源进入 manager 后，重新按 window type 映射为
  `TYPE_SYSTEM`或 `TYPE_KEYBOARD`单类型通知。
- session 隐藏时 `AVOID_AREA` dirty 会被保留，直到后续可见，见
  `window_scene/session/host/src/session.cpp:5993-5999`。

所以，“请求时是 `TYPE_END`”不保证延后恢复时仍逐类型全量发送。

## 通路一：跟随布局的 map 通知

### 服务端：在新 session rect 上计算后成组发送

`UpdateRectInner()`先把新 rect 写入 `LayoutController`，再调用 `NotifyClientToUpdateRect()`；后者调用
`GetAllAvoidAreas()`并把 map 传入同一次 `UpdateRect`：

```cpp
layoutController_->SetSessionRect(rect);
// ...
NotifyClientToUpdateRect("WMSPipeline", std::nullopt, transaction);

std::map<AvoidAreaType, AvoidArea> avoidAreas;
GetAllAvoidAreas(avoidAreas);
Session::UpdateRectWithLayoutInfo(winRect, reason, updateReason, transaction, avoidAreas);
```

来源：`window_scene/session/host/src/scene_session.cpp:9971-9992`、`:2052-2104`。

`UpdateRect`和 map 使用一个 `MessageOption::TF_ASYNC`事务，序列化见
`window_scene/session/container/src/zidl/session_stage_proxy.cpp:174-245`。因此这一载荷内部不会出现“新 rect 配旧
map”或“map 的各 type 分批到达”；但后续 listener 和 UI 处理仍会拆开。

### 客户端：尺寸回调先入队，再更新避让区和 UIContent

普通 reason 的 handler task 中，当前源码顺序是：

```cpp
NotifySizeChange(wmRect, wmReason);
// ...
UpdateViewportConfig(wmRect, wmReason, rsTransaction, nullptr, avoidAreas);
```

`UpdateViewportConfig()`内部先更新 `lastAvoidAreaMap_`并调用避让区 listener，最后调用 UIContent：

```cpp
for (const auto& [type, avoidArea] : avoidAreas) {
    if (UpdateLastAvoidAreaIfChanged(type, avoidArea)) {
        NotifyAvoidAreaChange(new AvoidArea(avoidArea), type);
    }
}
// ...
uiContent->UpdateViewportConfig(config, reason, rsTransaction, GetLastAvoidAreaMapCopy(), occupiedAreaInfo_);
```

来源：`wm/src/window_session_impl.cpp:1655-1677`、`:2288-2304`和`:2371-2384`。rotation/page-rotation等
专用分支也在各自任务内先 `NotifySizeChange()`，再 `UpdateViewportConfig()`，见同文件`:1457-1462`、
`:1544-1549`和`:1605-1609`。

需要区分三层“先后”：

1. C++ listener 调用顺序：size 在 avoid 之前；
2. NAPI 入队顺序：两者都使用 `napi_send_event(..., napi_eprio_immediate, ...)`，rotation 的 size callback
   例外为同步执行，见
   `interfaces/kits/napi/window_runtime/window_napi/js_window_listener.cpp:122-159`、`:195-230`；
3. 画面生效顺序：UIContent 还会经过 ArkUI UI/Platform task，不能用 JS callback 先后来替代。

即使 rect 值未变，只要 reason、通知标志等条件变化，handler 仍可能执行 viewport 更新；反过来，JS
`windowSizeChange`还会按宽高自行去重。不能用“没有 JS size callback”断言没有收到 `UpdateRect`。

## 通路二、三：单类型与逐类型全量直接通知

两者共用 `SessionStage::UpdateAvoidArea`异步 IPC。`TYPE_END`只是服务端 for-loop 哨兵，会连续发送多个单类型
事务；每个 type 在客户端独立入 handler、独立去重、独立 callback、独立触发一次 viewport 更新：

```cpp
if (window->UpdateLastAvoidAreaIfChanged(type, *avoidArea)) {
    window->NotifyAvoidAreaChange(avoidArea, type);
    window->UpdateViewportConfig(window->GetRect(), WindowSizeChangeReason::AVOID_AREA_CHANGE);
}
```

来源：`window_scene/session/host/src/scene_session.cpp:3048-3078`、
`window_scene/session/container/src/zidl/session_stage_proxy.cpp:761-789`和
`wm/src/window_session_impl.cpp:7328-7344`。

因此：

- 逐类型全量期间，`lastAvoidAreaMap_`和 ArkUI 收到的完整 map 会逐项收敛，中间态不是原子快照；
- 直接通知不带 rect。它在客户端 task 真正执行时读取 `GetRect()`；若相邻 `UpdateRect` IPC 尚未到达，可能使用
  旧 rect，若 `UpdateRect()`已在 IPC 线程写入 property 但 handler task尚未执行，也可能已经读到新 rect；
- 值未变化时不会有公开 callback，也不会调用 UIContent，所以发送次数、callback 次数和布局次数不能互相替代。

`WindowSessionImpl::UpdateRect()`在投递 handler 前就更新 window rect，见
`wm/src/window_session_impl.cpp:1353-1393`；这正是必须同时观察 IPC接收日志和 handler task Trace 的原因。

## 四类高风险时序

### 1. scale 与 rect 同一帧变化

`SceneSession::UpdateUIParam()`先执行 `UpdateScaleInner()`，其内部先通知 transform，再调用
`HandleLayoutAvoidAreaUpdate(TYPE_END)`，之后才执行 `UpdateRectInner()`，见
`window_scene/session/host/src/scene_session.cpp:10077-10101`和`:9837-9854`。

于是当前实现存在两种发送形态：

```text
RootScene gate == true
  transform -> 多个单类型 avoid IPC -> 新 rect + map IPC

RootScene gate == false
  transform -> 仅标 AVOID_AREA dirty -> 新 rect + map IPC
  -> post-process 识别 dirty window，避免再发一轮
```

前一种可能先出现一组按旧/过渡 rect 计算的直接结果，再由 map 通路收敛；后一种直接以新 rect+map成组下发。
是否真的产生错误区域，要比较服务端计算日志中的 win rect、bar rect、scale和随后 map，不能只看 callback 次数。

### 2. 状态栏/导航栏显隐或矩形变化

SCB Core 先完成所有 session 的 `UpdateUIParam()`；随后 `PostProcessNotifyAvoidArea()`才把 status/navigation bar
的 `VISIBLE/RECT` dirty 路由到 `SceneSessionManager::UpdateAvoidSessionAvoidArea()`，再向订阅窗口发送
`TYPE_SYSTEM`。因此同一 `FlushUIParams`里，应用自己的 rect map 通知在服务端调用顺序上通常早于 producer
触发的单类型通知。

来源：`window_scene/session_manager/src/scene_session_manager.cpp:16613-16669`、`:16887-16899`、
`:14882-14902`。但若应用 rect 未变化，它不会有布局通知，只会收到直接避让区通知。

SceneBoard 的 `notifyNextAvoidRectInfo`只是预写旋转计算输入，不立即向应用发送通知。状态栏属性变化会打印
`notifyNextAvoidRectInfo`并写入 portrait/landscape rect，见
`window_scene_board/product/phonebase/src/main/ets/SceneBoard/statusbar/SCBStatusBar.ets:795-803`、`:1080-1101`；
真正的 app avoid callback仍应沿后续 status-bar session dirty/manager链路确认。

### 3. 键盘 rect、避让区与 occupied-area 同时变化

`FlushUIParams`在所有 session `UpdateUIParam()`之后调用
`KeyboardSession::CalculateOccupiedAreaAfterUIRefresh()`。该函数先
`HandleLayoutAvoidAreaUpdate(TYPE_KEYBOARD)`，再在需要时执行 `ProcessKeyboardOccupiedAreaInfo()`：

```cpp
HandleLayoutAvoidAreaUpdate(AvoidAreaType::TYPE_KEYBOARD);
TLOGD(WmsLogTag::WMS_KEYBOARD, "Keyboard panel rect has changed");
// ...
ProcessKeyboardOccupiedAreaInfo(callingId, false, stateChanged_);
```

来源：`window_scene/session/host/src/keyboard_session.cpp:1158-1197`。

避让区 IPC 与 occupied-area IPC 都是异步通道，客户端又分别投递 handler task；occupied-area task 最终也会
调用 `UpdateViewportConfig(..., OCCUPIED_AREA_CHANGE, avoidAreas)`，见
`wm/src/window_session_impl.cpp:8823-8858`。因此键盘问题必须同时记录：

- `TYPE_KEYBOARD` avoid area；
- `OccupiedAreaChangeInfo`的 rect、textFieldPositionY/textFieldHeight；
- calling window rect和 `WindowSizeChangeReason`；
- ArkUI `KeyboardAvoid`、`DoKeyboardAvoid`和 safe-area布局 Trace。

ArkUI 仓已有 `docs/kb/issues/layout/keyboard-avoid-timing-change.md`，明确记录“已排队 UI 任务”和 KeyboardAvoid
执行顺序变化曾导致显示位置与点击位置不一致。窗口侧定位不能只保留其中一个通知。

### 4. 首次订阅和前后台切换

普通窗口第一个 listener 注册时，服务端 `UpdateSessionAvoidAreaListener()`同步插入订阅集合，并在 WMS
task scheduler 当前线程内触发初始 `UpdateAvoidArea()`；普通窗口随后用 `AVOID_AREA_CHANGE`的
`UpdateRect+map`发送初始状态。IPC、客户端 handler和JS callback仍是异步，所以 `on()`返回不是 callback barrier。

来源：`window_scene/session_manager/src/scene_session_manager.cpp:14816-14839`、`:14905-14931`和
`window_scene/common/src/task_scheduler.cpp:47-59`。隐藏窗口的 dirty保留机制又可能把实际回调延后到前台可见。

UEC 还要经过宿主 ArkUI UI task、`UIExtensionManager`、`ExtensionSession`和 provider handler；中间没有贯穿全链
的序号或专用 area日志。UEC时序按
[避让区文档的 UEC 链路](./immersive-avoid-area.md#uec-特例查询委托给宿主通知由宿主转发)逐跳断点确认。

## ArkUI：收到参数不等于布局完成

### `UVC`日志位于后续 UI/Platform task 之前

`UIContentImpl::UpdateViewportConfigWithAnimation()`先打印完整 viewport、reason、transaction和 avoid-area map：

```cpp
TAG_LOGD(ACE_LAYOUT, "UVC %{public}s, WSCR %{public}d, IRN %{public}d, %{public}s, keyboardInfo is null",
    config.ToString().c_str(), static_cast<uint32_t>(reason), rsTransaction == nullptr,
    stringifiedMap.c_str());
```

来源：`arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:4049-4079`。随后才选择以下执行方式：

- viewport相同且无 transaction：投递 UI task
  `ArkUIUpdateOriginAvoidAreaAndExecuteKeyboardAvoid`，在任务里执行 `ParseAvoidAreasUpdate()`；
- rotation/DPI：`UpdateConfigSync()`；
- 带 transaction：通过默认 Platform task 执行 `ArkUIPromiseViewportConfig`；
- 普通情况：通过默认 Platform task 执行 `ArkUIUpdateViewportConfig`，同类未执行 task可能由
  `UpdateConfigManager`取消并替换。

来源：`arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:4230-4259`、`:4339-4368`和
`arkui_ace_engine/frameworks/core/common/update_config_manager.h:39-98`。任务合并比较 viewport、reason和
transaction，不把 avoid-area map作为 `AceViewportConfig`相等条件，见
`arkui_ace_engine/adapter/ohos/entrance/ace_viewport_config.h:23-55`；但 viewport相同的普通 avoid更新会在前述
提前返回分支单独处理 map。

这里的默认 task type 来自 `UpdateConfigManager::UpdateConfig()`参数默认值
`TaskExecutor::TaskType::PLATFORM`，见
`arkui_ace_engine/frameworks/core/common/update_config_manager.h:39-48`。因此不能把所有 `UVC`后的工作统称为
“UI task”。

### 判断安全区写入与布局完成

后续 UI/Platform task 中 `FlushSafeArea()`更新各 type，值变化时 `SyncSafeArea()`标记页面、overlay和相关节点
dirty：

```cpp
if (safeAreaUpdated) {
    SyncSafeArea(SafeAreaSyncType::SYNC_TYPE_AVOID_AREA);
}
// SyncSafeArea -> SyncPageSafeArea -> MarkDirty(PROPERTY_UPDATE_MEASURE)
```

来源：`arkui_ace_engine/frameworks/core/pipeline_ng/pipeline_context.cpp:2353-2376`、`:3066-3088`。值未变化时
`SafeAreaManager::Update*SafeArea`返回 false，也不会产生对应 update Trace，见
`arkui_ace_engine/frameworks/core/components_ng/manager/safe_area/safe_area_manager.cpp:62-100`。

确认最终布局至少需要在 `UVC`之后看到：

1. 对应的 `SafeAreaManager::UpdateSystemSafeArea/UpdateCutoutSafeArea/UpdateNavSafeArea` Trace，或确认值相等；
2. `PipelineContext::OnSurfaceChanged`（rect变化场景）或 `SyncSafeArea`导致的 dirty；
3. 后续 `UITaskScheduler::FlushTask`和目标节点 Measure/Layout/安全区 Trace；
4. 必要时再确认 RS 显示帧。

`AnimateOnSafeAreaUpdate()`还会先 `FlushUITasks()`完成旧布局，再在动画闭包中 `SyncSafeArea()`并再次
`FlushUITasks()`，见 `arkui_ace_engine/frameworks/core/pipeline_ng/pipeline_context.cpp:6741-6755`。这正是
“callback已经到达，但中间帧仍是旧布局”的一种合法来源。

## 日志与 Trace 关联表

| 阶段 | Hilog/Trace 关键字 | 说明 |
|---|---|---|
| RootScene门禁 | `vsync request times:` (`WMSLayout`) | `> 0`表示直接通知会被转成 dirty；它不是应用布局状态 |
| WMS一帧入口 | `SceneSessionManager::FlushUIParams`、`FlushUIParams found dirty` (`WMSPipeline`) | 确认 dirty在哪一帧被消费 |
| 服务端 rect处理 | `WMS::WindowRectUpdate::FlushUI::UpdateRectInner`、`NotifyServerToUpdateRect` | 确认 session新几何和 reason |
| 服务端发送 map | `WMS::WindowRectUpdate::ServerNotify::NotifyClient`、`[WindowRectUpdate:ServerNotify]` | 同一个点后执行 `GetAllAvoidAreas` |
| manager避让刷新 | `ssm:UpdateAvoidArea:PID:<id>`、`UpdateSessionAvoidAreaListener` | 区分初始订阅、producer和普通窗口刷新 |
| 服务端计算 | `win <id> type ... avoidAreaType ... windowMode ... avoidAreaOption ... return ...` (`WMSImms`) | 判断是否有能力及计算输入 |
| 客户端收到 rect | `[WindowRectUpdate:ClientRecv] UpdateRect`、`WMS::WindowRectUpdate::ClientRecv::UpdateRect` | 此时 property rect已写入，handler可能尚未运行 |
| 客户端执行布局任务 | `UpdateRectForOtherReasonTask`、`WMS_WindowSessionImpl_UpdateRectForOtherReason` | 在这里先 size listener，后 viewport/avoid |
| 客户端 avoid cache/listener | `avoid type ... area ...`、`win <id> api <ver> type <type> area ...` (`WMSImms`) | 前者来自 map遍历，后者只在公开 listener扩散前出现 |
| WMS调用 UIContent | `WindowSessionImpl::UpdateViewportConfig id:` Trace、`Id: ..., reason: ..., viewportRect:` (`WMSLayout`) | 确认传入 UIContent 的 rect/reason |
| ArkUI接参 | `UVC ... WSCR ... IRN ...` (`AceLayout`) | 只代表接参和准备选择后续执行方式，不代表 UI/Platform task完成 |
| ArkUI提前分支 | `UpdateViewportConfig return in advance` | viewport相等，avoid map改走专用 UI task |
| ArkUI直接 listener | `OnAvoidAreaChanged type:`、`ArkUIUpdateOriginAvoidArea` | SceneBoard窗口/UEC转发等直接 listener链 |
| SafeArea写入 | `SafeAreaManager::Update*SafeArea` (`AceSafeArea` Trace) | 只有值变化才出现 |
| 最终布局 | `PipelineContext::OnSurfaceChanged`、`UITaskScheduler::FlushTask`、`AdjustNotExpandNode`、`ExpandSafeAreaFinish` | 从 dirty进入 Measure/Layout并消费 safe area |
| 键盘补充 | `Keyboard panel rect has changed`、`Calling session rect has changed`、`KeyboardAvoid in the UpdateViewportConfig task`、`DoKeyboardAvoid` | 同时对齐 avoid-area与occupied-area |

`WMS::WindowRectUpdate::ServerNotify::NotifyClient`对应代码片段：

```cpp
HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
    "WMS::WindowRectUpdate::ServerNotify::NotifyClient id=%d reason=%u rect=%{public}s",
    persistentId, static_cast<uint32_t>(reason), winRect.ToString().c_str());
```

来源：`window_scene/session/host/src/scene_session.cpp:2052-2067`。

客户端首尾片段：

```cpp
TLOGI_LMT(TEN_SECONDS, RECORD_100_TIMES, WmsLogTag::WMS_LAYOUT,
    "[WindowRectUpdate:ClientRecv] UpdateRect id:%{public}d name:%{public}s, preRect=%{public}s, "
    "newRect=%{public}s, reason:%{public}u displayId:%{public}" PRIu64, ...);

HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
    "WindowSessionImpl::UpdateViewportConfig id:%d [%d,%d,%u,%u] reason:%u orientation:%d hooked:%d", ...);
```

来源：`wm/src/window_session_impl.cpp:1375-1382`、`:2371-2384`。

## 推荐采集和检索

复现时只做一次明确变化，例如状态栏 `show -> hide`、一次旋转、一次 scale/rect切换或一次键盘弹出。日志必须
保留 timestamp、pid、tid、tag和完整 message；同时采集 WMS、SceneBoard/SystemUI和目标应用进程。先记录：

- 目标 app persistentId/windowId、displayId、window type/mode、当前 rect和scale；
- producer session ID及其 visible/rect；
- avoid-area type、四边 rect；
- API version、前后台/interactive状态；
- 是否 UEC及其宿主/provider ID。

对导出的 Hilog：

```powershell
rg -n "FlushUIParams|vsync request times|WindowRectUpdate|UpdateAvoidArea|AvoidAreaChange" captured.log
rg -n "UpdateSessionAvoidAreaListener|avoidAreaType|avoidAreaOption|avoid type|api .* type .* area" captured.log
rg -n "UVC |UpdateViewportConfig return in advance|OnAvoidAreaChanged|SafeAreaManager" captured.log
rg -n "Keyboard panel rect has changed|Calling session rect has changed|KeyboardAvoid|DoKeyboardAvoid" captured.log
```

对 Trace：

```text
SceneSessionManager::FlushUIParams
ssm:UpdateAvoidArea:PID:<persistentId>
WMS::WindowRectUpdate::FlushUI::UpdateRectInner
WMS::WindowRectUpdate::FlushUI::NotifyServerToUpdateRect
WMS::WindowRectUpdate::ServerNotify::NotifyClient
WMS::WindowRectUpdate::ClientRecv::UpdateRect
WMS::WindowRectUpdate::ClientRecv::UpdateRectForOtherReason
WMS::WindowRectUpdate::ClientRecv::UpdateRectForOtherReasonTask
WindowSessionImpl::UpdateViewportConfig
JsWindowListener::OnSizeChange
OnAvoidAreaChanged
PipelineContext::OnSurfaceChanged
SafeAreaManager::UpdateSystemSafeArea
SafeAreaManager::UpdateCutoutSafeArea
SafeAreaManager::UpdateNavSafeArea
UITaskScheduler::FlushTask
AdjustNotExpandNode
ExpandSafeAreaFinish
```

ArkUI详细 viewport Trace需打开 `persist.ace.trace.sync.debug.enabled`；节点安全区 Trace还需
`persist.ace.trace.layout.enabled`和`persist.ace.trace.safeArea.debug.enabled`。开关定义见
`arkui_ace_engine/adapter/ohos/osal/system_properties.cpp:200-218`和
`arkui_ace_engine/frameworks/base/log/ace_trace.h:29-47`。

## 最短定位决策树

1. **先找服务端 `ServerNotify::NotifyClient`。**有则是 map通路；没有而客户端有 area，则查直接通知路径。
2. **看 reason 和 rect。**`AVOID_AREA_CHANGE`且 rect不变仍可能是普通窗口的初始/dirty map刷新。
3. **看 `vsync request times`。**若触发点时 `> 0`，预期先 dirty、后续 flush；若 `<= 0`，预期可直接发送。
4. **查客户端 `UpdateRect`与公开 area日志。**只有前者说明 rect IPC到达；area值不变会被去重。
5. **查 `WindowSessionImpl::UpdateViewportConfig`。**没有则检查 display、rect、UIContent等提前返回。
6. **查 ArkUI `UVC`。**有 UVC无后续 update Trace时，区分“值相等”“UI/Platform task延迟或被替换”
   “context失效”。
7. **查 `UITaskScheduler::FlushTask`和目标节点。**只有这里才能判断 dirty是否真正进入 Measure/Layout。
8. **UEC逐跳排查。**宿主 UVC存在、provider WMS area日志缺失时，进入 UIExtension中间桥断点，不能只查
   `avoidAreaListenerSessionSet_`。

## 症状到判断

| 症状 | 首要判断 | 常见解释 |
|---|---|---|
| 先 avoid callback，后 size callback | 是否为 scale+rect同帧、逐类型直接通知 | 直接 IPC与 map IPC是两条通道；门禁结果会改变发送形态 |
| 先 size callback，后 avoid callback | 是否为同一 `UpdateRect+map` | map通路客户端源码本就先调用 size listener |
| callback到了但画面晚一帧 | UVC后的 UI/Platform task和 `UITaskScheduler::FlushTask` | callback队列与 ArkUI UI/Platform/VSync队列不同 |
| 一次变化收到多个 avoid callback | 是否为 `TYPE_END`逐类型全量 | 每个 type独立 IPC/去重/viewport，属于预期粒度 |
| 先收到一组区域，马上又被另一组纠正 | scale+rect同帧或 direct+map相邻 | 对比两组结果所使用的 rect、scale和 reason |
| 服务端执行刷新但没有 callback | 客户端按 type去重、可见/interactive/enable门禁 | 发送、callback和布局次数不是一一对应 |
| UVC存在但 SafeAreaManager Trace不存在 | 值是否相等、UI/Platform task是否执行 | UVC在选择执行方式前；manager相等值不打 update Trace |
| 首帧错误后稳定 | 初始listener异步、首次 map与后续producer更新 | `on()`返回不是初始状态到达保证 |
| 键盘画面与点击位置不一致 | occupied-area、avoid-area和 UI/Platform task顺序 | 同时追 `KeyboardAvoid`及窗口 rect，不要只看 TYPE_KEYBOARD |

## 当前观测性与测试缺口

- `MarkAvoidAreaAsDirty()`没有记录 persistentId、原请求 type、触发原因或 frame/generation；dirty合并后无法只靠
  Hilog还原被合并的原始请求。
- `SceneSession::UpdateAvoidArea()`成功路径没有携带 id/type/area的专用日志或 Trace；通常只能从 producer计算、
  IPC错误和客户端接收反推。
- `UpdateRect`与`UpdateAvoidArea`没有共同 generation ID；跨进程只能用时间、ID、rect、reason和area关联。
- `UVC`在后续执行方式确定前打印，普通 Platform task取消/替换也没有把 avoid-area摘要写入统一完成日志。
- 当前定向单测覆盖门禁分支、dirty、计算、序列化、客户端cache和去重，但没有建立 scale+rect、direct+map、
  JS callback与ArkUI布局完成之间的跨通道顺序断言。相关入口：
  `window_scene/test/unittest/window_immersive/scene_session_immersive_test.cpp:451-501`、
  `window_scene/test/unittest/window_immersive/scene_session_manager_immersive_test.cpp:317-395`和
  `wm/test/unittest/immersive/window_session_impl_immersive_test.cpp:100-126`。

后续若补 DFX或测试，优先增加 `{persistentId, type, reason, rect, area, generation/frame}`关联字段，并覆盖：

1. scale与rect同帧、门禁 true/false 两条分支；
2. producer rect/visible变化与app rect变化同一 `FlushUIParams`；
3. `TYPE_END`逐类型缓存中间态和最终收敛；
4. UVC入队、task替换、SafeArea写入和下一次布局flush；
5. 键盘 avoid-area、occupied-area与calling-window rect的组合顺序。
