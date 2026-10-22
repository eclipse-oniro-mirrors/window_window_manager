# 兼容模式（Compatible Mode）C++ 侧业务知识

> 定位：**window_scene 兼容模式子域知识文档**，非仓库根级指令文件。仓库级通用约束（代码风格、测试规范、Git 提交规则、架构开关）见仓库根 `AGENTS.md` / `CLAUDE.md` 与 `docs/CodeStyle.md` / `docs/Testing.md`；本文只补充兼容模式子域的路由、边界与验证要求，与仓库根文件冲突时以仓库根文件为准。
> 面向读者：后续维护/排障/二次开发的 agent 与工程师。
> 本文基于当前仓库代码梳理，**所有行号仅为定位参考**，以函数名/符号搜索为准。
> 统一日志标签：`WMS_COMPAT`（部分早期代码用 `WMS_LAYOUT`/`DMS`，grep 时注意）。

## 1. 概述

兼容模式用于让按小屏（手机）设计的应用在大屏设备（PC、折叠屏展开态、大屏平板）上正常运行。C++ 侧（window_scene，统一架构 Sceneboard）由三条主链路组成：

| 链路 | 作用 | NAPI 入口（JS 侧由 SCB 调用） | 所在文件 |
|------|------|-------------------------------|----------|
| 窗口 hook | WMS 按比例缩放窗口逻辑尺寸，app 感知被 hook 的窗口 rect | `session.updateHookWindowInfo()` | `window_scene\interfaces\kits\napi\scene_session_manager\js_scene_session.cpp` |
| 屏幕 hook | 给指定 uid 的 app 提供"虚拟显示"（宽/高/密度/旋转/方向均被 hook），app 读 display 信息时拿到 hook 后的值 | `sceneSessionManager.updateAppHookDisplayInfo()` / `updateDisplayHookInfo()` | `window_scene\interfaces\kits\napi\scene_session_manager\js_scene_session_manager.cpp` |
| 分栏 | 通知 ArkUI（app 侧）开启强制分栏布局 | `session.setForceSplitEnable()` | `window_scene\interfaces\kits\napi\scene_session_manager\js_scene_session.cpp` |

### 任务路由（Where to look）

按任务类型先读对应小节，再进代码：

| 任务类型 | 先读 | 关键代码入口 |
|----------|------|--------------|
| 窗口 hook 排障/改动（app 拿到的窗口 rect 不对、比例不生效） | §3 + §9 | `js_scene_session.cpp:7792` → `main_session.cpp:742` |
| 屏幕 hook 排障/改动（app 读到的 display 宽高/密度/旋转不对） | §4 + §9 | `js_scene_session_manager.cpp:5266` → `scene_session_manager.cpp:19195` |
| 分栏排障/改动（ArkUI 分栏不生效、selectMode 不对） | §5 + §9 | `js_scene_session.cpp:7749` → `main_session.cpp:776` |
| app↔WMS IPC 协议变更（字段增删） | §3.3 / §5.3 + §10 | `session_stage_proxy.cpp` / `session_stage_stub.cpp` |
| 预启动/游戏预热场景 hook 异常 | §4.4 | `session.cpp:6488-6549` |
| app 兼容能力/避让区异常（沉浸式适配不生效、避让区高度不对、能力上报丢失） | §6.2 + §9.11 | `window_session_property.h:778` → `scene_session.cpp:3512` |
| PC 应用在 Pad 上兼容（WMS_PC 标签） | §6 | `session.cpp:4443` 起 |
| 新需求设计/架构影响评估 | §11 | — |
| 写/改单测 | §8 + 仓库根 `docs/Testing.md` | `window_scene\test\unittest\BUILD.gn` |
| 提交代码 | 仓库根 `AGENTS.md`（CodeStyle/commit 规则） | — |

术语路由：日志标签 `WMS_LAYOUT`/`WMS_PC`/`DMS` 是兼容模式相关代码的历史/交叉标签（见各节标注）；`isAbilityHook`/`GetIsAbilityHookOff` 与本文 hook 机制无关（§9.9）；`HookInfo`（wm_common.h，WMS 侧）与 `DMHookInfo`（dm_common.h，DM 侧）是同族结构的两份定义。

**改动前门禁**：开始任何代码改动前，先在答复中陈述三项——① 任务类别（上表哪一行）；② 已读的小节/文档（本文 §x、仓库根 AGENTS.md 等）；③ 命中的 §10 约束（Do not / Ask before 逐条对照）。未完成陈述不得动手改代码。

### 总览图

```
 JS (SCB/ArkUI 业务层)
 │
 │ ①窗口hook                ②屏幕hook                          ③分栏
 │ updateHookWindowInfo     updateAppHookDisplayInfo /          setForceSplitEnable
 ▼                          updateDisplayHookInfo               │
 JsSceneSession             JsSceneSessionManager               JsSceneSession
 (js_scene_session.cpp)     (js_scene_session_manager.cpp)      (js_scene_session.cpp)
 │                          │                                   │
 ▼                          ▼                                   ▼
 MainSession::              SceneSessionManager::               MainSession::
 UpdateHookWindowInfo       UpdateAppHookDisplayInfo /          SetForceSplitEnable
 (main_session.cpp)         UpdateDisplayHookInfo               (main_session.cpp)
 │                          │                                   │
 │ 存property+diff          ▼                                   ├─ setSelectModeCallback_
 │ 变化才下发               组装 DMHookInfo                     │   → SSM::SetSelectMode (全局selectMode_)
 ▼                          ▼                                   ├─ property->SetForceSplitEnable
 SessionStageProxy::        ScreenSessionManagerClient::        ▼
 UpdateAppHookWindowInfo    UpdateDisplayHookInfo            SessionStageProxy::
 (IPC→app)                  (IPC→DMServer)                  SetForceSplitEnable
                            │                                   (IPC→app, ArkUI分栏)
                            ▼
                            ScreenSessionManager::
                            UpdateDisplayHookInfo (DMServer)
                            displayHookMap_[uid] + 按uid通知display change
```

## 2. 关键数据结构

