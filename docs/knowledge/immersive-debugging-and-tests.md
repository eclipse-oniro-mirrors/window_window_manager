# 沉浸式窗口：调试、源码检索与回归测试

## 证据快照

本文的日志文本、Trace slice 名称和代码片段在 2026-09-08 基于以下版本核验：

| 仓库 | commit |
|---|---|
| `window_window_manager` | `dd8e1ae076` |
| `window_scene_board` | `7797266343` |
| `arkui_ace_engine` | `37c17d14a8` |

日志字符串和行号比接口名更容易随重构变化。版本变化后应重新执行本文的检索命令，不能只沿用旧日志关键字。

## 快速分诊

如果现象包含“先收到避让区、后收到布局”“callback到了但画面晚一帧”“首帧错误后恢复”或
“scale/旋转/键盘时偶现旧区域”，先读
[避让区通知与布局通知时序定位](./immersive-avoid-area-layout-timing.md)，再按本文查具体日志和断点。

在判断“沉浸式不生效”前，至少记录：

1. `window_manager_use_sceneboard` 和运行时 SceneBoard 是否启用；
2. Phone、Pad、PC 产品形态，以及是否自由多窗；
3. 窗口类型、persistent/window ID、模式、窗口矩形、display ID；
4. 应用 target/API compatible version；
5. `layoutFullScreen`、`isIgnoreSafeArea_`、`enableImmersiveMode_` 和 `isImmersiveLayout()`结果；
6. 各系统栏的 `enable_`、颜色、动画、setting flag 和 owner；
7. SceneBoard real/effective `layoutFullScreen`、顶层会话和强制属性；
8. AvoidArea 类型、四边矩形、可见性参数、缩放和旋转；
9. 若已进入 ArkUI：UIContent 是否存在、`ignoreSafeArea_` 和 SafeAreaManager 当前 inset。

源码中的原生枚举是 `WMS_IMMS`、`WMS_LAYOUT`、`WMS_LAYOUT_PC`、`WMS_KEYBOARD`，设备 hilog 实际 tag
分别是 `WMSImms`、`WMSLayout`、`WMSLayoutPc`、`WMSKeyboard`；映射见
`utils/include/window_manager_hilog.h:47-79` 和 `utils/src/window_manager_hilog.cpp:20-50`。SceneBoard 的
`WinLogDomain.WMS_IMMS`同样输出为 `WMSImms`，见
`window_scene_board/staticcommon/basecommon/windowscene/src/main/ets/utils/WinLog.ts:20-30`、`:59-69`。
ArkUI 下游实际 tag 是 `AceSafeArea`、`AceLayout`，映射见
`arkui_ace_engine/adapter/ohos/osal/log_wrapper.cpp:62-67`、`:128-134`。

## 症状到链路

| 症状 | 首查文档 | 首查状态 |
|---|---|---|
| 调用布局接口后窗口大小变化 | [immersive-window-overview.md](./immersive-window-overview.md) | 统一/分离架构、窗口模式、旧接口 |
| `getImmersiveModeEnabledState` 为 true 但未沉浸 | 总览 | 偏好态、当前模式、`isImmersiveLayout` |
| 内容仍避让或错误覆盖 | [immersive-avoid-area.md](./immersive-avoid-area.md) | `isIgnoreSafeArea_`、API 兼容分支、UIContent、AvoidArea |
| 状态栏/导航区未按请求显隐 | [immersive-system-bars.md](./immersive-system-bars.md) | 有效 SystemBarProperty、owner、SceneBoard 顶层策略 |
| PC 全屏 hover 行为异常 | 总览 + 系统栏 | 主窗口、自由多窗、title/dock hover 状态 |
| 侧滑返回配置不生效 | 总览 | PC 门禁、gestureBackEnabled、本地到 host session 链路 |

## 关键日志与 Trace 总览

### 先理解日志和 Trace 的边界

- `TLOGD`、`WinLog.showDebug`依赖运行时日志级别；没有 Debug 日志不等于链路未执行。
- `TLOGI_LMT`是限频日志。例如普通避让区查询结果只允许在十秒窗口内记录一定次数；高频问题中日志条数不能
  直接当调用次数，见 `wm/src/window_scene_session_impl.cpp:3801-3805`。
- 日志里的 `win`、`windowId`、`persistentId`、ArkUI `instanceId`不是同一个诊断维度。先用同一窗口名和时间戳
  找到原生 window/persistent ID，再关联 SceneBoard persistent ID 与 ArkUI instance ID；不要只按一个数字串全局匹配。
- 当前设置布局沉浸和设置系统栏的函数没有覆盖完整链路的专用 `HITRACE_METER`。它们主要依靠日志、
  `Session::PostTask`生成的 `s:<task-name>`、SceneSessionManager 的 `ssm:<task-name>`，以及后续矩形、viewport、
  safe-area Trace 串联。`Session`和 manager 的统一 task Trace 包装见
  `window_scene/session/host/src/session.cpp:160-170`、`window_scene/common/src/task_scheduler.cpp:47-59`。
- “日志到达”只能证明执行到打点位置。异步任务是否完成、属性是否被去重、产品策略是否覆盖、布局是否真正消费，
  必须继续看下一层日志或 Trace。

### 一页定位矩阵

| 问题 | 首搜日志/Trace needle | 到达该点说明 | 下一步 |
|---|---|---|---|
| 布局沉浸设置未生效 | `SetLayoutFullScreen`、`win [...] status`、`s:OnLayoutFullScreenChange` | 请求到达 Rosen 客户端；server task 出现则已跨到 SceneSession | 再看 `SetIgnoreViewSafeArea`或 API < 10 的 `needAvoid` |
| 偏好态 true、实际不沉浸 | `enableImmersiveMode`、`isLayoutFullScreen`、`SetIgnoreSafeArea` | 偏好、SceneBoard 状态、ArkUI ignore 状态可以分别核对 | 同时核对 PC/自由多窗、窗口模式和 drawable rect |
| 状态栏未隐藏或颜色错误 | `statusBar:`、`owner`、`on prop change`、`update prop, topContainerSession`、`update prop to callbacks` | 可区分 app 属性、owner 仲裁、SceneBoard 选路和最终 UI callback | 找到首个缺失点；颜色与 enable 分字段追踪 |
| 状态栏回调到达但仍显示 | `registerSystemBarPropertyCallbacks`、`Hide status bar`、`isEnable changed` | 属性已经到 Phone SystemUI | 检查 dropdown/banner/keep-show 临时显示门禁 |
| 同步获取避让区为空 | `type ... area`、`avoidAreaOption ... return`、`status bar not visible` | 客户端结果、server 可用性、producer 可见性可以分别确认 | 结合窗口类型/模式/API/option 判断“合法空值”还是断链 |
| 注册后没有避让区通知 | `UpdateSessionAvoidAreaListener`、`not in avoidAreaListenerNodes`、`avoid area update rejected by recent` | 可确认服务端是否订阅以及是否被前台/Recent 门禁拦截 | 再区分布局 map 通路和直接 `UpdateAvoidArea`通路 |
| 收到通知但页面不动 | `OnAvoidAreaChanged`、`UVC`、`SafeAreaManager::Update*SafeArea` | 数据已进入 ArkUI bridge/viewport/manager | 看值是否去重、`SyncSafeArea`和最终节点 Trace |
| UEC 获取或通知为空 | `SafeArea get success`、`OnAvoidAreaChanged`、`ArkUIUpdateOriginAvoidArea` | 初始查询或宿主原始避让区已进入 ArkUI | UEC 中间桥缺少专用日志，应按任务名和断点继续追踪 |

日志中的避让区 type 数字依次是 `0=SYSTEM`、`1=CUTOUT`、`2=SYSTEM_GESTURE`、`3=KEYBOARD`、
`4=NAVIGATION_INDICATOR`、`5=FLOAT_NAVIGATION`，见 `interfaces/innerkits/wm/wm_common.h:1557-1566`。
系统栏 `settingFlag`是位标志：`1=COLOR_SETTING`、`2=ENABLE_SETTING`、`3=ALL_SETTING`、
`4=FOLLOW_SETTING`，见 `interfaces/innerkits/wm/wm_common.h:527-536`。数字必须结合字段和上下文解读，不能把
`settingFlag=4`误判为“enable=true”。

### 布局沉浸与沉浸偏好

Rosen 客户端入口日志同时给出 window ID、窗口名和请求值。系统窗口、模式能力和兼容模式失败也在同一函数附近：

```cpp
TLOGD(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] status %{public}d",
    GetWindowId(), GetWindowName().c_str(), static_cast<int32_t>(status));
if (WindowHelper::IsSystemWindow(GetType())) {
    TLOGI(WmsLogTag::WMS_IMMS, "system window not supported");
    return WMError::WM_OK;
}
```

来源：`wm/src/window_scene_session_impl.cpp:3904-3913`。如果出现首行而没有后续 ArkUI 日志，先检查函数中
`fullscreen window mode not supported`、`compatible mode disable fullscreen`和最终 `failed, win ... errCode ...`，
见 `wm/src/window_scene_session_impl.cpp:3931-3955`。

API compatible version >= 10 时，关键交接点是 UIContent；低版本则是 `WINDOW_FLAG_NEED_AVOID`：