| 结构 | 定义位置 | 字段 | 说明 |
|------|----------|------|------|
| `HookWindowInfo` | `interfaces\innerkits\wm\wm_common.h:810` | `enableHookWindow`(bool)、`widthHookRatio`(float, 默认1.0)、`notifyWindowChange`(bool)、`drawableRectHook`(bool) | 窗口 hook 配置，Parcelable。IPC 字段序：bool→float→bool→bool |
| `HookInfo` | `interfaces\innerkits\wm\wm_common.h:1420` | `width_`、`height_`、`density_`、`rotation_`、`enableHookRotation_`、`displayOrientation_`、`enableHookDisplayOrientation_`、`actualRect_`(Rect) | 屏幕 hook 配置（WMS 侧），非 Parcelable（跨进程时转 DMHookInfo） |
| `DMHookInfo` | `interfaces\innerkits\dm\dm_common.h:710` | HookInfo 全部字段 + `isFullScreenInForceSplit_`(bool) | 屏幕 hook 配置（DM 侧），多了分栏全屏标记 |
| `SelectMode` | `interfaces\innerkits\wm\wm_common.h:723` | `WIDE_MODE=0`、`SQUARE_MODE=1`、`INVALID_MODE=2` | 分栏显示模式 |
| `AppForceLandscapeConfig` | `window_scene\common\include\window_session_property.h:920` | `configJsonStr_`、`isRouter_`、`containsConfig_`、`configEnable_` | 按 bundleName 下发的强制横屏/分栏 JSON 配置 |
| `CompatibleModeProperty` | `window_scene\common\include\window_session_property.h:778` | 16 个 bool（`isAdaptToImmersive_`/`EventMapping`/`ProportionalScale`/`BackButton`/`DragScale`/`SubWindow`/`SimulationScale`/`CompatibleDevice`、`disableDragResize_`/`ResizeWithDpi_`/`FullScreen_`/`Split_`/`WindowLimit_`/`DecorFullscreen_`、`isFullScreenStart_`/`isSupportRotateFullScreen_`）+ `realTimeSwitchInfo_` | app 兼容能力声明与行为限制，Parcelable，随 property 跨进程传递（链路见 §6.2） |
| `CompatibleStyleMode` | `interfaces\innerkits\wm\wm_common.h:4345` | LANDSCAPE_DEFAULT/18_9/1_1/2_3/SPLIT/3_2/4_3/16_9/VERTICAL_FULL_SCALE | 兼容显示比例模式（关联机制，见 §6） |

## 3. 链路一：窗口 hook（UpdateHookWindowInfo）

### 3.1 入口（NAPI）

- 绑定：`js_scene_session.cpp:733`，`BindNativeFunction(env, objValue, "updateHookWindowInfo", ...)` → 静态壳 `JsSceneSession::UpdateHookWindowInfo`（`:3309`）→ 实体 `OnUpdateHookWindowInfo`（`:7792`）。
- JS 参数（单个 object，解析在 `js_scene_utils.cpp:1209 ConvertHookWindowInfoFromJs`）：
  - `enableHookWindow`: bool，必填
  - `widthHookRatio`: number，必填（落为 float）
  - `notifyWindowChange`: bool，可选
  - `drawableRectHook`: bool，可选
- 取 `weakSession_.promote()` 得到服务端 session（实际为 `MainSession`，主窗口），调 `session->UpdateHookWindowInfo(hookWindowInfo)`。

### 3.2 MainSession 处理（`window_scene\session\host\src\main_session.cpp:742`）

```
WSError MainSession::UpdateHookWindowInfo(const HookWindowInfo& hookWindowInfo)
```

1. 参数校验：`widthHookRatio < 0.0f` → `WS_ERROR_INVALID_PARAM`。
2. `property->GetHookWindowInfo()` 取旧值 `preInfo`；组装 `newInfo`（**注意：落库时 `notifyWindowChange` 强制写 false**，仅 `enableHookWindow/widthHookRatio/drawableRectHook` 透传）后 `property->SetHookWindowInfo(newInfo)`。
3. diff 判断（任一变化才下发）：`enableHookWindow` 变化 / `widthHookRatio` 变化（`MathHelper::NearZero` 容差比较）/ `drawableRectHook` 变化。
4. 有变化则调 `MainSession::UpdateAppHookWindowInfo`（`main_session.cpp:725`）。

`UpdateAppHookWindowInfo`：
- `PostTask`（session 内部任务线程，task 名 `"UpdateAppHookWindowInfo"`，弱引用提升防悬挂）。
- `sessionStage_->UpdateAppHookWindowInfo(hookWindowInfo)` —— **下发给 app 的仍是原始 hookWindowInfo（含 JS 传入的 notifyWindowChange）**，与 property 中存储值不同，这是刻意行为。

### 3.3 IPC 下发（WMS → app）

- Proxy：`window_scene\session\container\src\zidl\session_stage_proxy.cpp:2852`，`SessionStageProxy::UpdateAppHookWindowInfo`：
  - code：`SessionStageInterfaceCode::TRANS_ID_UPDATE_APP_HOOK_WINDOW_INFO`（枚举见 `window_scene\session\container\include\zidl\session_stage_ipc_interface_code.h:133`）
  - `MessageOption::TF_ASYNC`（单向通知，不等回包）
  - `HookWindowInfo` 以 Parcelable 整体写入。
- Stub（app 进程侧）：`window_scene\session\container\src\zidl\session_stage_stub.cpp:1731` `HandleUpdateAppHookWindowInfo` → 反序列化后调 app 侧 `ISessionStage::UpdateAppHookWindowInfo` 实现（在 ArkUI/窗口客户端仓库，不在本仓库）。
- 父类默认实现：`session.h:896` `virtual WSError UpdateHookWindowInfo(...) { return WS_OK; }`（非 MainSession 的 session 为空实现）。

### 3.4 其余内部触发点（不走 NAPI）

- **自由多窗口开关切换**：`SceneSessionManager::UpdateAppHookWindowInfoWhenSwitchFreeMultiWindow`（`scene_session_manager.cpp:19237`，由 `SetFreeMultiWindowMode*` 在 `:1222` 调用）。遍历所有主窗口 session：`hookWindowInfo.enableHookWindow = !isOpenFreeMultiWindow` 后调 `session->UpdateHookWindowInfo(...)`——开自由多窗口时关闭窗口 hook，关时恢复。
- **连接时状态回填**：`Session::InitSessionPropertyWhenConnect`（`session.cpp:1704`，由 `Session::Connect` `:1683` 调用）把服务端 property 的 `ForceSplitEnable`/`HookWindowInfo`（`:1773-1777`，仅主窗口）等回填到客户端 Connect 请求携带的 property；`Session::Reconnect`（`:1819`）则经 `SetSessionPropertyForReconnect` 恢复完整 property。
- **app 连接响应**：`SessionStub::HandleConnect`（`session_stub.cpp:540`）在 reply 中回写 property 序列，其中 `:641` `GetForceSplitEnable()`、`:642-643` `GetHookWindowInfo()`、`:644` `CompatibleModeProperty`（Parcelable）、`:648` `WriteCombinedCompatibleConfig`（`:511` 定义：字符串数组 + size，上限 `COMBINED_COMPATIBLE_CONFIG_MAX_SIZE=5`，`ws_common.h:59`）——app 连接完成即可拿到 hook/分栏/兼容模式全部状态。

## 4. 链路二：屏幕 hook（UpdateAppHookDisplayInfo / UpdateDisplayHookInfo）

### 4.1 两个 NAPI 入口（均在 `js_scene_session_manager.cpp`）

**入口 A：`updateAppHookDisplayInfo`（绑定 `:321`，实体 `OnUpdateAppHookDisplayInfo` `:5266`）**

参数（argc>=3，第 4 个可选）：
1. `uid`: number（int32）
2. `hookInfo`: object —— 解析在 `js_scene_utils.cpp:1132 ConvertHookInfoFromJs`：`width`/`height`/`density`/`rotation`/`displayOrientation`/`actualRect` 必填；`enableHookRotation`/`enableHookDisplayOrientation` 可选，缺失按 false（注意 §9.10 已知缺陷：当前 JS 实际无法置位 `enableHookRotation`）。
3. `enable`: bool
4. `persistentId`: number，可选 —— 指定目标 session（用于 session 尚未连接的 pending 场景）

**入口 B：`updateDisplayHookInfo`（绑定 `:311`，实体 `OnUpdateDisplayHookInfo` `:5210`）**

参数：`uid, width, height, density, enable`（简化版，无 rotation/方向）。内部组装 `DMHookInfo`（rotation/displayOrientation 相关全 false）。

两入口最终都落到 `ScreenSessionManagerClient::UpdateDisplayHookInfo(uid, enable, dmHookInfo)`。

### 4.2 SceneSessionManager::UpdateAppHookDisplayInfo（`scene_session_manager.cpp:19195`）

核心逻辑分两支：

**分支一（uid 无效但有 persistentId，`:19200-19213`）——pending 暂存：**
- enable 时校验 `width_/height_/density_ > 0`。
- `GetSceneSession(persistentId)` 找不到 → `WM_ERROR_NULLPTR`。
- 找到则 `session->SetPendingAppHookDisplayInfo(hookInfo, enable)` 暂存到 session，直接返回 OK。**暂存的 hookInfo 后续在 session Connect 时用 `callingUid_`（app 真实 uid）下发**，而不是 JS 传入的 uid。

**分支二（正常路径，`:19214-19234`）：**
- enable 时校验 `uid > 0` 且 `width_/height_/density_ > 0`，否则 `WM_ERROR_INVALID_PARAM`。
- `HookInfo` → `DMHookInfo` 转换，**enable=false 时 `enableHookRotation_`/`enableHookDisplayOrientation_` 强制置 false**；`actualRect_` 逐字段拷贝；`isFullScreenInForceSplit_` 从 `fullScreenInForceSplitUidSet_`（分栏全屏 uid 集合，见 §5.4）查询。
- `ScreenSessionManagerClient::GetInstance().UpdateDisplayHookInfo(uid, enable, dmHookInfo)`。

### 4.3 DM 侧落地（`window_scene\screen_session_manager\src\screen_session_manager.cpp:4161`）

`ScreenSessionManager::UpdateDisplayHookInfo`（DMServer，经 IPC：`ScreenSessionManagerClient`(`screen_session_manager_client.cpp:1037`) → `ScreenSessionManagerProxy`(`screen_session_manager_proxy.cpp:4727`) → stub `screen_session_manager_stub.cpp:1464`）：

1. 权限：`SessionPermission::IsSystemCalling()`，否则拒绝。
2. `hookInfoMutex_` 保护下维护 `displayHookMap_`：enable 时 `displayHookMap_[uid] = hookInfo`；disable 时 `erase(uid)`。
3. `NotifyDisplayChangedByUid(screenSessionMapCopy, DISPLAY_SIZE_CHANGED, uid)`（`:4196`）：对每个 screenSession → `NotifyDisplayChangedByUidInner`（`:4209`）：
   - `ScreenSessionManagerAdapter::GetInstance().OnDisplayChange(displayInfo, event, uid)` —— 带 uid 的 display change 通知，最终让目标 app 的 DisplayListener 收到"屏幕尺寸变化"，触发 app 重新查询（拿到 hook 后的）display 信息。
   - `CheckAttributeChangeWithUid`。

**查询接口（已存在，扩展时勿重复新建 IPC）**：`ScreenSessionManager::GetDisplayHookInfo(uid, hookInfo)`（`:4224`，`hookInfoMutex_` 下读 `displayHookMap_`）已有完整链路：stub `TRANS_ID_GET_DISPLAY_HOOK_INFO`（`screen_session_manager_stub.cpp:1467`，枚举 `DisplayManagerMessage` 定义于 `dmserver\include\display_manager_interface_code.h`）→ proxy `ScreenSessionManagerProxy::GetDisplayHookInfo`（`screen_session_manager_proxy.cpp:4772`）→ `ScreenSessionManagerClient::GetDisplayHookInfo`（`screen_session_manager_client.cpp:1046`）。**注意 reply 仅回 5 个字段**（width/height/density/rotation/enableHookRotation），**没有"是否开启"标志**——uid 不在 `displayHookMap_` 时返回全 0 默认值，与"未开启"不可区分（map 中存在即开启：enable 写入、disable erase）；`displayOrientation_`/`enableHookDisplayOrientation_`/`isFullScreenInForceSplit_`/`actualRect_` 均未回传。既有消费方：`SceneSession::HookAvoidAreaInCompatibleMode`（`scene_session.cpp:3521`，取 density 换算避让区）。扩展查询能力应**复用该 TRANS ID**，reply 新字段在 stub 写与 proxy 读**两处同步末尾追加**，无需新增枚举值；该链路无 `IsSystemCalling` 校验（调用方均为系统侧）。