```cpp
if (version >= 10) {
    TLOGI(WmsLogTag::WMS_IMMS, "win %{public}u status %{public}d",
        GetWindowId(), static_cast<int32_t>(status));
    if (auto uiContent = GetUIContentSharedPtr()) {
        uiContent->SetIgnoreViewSafeArea(status);
    }
}
```

来源：`wm/src/window_scene_session_impl.cpp:3858-3874`。低版本分支增删 `WINDOW_FLAG_NEED_AVOID`并调用
`NotifyWindowNeedAvoid(!status)`，见 `wm/src/window_scene_session_impl.cpp:3875-3898`。这里的
`win ... status`两条分支文本相同，单靠该日志
不能判断 API 分支；必须同时记录应用 compatible version，或检查是否出现 ArkUI 的 `SetIgnoreViewSafeArea`。

沉浸偏好有独立日志，但它不等于当前布局已经忽略安全区：

```cpp
TLOGD(WmsLogTag::WMS_IMMS, "id: %{public}u, enable: %{public}u", GetWindowId(), enable);
enableImmersiveMode_ = enable;
hostSession->OnLayoutFullScreenChange(enableImmersiveMode_);
WindowMode mode = GetWindowMode();
if (!windowSystemConfig_.IsPcWindow() || mode == WindowMode::WINDOW_MODE_FULLSCREEN) {
    return SetLayoutFullScreen(enableImmersiveMode_);
}
```

来源：`wm/src/window_scene_session_impl.cpp:8418-8441`。getter 会打印
`enableImmersiveMode=<value>`，而 `isImmersiveLayout()`在普通场景读 `isIgnoreSafeArea_`，在 PC/Pad 自由多窗比较
drawable rect，见 `wm/src/window_scene_session_impl.cpp:8444-8471`。

跨到统一服务端后，`OnLayoutFullScreenChange`运行在 `Session::PostTask`中，因此 Trace 中可搜索
`s:OnLayoutFullScreenChange`；同时日志给出最终投递给 SceneBoard 的布尔值：

```cpp
TLOGNI(WmsLogTag::WMS_LAYOUT_PC, "%{public}s isLayoutFullScreen: %{public}d", where, isLayoutFullScreen);
if (session->onLayoutFullScreenChangeFunc_) {
    session->SetIsLayoutFullScreen(isLayoutFullScreen);
    session->onLayoutFullScreenChangeFunc_(isLayoutFullScreen);
}
```

来源：`window_scene/session/host/src/scene_session.cpp:8736-8750`。SceneBoard 收到后输出
`on IsLayoutFullScreenChange:<status>`并写入 session，见
`window_scene_board/staticcommon/basecommon/windowscene/src/main/ets/scene/session/SCBSceneSession.ts:1217-1220`。

ArkUI 下游应看到两层证据。第一层表明 UIContent 接到请求，第二层表明 SafeAreaManager 的状态确实变化：

```cpp
TAG_LOGI(ACE_SAFE_AREA, "[%{public}s][%{public}s][%{public}d]: SetIgnoreViewSafeArea:%{public}u",
    bundleName_.c_str(), moduleName_.c_str(), instanceId_, ignoreViewSafeArea);
```

随后 UI task 调用 `pipelineContext->SetIgnoreViewSafeArea(ignoreSafeArea)`。来源：
`arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:4392-4411`。

```cpp
ignoreSafeArea_ = value;
TAG_LOGI(ACE_SAFE_AREA, "SetIgnoreSafeArea %{public}d", ignoreSafeArea_);
ACE_SCOPED_TRACE("SafeAreaManager::SetIgnoreSafeArea %d", ignoreSafeArea_);
```

来源：
`arkui_ace_engine/frameworks/core/components_ng/manager/safe_area/safe_area_manager.cpp:239-247`。若只有第一层日志，
检查 UIContent 的 platform version、app/UIExtension 类型门禁以及 UI task 是否执行。

### 系统栏属性和 SceneBoard 策略

系统栏问题要按字段追踪，不能只看 `enable`。owner 写入日志包含 type、owner、四个属性、isolate、四个 flag 和
owner list 大小：

```cpp
TLOGD(WmsLogTag::WMS_IMMS, "win [%{public}u %{public}s] type %{public}u owner %{public}u "
    "prop [%{public}u %{public}x %{public}x %{public}u, %{public}u] "
    "flag [%{public}u%{public}u%{public}u%{public}u] size %{public}u",
    GetWindowId(), GetWindowName().c_str(), static_cast<uint32_t>(type), static_cast<uint32_t>(owner),
    prop.enable_, prop.backgroundColor_, prop.contentColor_, prop.enableAnimation_, prop.isolate_,
    prop.flag_.enableFlag, prop.flag_.backgroundColorFlag,
    prop.flag_.contentColorFlag, prop.flag_.enableAnimationFlag, static_cast<uint32_t>(ownPropList.size()));
```

来源：`wm/src/window_scene_session_impl.cpp:4313-4345`。仲裁后的最终 active property 会打印
`prop [enable backgroundColor contentColor animation settingFlag]`，见
`wm/src/window_scene_session_impl.cpp:4377-4425`。若 owner 日志正确而最终 property 不符，问题在字段级优先级或
isolate 规则，不在 SceneBoard。

向服务端发送 status bar 属性时，客户端限频日志的字段顺序是
`windowId, enable, backgroundColor, contentColor, enableAnimation, settingFlag`：

```cpp
TLOGI_LMT(TEN_SECONDS, RECORD_100_TIMES, WmsLogTag::WMS_IMMS,
    "win %{public}u statusBar: %{public}u %{public}x %{public}x %{public}u %{public}u",
    GetWindowId(), property.enable_, property.backgroundColor_, property.contentColor_,
    property.enableAnimation_, property.settingFlag_);
UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_STATUS_PROPS);
```

来源：`wm/src/window_scene_session_impl.cpp:4031-4045`。服务端收到后再次输出完整 property，并触发
`onSystemBarPropertyChange_`，见 `window_scene/session/host/src/scene_session.cpp:2984-2999`；属性 action 分流还会输出
`type ... enable ...`，见 `window_scene/session/host/src/scene_session.cpp:7968-7975`。

SceneBoard 的关键三段日志是：

1. `on prop change, prop:[...]`：native-to-JS 事件已到 `SCBSceneSession`，见
   `window_scene_board/staticcommon/basecommon/windowscene/src/main/ets/scene/session/SCBSceneSession.ts:1297-1319`；
2. `update prop, topContainerSession: [...]`：中央策略选中了哪个顶层会话，见
   `window_scene_board/staticcommon/basecommon/windowscene/src/main/ets/scene/session/SCBSceneSessionManager.ts:6897-6927`；
3. `update prop to callbacks...`、`same as last`或`failed`：最终属性被发布、去重或因无 callback 被丢弃，见
   `window_scene_board/staticcommon/basecommon/windowscene/src/main/ets/scene/session/SCBSceneSessionManager.ts:6797-6826`。

Phone 状态栏消费者还存在临时显示门禁。只有满足“当前显示、无下拉、无特定 banner、无需 keep showing”才执行隐藏：

```ts
} else if (
  this.viewController.isShowing() &&
    !viewMgrPolicy.isViewShowing(ViewType.DROPDOWN) &&
    !bannerMgr.isBannerPanelType(BannerPanelType.TYPE_LIVE_LIST) &&
    !this.enableStatusBarKeepShowing()
) {
  log.showInfo('Hide status bar');
  scbStatusBarVM.onIsBarShow(false, property.enableAnimation);
}
```

来源：
`window_scene_board/product/phonebase/src/main/ets/SceneBoard/statusbar/SCBStatusBar.ets:765-803`。因此出现
`registerSystemBarPropertyCallbacks, enable:false`但没有 `Hide status bar`时，应检查这些 SystemUI 状态，而不是
回退修改 app 的系统栏属性。

### 避让区查询、刷新和通知

同步查询首先看 Rosen 客户端结果日志：

```cpp
avoidArea = hostSession->GetAvoidAreaByType(type, sessionRect, apiVersion);
getAvoidAreaCnt_++;
TLOGI_LMT(TEN_SECONDS, RECORD_100_TIMES, WmsLogTag::WMS_IMMS,
    "win %{public}u type %{public}d times %{public}u area %{public}s",
    GetWindowId(), type, getAvoidAreaCnt_.load(), avoidArea.ToString().c_str());
```

来源：`wm/src/window_scene_session_impl.cpp:3781-3806`。同一段还会打印 system window API < 18 和
float-navigation 未启用的门禁。ignoring-visibility 查询使用非限频的
`win [id name] type ... area ...`，见 `wm/src/window_scene_session_impl.cpp:3809-3830`。

服务端可用性判断的总日志最关键，它一次给出 window type、avoid-area type、window mode、option 和返回值：

```cpp
TLOGD(WmsLogTag::WMS_IMMS, "win %{public}d type %{public}u avoidAreaType %{public}u "
    "windowMode %{public}u avoidAreaOption %{public}u, return %{public}d",
    GetPersistentId(), static_cast<uint32_t>(winType), static_cast<uint32_t>(type),
    static_cast<uint32_t>(winMode), GetSessionProperty()->GetAvoidAreaOption(), isAvailable);
```