**枚举与架构事实（扩展查询前必读）**：`DisplayManagerMessage` 为 `enum class : unsigned int`，首成员从 0 顺序递增，`TRANS_ID_SET_CLIENT = 2500`（`:133`）是第二块锚点，`TRANS_ID_GET_DISPLAY_HOOK_INFO`（`:166`）位于枚举**中部**——插入/删除/重排成员会使其后所有码值漂移（wire ABI 破坏），新增枚举值只允许追加在**末尾**。该枚举头虽放在 `dmserver\include\` 下，但 **dmserver 分离架构源码不消费它**（dmserver 的 IPC 由 `IDisplayManager.idl` 生成、码值独立），实际消费方是统一架构主链（`screen_session_manager_interface.h` → proxy/stub）、lite 链（`screen_session_manager_lite_proxy`、`dm_lite\src\display_manager_lite_proxy.cpp`）与 fuzz（枚举了全部码值）。查询链**仅统一架构存在**（dmserver 无实现、.idl 无声明），扩展查询**无需**分离架构同步。另注意：两个 stub fuzzer 的码表是**手工维护**的（`screensessionmgrstub_fuzzer.cpp`、`screensessionmgrstubwake_fuzzer.cpp`）——新增枚举值后需同步补码表与 case，否则新码零 fuzz 覆盖。

### 4.4 pending / 预启动（prelaunch）机制（`window_scene\session\host\src\session.cpp`）

- `SetPendingAppHookDisplayInfo`（`:1159`）：mutex 保护下暂存 `pendingAppHookDisplayInfo_`/`pendingAppHookDisplayInfoEnable_`/`hasPendingAppHookDisplayInfo_`。
- `NotifyPendingAppHookDisplayInfo`（`:6488`）：取走 pending，若存在且 `updateAppHookDisplayInfoFunc_` 有效，以 `callingUid_` 回调（→ SSM::UpdateAppHookDisplayInfo 走分支二直下 DM）。调用时机：
  - `Session::Connect`（`:1686`）
  - `Session::Reconnect`（`:1842`）
- `HandlePrelaunchDisplayHook`（`:6508`，Connect 流程 `:1692` 调用）：门控 `ctx.enable && updateAppHookDisplayInfoFunc_`。`ctx.enable` 来自 `GetPrelayoutContext`（`:6430`）置的 `sessionInfo_.isGamePrelaunch_`（`:6435`）——**仅游戏预启动**才填充 display 上下文（宽高/rotation 取 `PreCalcWindowProperty` 预计算值，density 取目标 display 的 screenSession 属性）；普通 `isPrelaunch_` 只预排 winRect、不触发 hook（`:6449-6453` 早退）。满足门控时用 ctx.display 组装 `HookInfo`（`enableHookRotation_=true`）以 enable=true、`callingUid_` 调回调；成功后置 `prelaunchDisplayHookEnabled_=true`。
- `ClearPrelaunchDisplayHook`（`:6534`）：仅 `prelaunchDisplayHookEnabled_` 为 true 且回调有效时执行，以空 `HookInfo{}` + enable=false、`callingUid_` 调回调；由 `SetIsGamePrelaunch(false)`（`:1994`，`:1995` 调用）触发。
- 回调注册：`SceneSessionManager::RegisterUpdateAppHookDisplayInfoFunc`（`scene_session_manager.cpp:16389`）在 CreateSceneSession（`:3959`）时给每个 sceneSession 挂 `UpdateAppHookDisplayInfo` 闭包；`Session::SetUpdateAppHookDisplayInfoFunc`（`session.cpp:4022`）存储。

### 4.5 方向变化通知（配套接口）

`notifyHookOrientationChange` NAPI（`js_scene_session_manager.cpp:323` 绑定 / `:5316` 实体，参数 persistentId）→ `SceneSessionManager::NotifyHookOrientationChange`（`:19292`，PostSyncTask）→ `sceneSession->UpdateOrientation()` → `Session::UpdateOrientation`（`session.cpp:1590`）→ `sessionStage_->UpdateOrientation()`（IPC 通知 app 更新方向）。用于屏幕 hook 的 rotation 生效后强制 app 重算方向。

## 5. 链路三：分栏（SetForceSplitEnable，通知 ArkUI）

### 5.1 入口（NAPI）

- 绑定：`js_scene_session.cpp:732` `"setForceSplitEnable"` → 静态壳 `:3302` → 实体 `OnSetForceSplitEnable`（`:7749`）。
- JS 参数（3 个，全部必填）：
  1. `isForceSplitEnabled`: bool
  2. `needUpdateViewport`: bool（是否同步更新 app 视口）
  3. `selectMode`: number（`SelectMode`，WIDE_MODE=0 / SQUARE_MODE=1 / INVALID_MODE=2）
- → `session->SetForceSplitEnable(...)`（MainSession override）。

### 5.2 MainSession::SetForceSplitEnable（`main_session.cpp:776`）

`PostTask`（task 名 `"SetForceSplitEnable"`）内依次：
1. `setSelectModeCallback_(selectMode)` —— 回调在 SSM `CreateSceneSession`（`scene_session_manager.cpp:3477-3485`，**仅主窗口**）时注册为 `SSM::SetSelectMode`（`:22544`）：把 `selectMode_` 存为 SSM 全局原子变量（`TLOGI` 记录旧→新值）。
2. `property->SetForceSplitEnable(isForceSplitEnabled)` —— 落窗口 property（`window_session_property.cpp:2858`，IPC 序列化/反序列化支持见 `:1860`）。
3. `sessionStage_->SetForceSplitEnable(isForceSplitEnabled, needUpdateViewport, selectMode)` —— IPC 通知 app（ArkUI）开启/关闭分栏。

### 5.3 IPC（WMS → app）与查询（app → WMS）

- Proxy：`session_stage_proxy.cpp:2879`，code `TRANS_ID_SET_FORCE_SPLIT_ENABLE`（`session_stage_ipc_interface_code.h:134`），`TF_ASYNC`，依次写 bool/bool/uint32。
- Stub：`session_stage_stub.cpp:1743` `HandleSetForceSplitEnable` → app 侧 `ISessionStage::SetForceSplitEnable`（ArkUI 实现分栏布局与视口更新）。
- 反向查询（app → WMS，`SessionProxy` → `SessionStub`）：
  - `GetForceSplitEnable(bool&)`：proxy `session_proxy.cpp:2968` → stub `HandleGetForceSplitEnable`（`session_stub.cpp:2192`）→ `SceneSession::GetForceSplitEnable`（`scene_session.cpp:9169`，读 property）。
  - `GetSelectMode(SelectMode&)`：proxy `:3000` → `HandleGetSelectMode`（`:2176`）→ `SceneSession::GetSelectMode`（`scene_session.cpp:9180`，PostSyncTask）经 `getSelectModeFunc_`（SSM 注册于 `scene_session_manager.cpp:3408`）读 SSM 全局 `selectMode_`。
  - `GetAppForceLandscapeConfig(AppForceLandscapeConfig&)`：proxy `:2939` → `HandleGetAppForceLandscapeConfig`（`:2159`）→ `SceneSession::GetAppForceLandscapeConfig`（`scene_session.cpp:9160`）。
- 另有独立 NAPI `sceneSessionManager.setSelectMode`（绑定 `js_scene_session_manager.cpp:317`）可直接设置 SSM 全局 `selectMode_`。

### 5.4 分栏全屏联动（app 全屏状态 → 屏幕 hook）

分栏模式下 app 切到全屏时，需要让屏幕 hook 感知，否则虚拟 display 比例不对：

```
app: SessionProxy::NotifyIsFullScreenInForceSplitMode(isFullScreen)   (session_proxy.cpp:4541)
 → SessionStub::HandleNotifyIsFullScreenInForceSplitMode (session_stub.cpp:2899)
 → MainSession::NotifyIsFullScreenInForceSplitMode (main_session.cpp:644)
    · isFullScreenInForceSplit_ 原子存储
    · forceSplitFullScreenChangeCallback_(GetCallingUid(), isFullScreen)
 → SSM::NotifyIsFullScreenInForceSplitMode(uid, isFullScreen) (scene_session_manager.cpp:22246)
    · 维护 fullScreenInForceSplitUidSet_（insert/erase，mutex 保护）
    · ScreenSessionManagerClient::NotifyIsFullScreenInForceSplitMode
 → ScreenSessionManager::NotifyIsFullScreenInForceSplitMode (screen_session_manager.cpp:4232)
    · displayHookMap_[uid].isFullScreenInForceSplit_ = isFullScreen（uid 不在 map 中则忽略）
    · 再次 NotifyDisplayChangedByUid(...) 触发 app 收到新的（含全屏标记的）display 信息
```

- 回调注册：`RegisterForceSplitFullScreenChangeCallback`（SSM 在 `scene_session_manager.cpp:3461` 注册）。
- 会话恢复：`OnSessionRecoverStateChange` 的 `SESSION_FINISH_RECONNECT` 分支（`:710`）用 property 恢复 `NotifyIsFullScreenInForceSplitMode(property->IsFullScreenInForceSplitMode())`。
- `isFullScreenInForceSplit_` 通过 `DMHookInfo` 字段随屏幕 hook 下发（见 §4.2 分支二）。

### 5.5 分栏比例联动（分栏 ↔ 窗口 hook）

`MainSession::NotifySplitRatioChanged(float newRatio)`（`main_session.cpp:673`，app 侧经 `SessionProxy::NotifySplitRatioChanged`（`session_proxy.cpp:4572`）→ `HandleNotifySplitRatioChanged`（`session_stub.cpp:2914`）触发）：`property->SetWidthHookRatio(newRatio)`（`window_session_property.cpp:2880`）+ `splitRatioChangeCallback_`（`RegisterSplitRatioChangeCallback` 注册）。即分栏比例直接写进窗口 hook 的 `widthHookRatio`，两条链路在此交汇。

### 5.6 页面状态上报（app → SCB JS）

分栏应用页面切换时上报，供 SCB 侧记录/显示：

```
app: SessionProxy::NotifyPageEnable(action, message)  (session_proxy.cpp:4629)
 → MainSession::NotifyPageEnable (main_session.cpp:692)
    · 校验 action/message 非空且长度 <= MAX_ACTION_LIMIT_SIZE/MAX_MESSAGE_LIMIT_SIZE
 → SSM::NotifyPageEnableFunc (scene_session_manager.cpp:9230)
 → pageEnableFunc_（JsSceneSessionManager::RegisterPageEnableCallback 注册，:6915）
 → napi 回调 SCB JS 层（bundleName, windowId, action, message）
```

### 5.7 强制横屏/分栏配置（AppForceLandscapeConfig）

- NAPI `setAppForceLandscapeConfig`（绑定 `js_scene_session_manager.cpp:315`，实体 `:5346`，参数 bundleName + config{containsConfig/isRouter/configJsonStr}）→ `SSM::SetAppForceLandscapeConfig`（`:19477`）：存 `appForceLandscapeMap_[bundleName]` 并对同 bundleName 的存量 session 调 `NotifyAppForceLandscapeConfigUpdated()`。
- session 侧查询：`SceneSession::GetAppForceLandscapeConfig`（`scene_session.cpp:9160`）经 `forceSplitFunc_`（SSM 注册于 `:3405`，读 `GetAppForceLandscapeConfig` `:19503`）。配置在 app 连接/页面路由时决定是否触发强制横屏/分栏（`configJsonStr_` 为 JSON 规则）。

## 6. 关联机制

### 6.1 PC 应用兼容（PcAppInpad 系列）

同目录下还有一组 PC 应用在 Pad 上的兼容接口（`js_scene_session.cpp`，日志标签 `WMS_PC`），与本三条链路独立但常一起出现：

- `setPcAppInpadCompatibleMode` / `setPcAppInpadSpecificSystemBarInvisible` / `setPcAppInpadOrientationLandscape` / `setMobileAppInPadLayoutFullScreen`
- 服务端实现均在 `Session`（`session.cpp:4443` 起），只写 property；重连时随 property 恢复（`session.cpp:1781-1784`）。

另有 `MainSession::NotifyCompatibleModeChange(CompatibleStyleMode)`（`main_session.cpp:717`；app 侧经 `SessionProxy::NotifyCompatibleModeChange`（`session_proxy.cpp:4603`）→ `HandleNotifyCompatibleModeChange`（`session_stub.cpp:2945`）触发，经 `compatibleModeChangeCallback_` 回调 SSM）用于兼容比例模式变化通知，`CompatibleStyleMode` 枚举含 `LANDSCAPE_SPLIT` 等值。

### 6.2 app 兼容能力上报与消费（CompatibleModeProperty / CombinedCompatibleConfig）

app 侧的兼容能力声明（"适配了沉浸式/事件映射/比例缩放/拖拽缩放"）与行为限制（禁止拖拽 resize/禁止 DPI 调整/禁止全屏/禁止分栏/限制窗口数量/禁止装饰全屏）由 `CompatibleModeProperty`（`window_session_property.h:778`，字段见 §2）承载；`CombinedCompatibleConfig` 是字符串数组（来自 SessionInfo，SCB 侧填充，上限 `COMBINED_COMPATIBLE_CONFIG_MAX_SIZE=5`）。

数据流：
1. **填充**：`SceneSession` 构造时把 `sessionInfo.combinedCompatibleConfig` 写入 property（`scene_session.cpp:244`）；app 侧 `SessionProxy` 连接时同样携带（`session_proxy.cpp:295`）。
2. **Connect 回写**：`SessionStub::HandleConnect` reply 中 `:644` 回写 `CompatibleModeProperty`（Parcelable）、`:648` `WriteCombinedCompatibleConfig`（见 §3.4）。
3. **WMS→app 推送**：`Session`（`session.cpp:4425`）经 `sessionStage_->NotifyCompatibleModePropertyChange(property)` → `SessionStageProxy::NotifyCompatibleModePropertyChange`（`session_stage_proxy.cpp:1906`，code `TRANS_ID_NOTIFY_COMPATIBLE_MODE_PROPERTY_CHANGE`，`session_stage_ipc_interface_code.h:92`）→ app 侧 `HandleNotifyCompatibleModePropertyChange`（`session_stage_stub.cpp:1281`）→ app 实现 `WindowSceneSessionImpl::NotifyCompatibleModePropertyChange`（`wm\src\window_scene_session_impl.cpp:7459`，存入 property_）。

服务端消费点（`IsAdaptTo*` / `Disable*` 门控）：
- `IsAdaptToImmersive()`：**避让区 hook 门控**——`GetSystemAvoidArea`（`scene_session.cpp:3232`）与 `GetAINavigationBarArea`（`:3436`）命中即改走 `HookAvoidAreaInCompatibleMode`（`:3512`，细节见 §9.11）并跳过常规避让区计算。
- `IsAdaptToEventMapping()`：事件映射（`session.cpp:1763`、`scene_session.cpp:3811`）。
- `IsAdaptToProportionalScale()` / `IsAdaptToSimulationScale()`：比例/模拟缩放（`scene_session.cpp:1223`、`:4855-4880`）。
- `IsAdaptToDragScale()`：拖拽缩放（`move_drag_controller.cpp:1552`、`session.cpp:1436`、`scene_session.cpp:1318/4955`）。

## 7. 线程与 IPC 模型要点

- **NAPI → session**：均在 JS 线程解析参数，`weakSession_.promote()` 判活后调用。
- **session 内部**：`UpdateAppHookWindowInfo`/`SetForceSplitEnable` 均通过 `PostTask`（task 名即函数名字符串）切到 session 任务线程，lambda 捕 `wptr` 防悬挂；`GetSelectMode` 用 `PostSyncTask`。
- **WMS → app IPC**：`SessionStageProxy`，`TF_ASYNC` 单向；`HookWindowInfo` 走 Parcelable，SetForceSplitEnable 走原始字段（bool/bool/uint32）。
- **WMS → DMServer IPC**：`ScreenSessionManagerClient` → `ScreenSessionManagerProxy`（统一架构下 DM 与 WM 同进程部署，仍走 zidl 接口隔离）。
- **互斥量**：`pendingAppHookDisplayInfoMutex_`（pending 暂存）、`hookInfoMutex_`（DM 侧 displayHookMap_）、`fullScreenInForceSplitUidSetMutex_`（分栏全屏 uid 集合）、`hookWindowInfoMutex_`/`selectModeMutex_`（property）。
- `selectMode_`（SSM）与 `isFullScreenInForceSplit_`（MainSession）为原子变量。

## 8. 验证与排障

### 测试目标映射（测试文件 → gn 目标）

测试目标定义于 `window_scene\test\unittest\BUILD.gn`（DM 侧在 `window_scene\test\dms_unittest\BUILD.gn`），`module_out_path = window_manager/window_manager/window_scene`：

| 链路 | 测试文件 | 构建目标（ohos_unittest） | 用例示例 |
|------|----------|---------------------------|----------|
| 窗口 hook | `window_scene\test\unittest\main_session_test.cpp` | `ws_main_session_test` | UpdateHookWindowInfo01-04 |
| 窗口 hook（IPC 下发） | `window_scene\test\unittest\compatible_mode_main_session_test.cpp` | `ws_compatible_mode_main_session_test` | UpdateAppHookWindowInfo01-04、NotifyCompatibleModeChange |
| 屏幕 hook | `window_scene\test\unittest\scene_session_manager_test7.cpp` | `ws_scene_session_manager_one_test` | UpdateAppHookDisplayInfo001-002 |
| 屏幕 hook（pending/prelaunch） | `window_scene\test\unittest\session_test4.cpp` | `ws_session_test` | TestHandlePrelaunchDisplayHook* / TestClearPrelaunchDisplayHook* |
| 屏幕 hook（布局联动） | `window_scene\test\unittest\layout\session_layout_test.cpp` | `ws_layout_test` | SetPendingAppHookDisplayInfo001 / NotifyPendingAppHookDisplayInfo001-002 |
| 避让区 hook（兼容能力消费） | `window_scene\test\unittest\scene_session_test5.cpp` | `ws_scene_session_test` | HookAvoidAreaInCompatibleMode（门控/类型/密度分支） |
| 屏幕 hook（DM 侧） | `window_scene\test\dms_unittest\screen_session_manager_test_eight.cpp` 等 | `ws_screen_session_manager_test_eight` | UpdateDisplayHookInfo001-002、NotifyIsFullScreenInForceSplitMode、GetDisplayHookInfo |
| 屏幕 hook（DM IPC 两侧） | `window_scene\test\dms_unittest\screen_session_manager_proxy_test.cpp` / `screen_session_manager_proxy_ut_test.cpp` / `screen_session_manager_client_test.cpp` | `ws_screen_session_manager_proxy_test` / `ws_screen_session_manager_proxy_ut_test` / `ws_screen_session_manager_client_test` | GetDisplayHookInfo（proxy 读写序）、UpdateDisplayHookInfo、NotifyIsFullScreenInForceSplitMode。**注意**：`proxy_test` 与 `proxy_ut_test` 构建门控不同（前者 sceneboard=true、后者 false），分别覆盖两种架构形态，改 IPC 时**两个文件都要同步改** |
| 分栏 | `window_scene\test\unittest\main_session_test.cpp` | `ws_main_session_test` | SetForceSplitEnable01-05、RegisterSetSelectModeCallback01、NotifyIsFullScreenInForceSplitMode |
| 分栏（IPC） | `window_scene\test\unittest\session_stage_proxy_test.cpp` / `session_stage_stub_test.cpp` | `ws_session_stage_proxy_test` / `ws_session_stage_stub_test` | SetForceSplitEnable01、HandleSetForceSplitEnable01-02 |
| 分栏/兼容 property | `window_scene\test\unittest\window_session_property_test.cpp` / `compatible_mode_property_test.cpp` | `ws_window_session_property_test` / `ws_compatible_mode_property_test` | SetForceSplitEnable001、IsAdaptTo* 系列 |

Mock：`window_scene\test\mock\mock_session_stage.h:122` 有 `MOCK_METHOD3(SetForceSplitEnable, ...)`。全量入口：`window_scene:test`（见 `docs/Testing.md`）。

### 构建与运行命令

本仓库是 OpenHarmony 组件（component `window_manager`，subsystem `window`），**不能脱离 OpenHarmony 源码树独立编译**。以下命令在 OHOS 源码树根（本仓库对应 `foundation/window/window_manager`）执行：

```bash
# 构建（单个测试目标）
./build.sh --subsystem-name window --component-name window_manager --build-target ws_main_session_test
# 已有 out 目录时也可直接：
ninja -C out/<product> ws_main_session_test