来源：`window_scene/session/host/src/scene_session.cpp:3597-3623`。附近的
`not support`、`parent session is nullptr`、`rect mismatch`、`no calcu cutout in split`能进一步解释 false，见
`window_scene/session/host/src/scene_session.cpp:3547-3609`。

producer 的典型诊断日志包括：

- system：`win ... type ... flag ...`表示旧兼容 `NEED_AVOID`导致返回空；`status bar not visible`表示可见性门禁；
  `constantly isVisible`可确认常驻显示修正，见 `window_scene/session/host/src/scene_session.cpp:3223-3234`、
  `:3283-3303`；
- keyboard：`isSystemKeyboard/state/gravity`、`Keyboard avoid area needs to be empty in floating mode`，见
  `window_scene/session/host/src/scene_session.cpp:3310-3337`；
- cutout/旋转：`There is no cutout info`、`There is no cutout area`、`rect ... cutout ...`，见
  `window_scene/session/host/src/scene_session.cpp:4056-4078`；
- navigation indicator：`window mode pip return`以及公共计算日志 `win ... type ... rect ... bar ...`，见
  `window_scene/session/host/src/scene_session.cpp:3429-3447`、`:4051-4053`。

监听注册后，服务端日志 `UpdateSessionAvoidAreaListener win <id> haveListener <0|1>`是订阅成立的第一证据；
PID 不匹配、session 不存在会在这里明确报错：

```cpp
TLOGNI(WmsLogTag::WMS_IMMS, "%{public}s win %{public}d haveListener %{public}d",
    where, persistentId, haveListener);
// ... permission/session checks ...
if (haveListener) {
    avoidAreaListenerSessionSet_.insert(persistentId);
    UpdateAvoidArea(persistentId);
}
```

来源：`window_scene/session_manager/src/scene_session_manager.cpp:14816-14839`。

普通窗口刷新若被过滤，会出现 `session is nullptr`、`isVisible ... sessionState ...`或
`not in avoidAreaListenerNodes`；通过后以 `AVOID_AREA_CHANGE`触发携带全类型 map 的窗口布局通知：

```cpp
sceneSession->UpdateSizeChangeReason(SizeChangeReason::AVOID_AREA_CHANGE);
sceneSession->NotifyClientToUpdateRect("AvoidAreaChange", std::nullopt, nullptr);
```

来源：`window_scene/session_manager/src/scene_session_manager.cpp:14905-14931`。manager task 的 Trace 名为
`ssm:UpdateAvoidArea:PID:<persistentId>`，由 task 名和统一 wrapper 组合，见
`window_scene/session_manager/src/scene_session_manager.cpp:14970-14983`、
`window_scene/common/src/task_scheduler.cpp:47-59`。

跟随布局的 map 通路有一组可首尾关联的日志与 Trace：

```cpp
HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
    "WMS::WindowRectUpdate::ServerNotify::NotifyClient id=%d reason=%u rect=%{public}s",
    persistentId, static_cast<uint32_t>(reason), winRect.ToString().c_str());
// ...
GetAllAvoidAreas(avoidAreas);
Session::UpdateRectWithLayoutInfo(winRect, reason, updateReason, transaction, avoidAreas);
```

来源：`window_scene/session/host/src/scene_session.cpp:2052-2104`。客户端对应点是：

```cpp
HITRACE_METER_FMT(HITRACE_TAG_WINDOW_MANAGER,
    "WMS::WindowRectUpdate::ClientRecv::UpdateRect id=%d rect=%s reason=%u",
    GetPersistentId(), wmRect.ToString().c_str(), wmReason);
```

来源：`wm/src/window_session_impl.cpp:1358-1382`。继续搜索
`WMS::WindowRectUpdate::ClientRecv::UpdateRectForOtherReasonTask`和
`WindowSessionImpl::UpdateViewportConfig`，可确认客户端通知 UIContent，见
`wm/src/window_session_impl.cpp:1655-1674`、`:2371-2384`。

逐个全量通知和单个通知最终都可落到客户端 `UpdateAvoidArea()`。只有值发生变化时才回调 listener 并刷新
viewport，所以“服务端发送了、客户端没回调”首先检查按 type 去重：

```cpp
if (window->UpdateLastAvoidAreaIfChanged(type, *avoidArea)) {
    window->NotifyAvoidAreaChange(avoidArea, type);
    window->UpdateViewportConfig(window->GetRect(), WindowSizeChangeReason::AVOID_AREA_CHANGE);
}
```

来源：`wm/src/window_session_impl.cpp:7306-7344`。公开 listener 前的最终日志为
`win <id> api <version> type <type> area <area>`；system window/system-host UEC API < 18 则打印
`not supported`并向 listener 交付空值，见 `wm/src/window_session_impl.cpp:7269-7292`。

status bar UI 生产下一帧几何时，应同时看到 SceneBoard 的 `notifyNextAvoidRectInfo`以及 native manager 的
`type ... portraitRect ... landspaceRect ... displayId ...`，两端代码见
`window_scene_board/product/phonebase/src/main/ets/SceneBoard/statusbar/SCBStatusBar.ets:1080-1100`、
`window_scene/session_manager/src/scene_session_manager.cpp:15178-15186`。AI navigation 和 float navigation 还会输出
visible 与 rect，并仅在值变化时遍历 listener，见
`window_scene/session_manager/src/scene_session_manager.cpp:15145-15175`、`:15203-15232`。

### ArkUI 接收、存储和最终布局 Trace

直接通知到达 ArkUI 时，同时生成 `OnAvoidAreaChanged ...` Trace 和 `AceLayout`日志；字段包含 type、四边区域、
keyboard rect、文本框位置/高度以及 instance ID：

```cpp
ACE_SCOPED_TRACE("OnAvoidAreaChanged type: %d, value: %s, instanceId: %d, "
    "keyboardInfo is null", type, avoidArea.ToString().c_str(), instanceId_);
TAG_LOGD(ACE_LAYOUT, "OnAvoidAreaChanged type: %{public}d, value: %{public}s; instanceId: %{public}d, "
    "keyboardInfo is null", type, avoidArea.ToString().c_str(), instanceId_);
```

来源：`arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:663-727`。初始化阶段则看
`InitializeSafeArea systemInsets:..., cutoutInsets:..., navInsets:..., floatNavInsets:...`，见
`arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:2886-2904`。

布局 map 通路进入 UIContent 后，`AceLayout`的 `UVC`日志和可选 Trace 会带 viewport、
windowSizeChangeReason、事务是否为空、完整 avoid-area map 和 keyboard rect：

```cpp
TAG_LOGD(ACE_LAYOUT, "UVC %{public}s, WSCR %{public}d, IRN %{public}d, %{public}s, keyboardRect %{public}s",
    config.ToString().c_str(), static_cast<uint32_t>(reason), rsTransaction == nullptr,
    stringifiedMap.c_str(), keyboardRect.ToString().c_str());
```

来源：`arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:4055-4079`。

SafeAreaManager 只在值变化时生成 `SafeAreaManager::UpdateSystemSafeArea`、
`UpdateCutoutSafeArea`、`UpdateNavSafeArea` Trace；相等值会直接返回 false，见
`arkui_ace_engine/frameworks/core/components_ng/manager/safe_area/safe_area_manager.cpp:62-100`。所以
`OnAvoidAreaChanged`存在但 manager Trace 不出现，可能只是新旧 inset 相同，并不必然是断链。

最终节点覆盖错误时，可开启 safe-area 调试 Trace，搜索：

- `AdjustNotExpandNode[tag][self][parent][key][paintRectRect]`；
- `ExpandSafeAreaFinish[tag][self][parent][key][opt][paintRectRect][selfAdjust]`。

对应打点见 `arkui_ace_engine/frameworks/core/components_ng/layout/layout_wrapper.cpp:304-321`、`:376-390`。
`ACE_LAYOUT_SCOPED_TRACE`由 `persist.ace.trace.layout.enabled`控制，定义与参数监听见
`arkui_ace_engine/frameworks/base/log/ace_trace.h:29-47`、
`arkui_ace_engine/adapter/ohos/entrance/ace_container.cpp:117-119`、`:4745-4755`；上述节点 Trace 还要求
`persist.ace.trace.safeArea.debug.enabled=true`，参数读取见
`arkui_ace_engine/adapter/ohos/osal/system_properties.cpp:215-218`。`UpdateViewportConfig`的详细 scoped Trace 还受
`persist.ace.trace.sync.debug.enabled`控制，见
`arkui_ace_engine/adapter/ohos/osal/system_properties.cpp:200-203`和
`arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:4059-4075`。

### UEC 定位补充

UEC 的初始查询可从宿主 ArkUI 日志 `SafeArea get success, type:<type>, insets:<insets>`确认，见
`arkui_ace_engine/adapter/ohos/entrance/ace_container.cpp:3497-3517`。宿主收到更新后，
`OnAvoidAreaChanged`或 `UVC`能够证明原始区域已经到达宿主 ArkUI；任务名 `ArkUIUpdateOriginAvoidArea`表明它被投递到
UI 线程，见 `arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:715-727`。