# 运行（产物一般位于 out/<product>/tests/unittest/window_manager/window_manager/window_scene/，以实际为准）
hdc shell "mkdir -p /data/local/tmp/wm_ut"
hdc file send out/<product>/tests/unittest/window_manager/window_manager/window_scene/ws_main_session_test /data/local/tmp/wm_ut/
hdc shell "chmod +x /data/local/tmp/wm_ut/ws_main_session_test && \
  /data/local/tmp/wm_ut/ws_main_session_test --gtest_filter='MainSessionTest.UpdateHookWindowInfo*'"
```

### Done 定义（按改动范围选择，全绿才算完成）

| 改动范围 | 必须通过 |
|----------|----------|
| 窗口 hook 逻辑（`MainSession::UpdateHookWindowInfo`/`UpdateAppHookWindowInfo`） | `ws_main_session_test` 的 UpdateHookWindowInfo01-04 + `ws_compatible_mode_main_session_test` |
| 屏幕 hook 逻辑（SSM/Session/DM） | `ws_scene_session_manager_one_test` 的 UpdateAppHookDisplayInfo001-002 + `ws_session_test`（prelaunch）+ `ws_layout_test`（pending）；动 DM 侧另加 `ws_screen_session_manager_test_eight` |
| 分栏逻辑（`SetForceSplitEnable`/selectMode/全屏联动） | `ws_main_session_test` 的 SetForceSplitEnable01-05 + NotifyIsFullScreenInForceSplitMode |
| IPC 协议（proxy/stub/Parcelable 字段） | 上表对应行 + `ws_session_stage_proxy_test` + `ws_session_stage_stub_test`（两侧必须同时改、同时测） |
| property 序列化 | `ws_window_session_property_test`（+ `ws_compatible_mode_property_test`） |

**验证失败/无法验证时的回退**：若在独立 checkout（无 OHOS 源码树/编译环境/设备）中工作，无法执行上述命令，必须在最终答复中明确写出"未能运行构建与单测"及具体原因（缺环境/缺设备），并列出建议人工执行的命令与目标；**不得声称已验证**。构建成功但用例失败时，修复后重跑同一目标，不得缩小 `--gtest_filter` 范围来"绕过"失败用例。

### 日志关键字（hilog）

- NAPI 入口：`TLOGD(WmsLogTag::WMS_COMPAT, "[NAPI]")`
- `hookWindowInfo:[...]`（MainSession::UpdateHookWindowInfo，`HookWindowInfo::ToString()`）
- `isForceSplitEnabled: %d, needUpdateViewport: %d, selectMode: %u`
- `set SelectMode from %u to %u`
- `uid: %d, hookInfo: [%s], enable: %d, persistentId: %d`（SSM::UpdateAppHookDisplayInfo）
- `update hookinfo ...`（DM 侧，DMS 标签）
- `Id: %d`（NotifyPendingAppHookDisplayInfo）
- `Update prelaunch display hook successfully` / `Clear prelaunch display hook successfully`

## 9. 易错点 / 注意事项

1. **入口文件**：`updateAppHookDisplayInfo`/`updateDisplayHookInfo`/`notifyHookOrientationChange`/`setAppForceLandscapeConfig`/`setSelectMode` 在 `js_scene_session_manager.cpp`；`updateHookWindowInfo`/`setForceSplitEnable` 在 `js_scene_session.cpp`。
2. **notifyWindowChange 双面性**：property 落库时强制 false（`main_session.cpp:760`），但 IPC 下发给 app 的 `UpdateAppHookWindowInfo` 传原始值（`main_session.cpp:767`）。
3. **disable 语义清洗**：`UpdateAppHookDisplayInfo` 在 enable=false 时强制 `enableHookRotation_=false`、`enableHookDisplayOrientation_=false`（`scene_session_manager.cpp:19223-19225`），防止残留半开状态。
4. **pending 路径的 uid**：persistentId 分支暂存的 hookInfo 在 Connect 后用 `callingUid_`（app 真实 uid）下发，JS 传的 uid 被忽略。
5. **diff 才下发**：窗口 hook 属性无变化不会 IPC 通知 app；排障时"设了没反应"先确认值是否真的变化（浮点用 NearZero 容差比较）。
6. **权限**：DM 侧 `UpdateDisplayHookInfo`/`NotifyIsFullScreenInForceSplitMode` 要求 `IsSystemCalling()`；NAPI 层面依赖 SCB 进程调用。
7. **仅主窗口**：`SetSelectModeCallback`/`PageEnableCallback` 只在主窗口 session 注册；`SetSessionPropertyForReconnect` 只对主窗口恢复 hook/分栏 property。
8. **两条 selectMode 存储**：SSM 全局 `selectMode_`（setSelectMode NAPI 与 SetForceSplitEnable 回调写）与 `WindowSessionProperty::selectMode_`（property，随 IPC 传递）是两个状态，勿混淆。
9. **`isAbilityHook`/`GetIsAbilityHookOff`**（session.cpp:1780 等）属于会话恢复/委托（delegator）机制，与本文 hook 机制无关，仅名字相近。
10. **已知缺陷：JS 无法置位 `enableHookRotation`**：`ConvertHookInfoFromJs`（`js_scene_utils.cpp:1178-1183`）条件写反——属性存在且**转换成功**时反而走 else 强制赋 false；缺失/转换失败时才赋值（此时局部变量恒为 false）。对比同函数紧邻的 `enableHookDisplayOrientation`（`:1192-1198`）为正确写法（成功才赋值）。后果：`updateAppHookDisplayInfo` 的 JS 调用方传 `enableHookRotation: true` 无效，rotation hook 从 JS 侧开不了——排障"JS 设了不生效"先查这里；C++ 内部路径（prelaunch `HandlePrelaunchDisplayHook` 直接组装 `HookInfo`）不受影响。这是真实缺陷（可修，不属于 §10-3 所列刻意清洗），但属 NAPI 参数语义，修复前按 §10 Ask before #1 征询。
11. **避让区 hook 的行为细节**（`scene_session.cpp:3512`，门控 `CompatibleModeProperty::IsAdaptToImmersive()`，见 §6.2）：①仅 `TYPE_SYSTEM`（顶部，`HOOK_SYSTEM_BAR_HEIGHT=40` × 密度）与 `TYPE_NAVIGATION_INDICATOR`（底部，`HOOK_AI_BAR_HEIGHT=28` × 密度；常量在 `:112-113`）两类被改写，其余类型记错误日志直接返回；②默认像素比 `vpr = 1.9f`，**行内注释错写为 3.5f——以代码为准，勿按注释"修正"代码或按注释理解行为**；③密度取 `GetDisplayHookInfo(GetCallingUid())`，为 0（该 app 未配置虚拟显示）时用默认值；④优先级陷阱：app 窗口带 `WINDOW_FLAG_NEED_AVOID` 时在 hook 分支**之前**早退（`:3226-3231`，连 hook 避让区也拿不到）；PIP 模式不 hook AI 导航条（`:3431`）。
12. **`GetDisplayHookInfo` 未命中时不写 out 参**（`screen_session_manager.cpp:4224`）：uid 不在 `displayHookMap_` 时函数不赋值，而 stub 侧 `DMHookInfo hookInfo;`（`screen_session_manager_stub.cpp:1469`）是**未初始化局部变量**——"返回全零"实为栈巧合（`DMHookInfo` 除 `actualRect_` 外无默认成员初始化器）。扩展该查询时务必对未命中路径显式值初始化（`DMHookInfo{}`），使"未配置 = 全零"成为确定语义而非碰运气。

## 10. 强制约束（Do not / Ask before）

面向 agent 的硬性边界，与 §9 描述性注意事项配合使用。

### Do not（禁止）

1. **不得破坏 IPC 字段序**：`HookWindowInfo` 的 Parcel 序（bool→float→bool→bool）、`SetForceSplitEnable` 的写序（bool/bool/uint32）是跨进程契约。新增字段只能在读写两侧（`Marshalling`/`Unmarshalling`、proxy 写入与 stub 读取）**同步追加在末尾**；禁止插入、重排、删除已有字段。
2. **不得绕过权限校验**：DM 侧 `UpdateDisplayHookInfo`/`NotifyIsFullScreenInForceSplitMode` 的 `SessionPermission::IsSystemCalling()` 校验失败是预期行为，禁止删除或放宽校验来"修复"调用失败。
3. **不得"修复"刻意的语义清洗**：property 落库时 `notifyWindowChange` 强制置 false（§9.2）、`enable=false` 时 hook 开关强制清 false（§9.3）、pending 路径忽略 JS 传入 uid 改用 `callingUid_`（§9.4），均为设计行为，不是 bug。
4. **不得合并两套 selectMode 状态**：SSM 全局 `selectMode_` 与 `WindowSessionProperty::selectMode_` 是独立状态（§9.8），禁止互相赋值或删除其一。
5. **不得在 NAPI/JS 线程直接调用 `sessionStage_` IPC**：必须沿用 `PostTask` 模式（弱引用提升判活，task 名即函数名）。
6. **不得缩小 gtest filter 规避失败用例**（见 §8 回退要求）。
7. **IPC 改动必须同步"源"的全部四处，不得只改一侧**：`window_scene` 的 zidl proxy/stub 是**手写源码**（本模块无 .idl），以 `session_stage` 为例，一处 IPC 变更需人工同步：接口纯虚类（`session_stage_interface.h`）、proxy 写入（`session_stage_proxy.cpp`）、stub 分发+handler（`session_stage_stub.cpp`）、IPC code 枚举（`session_stage_ipc_interface_code.h`）；相邻模块 `dmserver`/`wmserver`/`dm_lite` 的 proxy/stub 则是 `.idl` **生成产物**（见各模块根目录 `*.idl`），须改 `.idl` 后经构建重新生成，不得直接手改生成文件。
8. **不得反转依赖方向**：`interfaces\innerkits` 公共头（`wm_common.h`、`dm_common.h` 等）必须自包含，禁止 include `window_scene` 内部实现头；修改 `HookWindowInfo`/`DMHookInfo` 等公共结构时不得引入对内部模块的依赖。
9. **NAPI 层不得直调 `ScreenSessionManagerClient`，必须经 `SceneSessionManager` 包装**：`js_scene_session_manager.cpp` 属 window_scene 会话层，`screen_session_manager_client` 属 DM client 层——分层契约是 NAPI → SSM（`SceneSessionManager` 单例）→ `ScreenSessionManagerClient` → proxy。新增查询/设置类 NAPI（如 `GetDisplayHookInfo` 包装）照此分层在 SSM 加薄包装方法，不得为"省一层"在 NAPI 里直接调 `ScreenSessionManagerClient::GetInstance()`（写链路 `UpdateAppHookDisplayInfo`/`UpdateDisplayHookInfo` 均走 SSM，读链路保持对称）。
10. **SSM 新增对外方法必须同步符号导出表**：`SceneSessionManager` 的库带 version_script（`window_scene\session_manager\libscene_session_manager.map`），新方法不加入导出列表则 NAPI 模块（动态链接该库）**链接期符号不可见**——`UpdateAppHookDisplayInfo*` 在 `:208` 有先例，新增 `Get*`/`Set*` 公有方法时照此追加；Session 层符号已被 `libscene_session.map` 通配覆盖，无需额外处理。

### Ask before（先征询用户）

1. **新增/修改 NAPI 参数**（`updateHookWindowInfo`/`setForceSplitEnable`/`updateAppHookDisplayInfo` 等）：调用方是 SCB JS 侧与 ArkUI，跨仓对接需确认。
2. **修改 IPC 枚举或 IPC 字段**：WMS→app 链路（`SessionStageInterfaceCode`，`session_stage_ipc_interface_code.h`）的 stub/proxy 实现在 ArkUI/窗口客户端仓库（不在本仓库），需两侧同步发版；WMS→DM 链路（`screen_session_manager` zidl）的 `DisplayManagerMessage` 枚举（`dmserver\include\display_manager_interface_code.h`）是**多消费方共享契约**——统一架构主链、lite 链（`screen_session_manager_lite_proxy`、`dm_lite`）与 fuzz 全部按序号消费，新增枚举值只允许追加在枚举**末尾**（中部插入会使后续所有码值漂移、破坏 wire ABI；`TRANS_ID_SET_CLIENT=2500` 与 `TRANS_ID_GET_DISPLAY_HOOK_INFO` 均在中部），变更前同样需先确认。
3. **修改 `WSError`/`WMError`/`WmErrorCode` 返回值语义**：错误码跨 C/ArkTS 两侧消费。
4. **修改 pending/prelaunch 语义或 diff 下发条件**：影响 app 启动时序，需评估预启动、会话恢复场景。
5. **`git commit`**：仓库根 `AGENTS.md` 规定必须先经用户批准，并用 `git commit -s` + Angular 格式 + `Co-Authored-by: Agent` 落款。

## 11. 设计指导（新需求落点与架构影响）

面向"后续需求怎么设计"的判断框架，提炼自本子域已验证的模式与教训。本节是指导而非硬约束，与 §10 冲突时以 §10 为准。

### 11.1 需求落点：先归类，再选模板

| 需求形态 | 落点模板 | 参考 |
|----------|----------|------|
| SCB 下发新配置/开关（JS→WMS，窗口维度） | NAPI（`js_scene_session.cpp`）→ MainSession → property + diff 下发 → SessionStageProxy | §3 |
| SCB 下发新配置（显示维度） | NAPI（`js_scene_session_manager.cpp`）→ SSM → ScreenSessionManagerClient → DM 侧 uid-keyed map + 按 uid 通知 | §4 |
| SCB 查询状态（JS←WMS） | **优先复用既有查询链**（TRANS ID 复用 + reply 末尾追加），其次才新建 | §4.3、§5.3 |
| app 上报能力/状态（app→WMS） | 能力声明类 → `CompatibleModeProperty` 加 bool 位 + Marshalling + 消费点门控；事件通知类 → `SessionProxy::Notify*` 家族 | §5.4-§5.6、§6.2 |
| 服务端数据面扩展 | uid-keyed map + mutex + 按_uid 通知模板（`displayHookMap_`） | §4.3 |

原则：**离哪个既有机制最近就扩展哪个**。新建 IPC 面的成本 = 手写四处同步（§10-7）+ 跨仓确认（§10 Ask before），仅在确无邻近机制时选择。既有代码的演进方向也印证这一点：查询面以"reply 追加字段"扩展（§4.3 仅回 5 字段）、能力面以"加 bool 位"扩展（§2 `CompatibleModeProperty`）、配置面走字符串数组（`CombinedCompatibleConfig`），而非新开通道。

### 11.2 架构影响评估清单（设计评审时逐项回答）

1. **目标架构**：仅统一架构还是两架构都要？判据：对应链路在 dmserver/wmserver 是否存在（grep 确认，勿凭头文件位置推断——`dmserver\include\` 下的共享枚举并不代表 dmserver 消费它，见 §4.3）。本子域全部链路仅统一架构存在，分离架构无需同步。
2. **IPC 面**：复用 TRANS ID（reply 末尾追加）还是新增（枚举**末尾**追加）？是否触碰共享契约（统一主链/lite/fuzz 均按序号消费该枚举）？
3. **手写 vs 生成物**：window_scene 的 zidl 手写四处同步；dmserver/wmserver/dm_lite 须改 `.idl` 后重新生成，不得手改生成文件（§10-7）。
4. **跨仓边界**：SCB（JS 调用方）、ArkUI/窗口客户端（`session_stage` stub 实现在外仓）——涉及即列入 Ask before。
5. **状态归属**：property（随 Connect 回写/重连恢复，§3.4）/ 运行时 map（进程级，重启即失）/ SSM 全局原子——三类勿混存同一语义（§9.8 双 selectMode 教训）。
6. **时序与线程**：JS 线程只做参数解析（下发走 PostTask）；app 未连接场景用 pending 暂存模板（§4.4）。

### 11.3 本子域反复出现的已验证模式

- **diff-then-notify**：无变化不 IPC（§9.5），减少无意义跨进程流量；
- **disable 即清洗**：关闭时清子开关/erase 表项，不留半开状态（§9.3）；
- **pending + callingUid 对账**：JS 传入的目标标识仅用于寻址，落地一律用 app 真实 uid（§9.4）；
- **property 单一事实源**：下发与回填共用一份，Connect reply / 重连恢复保证两端一致（§3.4）；
- **能力声明与策略下发分离**：app 声明"适配了什么"（`CompatibleModeProperty`），SCB 决定"做什么"（hook 配置），两个方向勿混用载体。