`UIExtensionManager::TransferOriginAvoidArea`、`UIExtensionPattern::DispatchOriginAvoidArea`、
`SessionWrapperImpl::NotifyOriginAvoidArea`当前没有携带 area/type 的专用日志或 Trace。因此宿主日志存在而 provider 的
`wm/src/window_session_impl.cpp:7284-7290`日志不存在时，不能凭日志精确区分三段中哪一段丢失；应按
[避让区文档的 UEC 链路](./immersive-avoid-area.md#uec-特例查询委托给宿主通知由宿主转发)设置断点，并同时检查
provider session 生命周期。这是当前观测性缺口，不应误写成 provider 没有注册 listener。

### 推荐检索词

对导出的 hilog 文本先按实际 tag 和当前窗口 ID 缩小范围：

```powershell
rg -n "WMSImms|WMSLayout|WMSLayoutPc|WMSKeyboard|AceSafeArea|AceLayout" captured.log
rg -n "SetLayoutFullScreen|OnLayoutFullScreenChange|SetIgnoreViewSafeArea|SetIgnoreSafeArea" captured.log
rg -n "statusBar:|owner|on prop change|topContainerSession|update prop to callbacks|Hide status bar" captured.log
rg -n "GetAvoidArea|avoidAreaOption|UpdateSessionAvoidAreaListener|AvoidAreaChange|OnAvoidAreaChanged|UVC" captured.log
rg -n "FlushUIParams|vsync request times|UpdateRectForOtherReasonTask|UpdateViewportConfig return in advance" captured.log
```

在 Perfetto/SmartPerf 等 Trace 查看器中按 slice 名搜索：

```text
s:OnLayoutFullScreenChange
ssm:UpdateAvoidArea:PID:<persistentId>
SceneSessionManager::FlushUIParams
WMS::WindowRectUpdate::FlushUI::UpdateRectInner
WMS::WindowRectUpdate::FlushUI::NotifyServerToUpdateRect
WMS::WindowRectUpdate::ServerNotify::NotifyClient
WMS::WindowRectUpdate::ClientRecv::UpdateRect
WMS::WindowRectUpdate::ClientRecv::UpdateRectForOtherReason
WMS::WindowRectUpdate::ClientRecv::UpdateRectForOtherReasonTask
WindowSessionImpl::UpdateViewportConfig
PipelineContext::OnSurfaceChanged
OnAvoidAreaChanged
SafeAreaManager::SetIgnoreSafeArea
SafeAreaManager::UpdateSystemSafeArea
SafeAreaManager::UpdateCutoutSafeArea
SafeAreaManager::UpdateNavSafeArea
UITaskScheduler::FlushTask
AdjustNotExpandNode
ExpandSafeAreaFinish
```

建议以一次明确的状态翻转为采集窗口：设置前记录窗口信息，执行一次 `false -> true`或显隐切换，等待 UI 稳定后停止。
连续高频切换会同时触发日志限频、属性去重和异步任务合并，反而不利于判断首个断点。

## 断点链

### 布局或状态接口无效

1. `JsWindow::OnSetWindowLayoutFullScreen`、`OnSetImmersiveModeEnabledState` 或 `OnIsImmersiveLayout`；
2. `WindowSceneSessionImpl::SetLayoutFullScreen`、`SetLayoutFullScreenByApiVersion`；
3. `UIContent::SetIgnoreViewSafeArea` 是否被调用；
4. 旧应用分支则看 `WINDOW_FLAG_NEED_AVOID` 和 `NotifyWindowNeedAvoid`；
5. 需要核对产品策略时，再看 `SceneSession::OnLayoutFullScreenChange` 和 SceneBoard `layoutFullScreenChange`。

### 系统栏仍显示或样式错误

1. `JsWindow::OnSetSpecificSystemBarEnabled` / `OnSetSystemBarProperties` / `OnSetStatusBarColor`；
2. `WindowSceneSessionImpl::UpdateSystemBarProperties`；
3. `SetOwnSystemBarProperty` 和 `GetCurrentActiveSystemBarProperty`；
4. `NotifySpecificWindowSessionProperty`；
5. `SceneSession::HandleSpecificSystemBarProperty`；
6. `JsSceneSession::OnSystemBarPropertyChange`；
7. `SCBSceneSessionManager::updateSystemBarProperty` 的实际产品分支；
8. Phone/PC 对应状态栏或导航 UI callback。

### 避让区错误

1. `WindowSceneSessionImpl::GetAvoidAreaByType` 或 ignoring-visibility 变体；
2. `SceneSession::CheckGetAvoidAreaAvailable`；
3. `SceneSession::GetAvoidAreaByTypeInner` 和具体 producer；
4. `SceneSessionManager::UpdateAvoidArea`；
5. `WindowSessionImpl` listener；
6. 若窗口结果正确但页面错误，再进入 `UIContentImpl::AvoidAreaChangedListener`；
7. `PipelineContext::Update*SafeArea`、`SafeAreaManager`，最后才是目标节点布局。

## 快速检索

从 OpenHarmony 工作区根目录执行：

```powershell
rg -n "SetLayoutFullScreen|SetImmersiveModeEnabledState|IsImmersiveLayout|SetFullScreen" `
  foundation/window/window_window_manager/wm foundation/window/window_window_manager/interfaces

rg -n "SetSystemBarProperty|SetOwnSystemBarProperty|GetCurrentActiveSystemBarProperty" `
  foundation/window/window_window_manager/wm foundation/window/window_window_manager/window_scene

rg -n "GetAvoidAreaByType|UpdateAvoidArea|avoidAreaChange|SetAvoidAreaOption" `
  foundation/window/window_window_manager/wm foundation/window/window_window_manager/window_scene

rg -n "layoutFullScreenChange|systemBarPropertyChange|titleAndDockHoverShowChange|updateSystemBarProperty" `
  foundation/window/window_scene_board/staticcommon

rg -n "SetIgnoreViewSafeArea|AvoidAreaChangedListener|UpdateSystemSafeArea|UpdateNavSafeArea" `
  foundation/arkui/arkui_ace_engine/adapter/ohos foundation/arkui/arkui_ace_engine/frameworks/core
```

上面的命令按本工作区目录给出；在其他 checkout 中执行前应按实际仓库位置调整。禁止因为一次搜索无结果就断言代码不存在。

## WindowManager 定向测试

当前源码有两个沉浸式聚合目标：

| GN 目标 | BUILD 文件 | 当前直接 sources |
|---|---|---|
| `wm_immersive_test` | `wm/test/unittest/immersive/BUILD.gn:21` | 同目录 `window_manager_immersive_test.cpp`、`window_scene_session_impl_immersive_test.cpp`、`window_session_impl_immersive_test.cpp` |
| `window_scene_immersive_test` | `window_scene/test/unittest/window_immersive/BUILD.gn:25` | 同目录 `scene_session_immersive_test.cpp`、`scene_session_manager_immersive_test.cpp` |

其他高价值回归入口：

- `wm/test/unittest/window_scene_session_impl_test.cpp`：布局全屏、旧全屏、沉浸偏好、查询、gesture 和 PC hover 门禁；
- `wm/test/unittest/window_impl_test.cpp`及相关分离架构测试：旧 WMS 行为；
- `window_scene/test/unittest/session_proxy_test.cpp`、`window_scene/test/unittest/session_stub_test.cpp`：IPC 序列化与错误路径；
- `test/systemtest/wms/window_immersive_test.cpp`、`test/systemtest/wms/window_split_immersive_test.cpp`：系统行为；
- `wmserver/test/unittest/avoid_area_controller_test.cpp`：分离架构避让区。

从 OpenHarmony 根目录解析 BUILD 中的准确 GN label 后再执行 `build.sh`；不要根据短目标名猜输出路径。
如果当前工作区不含完整构建环境，应明确记录“未运行二进制测试”，不能把静态搜索当作测试通过。

## SceneBoard 与 ArkUI 回归入口

SceneBoard 核心沉浸策略主要混在 session-manager 和产品 SystemUI 测试中，不是
`feature/immersivekeyguard`：

- `staticcommon/basecommon/windowscene/src/ohosTest/ets/test/ets/WindowScene/scene/session/SCBSceneSession.test.ets`；
- `staticcommon/basecommon/windowscene/src/ohosTest/ets/test/ets/WindowScene/scene/session/SCBSceneSessionManager6.test.ets`；
- `staticcommon/basecommon/windowscene/src/ohosTest/ets/test/ets/WindowScene/scene/session/SCBSceneSessionManager8.test.ets`；
- Phone 状态栏与导航栏测试；
- PC 产品的 gesture bar、Dock/标题栏 hover 测试。

进入 ArkUI 前先查其 KB，再按问题选择：

- `arkui_ace_engine/test/unittest/interfaces/ui_content_test.cpp`；
- `arkui_ace_engine/test/unittest/core/manager/safe_area_manager_test_ng.cpp` 与
  `arkui_ace_engine/test/unittest/core/manager/safe_area_manager_test_ng_part_two.cpp`；
- `arkui_ace_engine/test/unittest/core/layout/safe_area/`；
- UIExtension 传播相关测试。

## 知识维护检查表

行为或路径变化时：

1. 更新最小相关知识文件，不把所有内容堆入总览；
2. 刷新三仓提交快照；
3. 对每个行为结论重新打开实现和邻近测试；
4. 保持窗口模式、布局安全区、系统栏、避让区和产品策略五维分离；
5. 不确定的产品结论标记为“推测”；
6. 检查本文及 `AGENTS.md` 的相对链接；
7. 用 `rg` 验证接口拼写、源文件路径和测试目标仍存在。
