# 沉浸式窗口：定义、架构与北向接口

## 文档定位

本文是窗口子系统沉浸式能力的总入口，回答以下问题：

- “全屏”“沉浸式布局”“隐藏系统栏”分别改变什么；
- 新旧北向接口之间的关系；
- 统一架构和分离架构为何可能表现不同；
- `getImmersiveModeEnabledState()` 与 `isImmersiveLayout()` 为什么不能互换。

系统栏属性与 SceneBoard 策略见 [immersive-system-bars.md](./immersive-system-bars.md)，避让区见
[immersive-avoid-area.md](./immersive-avoid-area.md)，定位和回归见
[immersive-debugging-and-tests.md](./immersive-debugging-and-tests.md)。

## 证据快照

本文按以下工作区快照核验，仓库更新后应重新搜索符号并刷新行号：

| 仓库 | 分支 | 提交 | 提交日期 |
|---|---|---|---|
| `window_window_manager` | `master` | `dd8e1ae076` | 2026-09-07 |
| `window_scene_board` | `master` | `7797266343` | 2026-09-02 |
| `arkui_ace_engine` | `master` | `37c17d14a8` | 2026-09-04 |

版本和废弃信息优先参考[OpenHarmony SDK 接口声明](https://raw.githubusercontent.com/openharmony/interface_sdk-js/master/api/@ohos.window.d.ts)与
[窗口 API 官方参考](https://developer.huawei.com/consumer/cn/doc/harmonyos-references/arkts-apis-window-window)；
概念和产品使用限制可参考[窗口沉浸式官方指南](https://developer.huawei.com/consumer/cn/doc/HarmonyOS-Guides/immersive-window-feature)。
实现语义必须回到当前源码和测试确认。

## 最新定义与五个独立维度

当前“沉浸式布局”的核心判据是：**应用实际可绘制/可布局区域等于窗口大小**。它描述窗口内的布局范围，
不要求窗口模式一定是 `FULLSCREEN`，也不要求状态栏或导航区域一定隐藏。

分析问题前先拆开五个维度：

| 维度 | 代表状态或接口 | 不代表什么 |
|---|---|---|
| 窗口模式 | `WindowMode::WINDOW_MODE_FULLSCREEN`、最大化、悬浮、分屏 | 不自动证明内容绘制到整个窗口 |
| 布局/安全区 | `setWindowLayoutFullScreen`、`SetIgnoreViewSafeArea` | 不直接控制系统栏显隐 |
| 系统栏显隐 | `SystemBarProperty.enable_`、`setSpecificSystemBarEnabled` | 不必然改变窗口大小或模式 |
| 系统栏样式 | 背景色、内容色、动画、setting flag | 不必然改变显隐 |
| 产品策略 | SceneBoard 顶层会话、锁屏、分屏、PC Dock/标题栏策略 | 可能覆盖或派生最终显示效果 |

`WindowSceneSessionImpl::IsImmersiveLayout()`体现了最新判据：PC/Pad 自由多窗下读取 UIContent 的窗口绘制尺寸并与
窗口矩形比较；其他形态读取 `isIgnoreSafeArea_`。见 `wm/src/window_scene_session_impl.cpp:8454-8471`。

## 架构归属

| 层级 | 职责 | 稳定入口 |
|---|---|---|
| ArkTS/ANI/NDK 与 Rosen client | 参数校验、兼容版本、窗口本地状态、IPC 发起 | `interfaces/kits/`、`wm/` |
| 统一架构原生服务 | `SceneSession` 属性、避让区计算与通知 | `window_scene/session/`、`window_scene/session_manager/` |
| SceneBoard | 顶层会话与产品形态策略、SystemUI 最终呈现 | 关联仓 `window_scene_board/staticcommon/`、`product/` |
| ArkUI 下游边界 | `UIContent` 接收忽略安全区状态，或消费 Rosen 避让区 | 关联仓 `arkui_ace_engine/adapter/ohos/entrance/` |

构建使用 `window_manager_use_sceneboard` 选择实现；运行时 `Window::Create()` 还会检查
`SceneBoardJudgement::IsSceneBoardEnabled()`，分别创建 `WindowSceneSessionImpl` 或 `WindowImpl`。
见 `wm/src/window.cpp:37-64`、`:75-117`。不能只根据目录名判断设备正在走哪条路径。

## 北向接口谱系

### 布局与状态接口

| ArkTS 接口 | SDK 状态 | 当前语义 |
|---|---|---|
| `setFullScreen(boolean)` | API 6；API 9 起废弃 | 更旧的复合接口：同时处理布局全屏和状态栏显隐；不应作为新代码入口 |
| `setLayoutFullScreen(boolean)` | API 7；API 9 起废弃 | 旧布局接口；替代项为 `setWindowLayoutFullScreen` |
| `setWindowLayoutFullScreen(boolean)` | API 9 起 | 设置主窗或子窗是否进入沉浸式布局；Promise 重载继续使用，旧 callback 重载在 API 12 标记废弃 |
| `setImmersiveModeEnabledState(boolean)` | API 12 起 | 设置沉浸式偏好/中间态；PC 非全屏时可先记录，进入全屏后再应用 |
| `getImmersiveModeEnabledState()` | API 12 起 | 查询上述偏好态 `enableImmersiveMode_`，不保证当前实际布局已经沉浸式 |
| `isImmersiveLayout()` | API 20 起 | 查询当前实际布局是否符合沉浸式判据，是更贴近最新定义的查询接口 |

注意：废弃的是具体旧接口或 callback 重载，不能因为某个 callback 重载废弃，就把同名 Promise 接口整体标为废弃。
上述动态接口在 API 23 起也有对应静态接口；公开名称的准确拼写是 `isImmersiveLayout`（大写 `L`）。

## 当前沉浸式设置规格

以下矩阵以公开 `Window`接口和统一架构当前实现为主。“支持”表示允许进入设置逻辑，不表示系统栏必然隐藏；
系统栏显隐与样式仍由独立接口和 SceneBoard 产品策略决定。

| 接口/对象 | 可以 | 不可以或无效 | 其他约束 |
|---|---|---|---|
| `setWindowLayoutFullScreen(bool)` | 普通主窗口、子窗口可设置布局是否忽略安全区 | 窗口已销毁时报错；统一架构系统窗口返回成功但不生效 | PC/Pad 自由多窗可能旁路为 `SetIgnoreSafeArea()`；非 Phone/Pad 主窗可能同时切换 fullscreen mode |
| `setImmersiveModeEnabledState(bool)` | 仅主窗口、子窗口 | 其他窗口类型由 NAPI 拒绝；窗口无效、无 host session 或不支持 fullscreen mode 时失败 | 非 PC 或当前已 fullscreen 时立即应用布局；PC 非 fullscreen 可只保存偏好；自由多窗 NAPI 旁路不写偏好态 |
| `getImmersiveModeEnabledState()` | 有效的主窗口、子窗口 | 其他类型由 NAPI 拒绝；无效窗口失败 | 只读取 `enableImmersiveMode_`，不保证当前布局已沉浸 |
| `isImmersiveLayout()` | 统一架构有效窗口 | 无效窗口失败；分离架构基类返回设备不支持 | PC/Pad 自由多窗比较实际绘制 rect 与窗口 rect，其他场景读取 `isIgnoreSafeArea_`；自由多窗还要求 UIContent 存在 |
| 系统栏 enable/style 接口 | 按各自窗口类型与权限规则设置状态栏/导航区域 | 不能替代布局沉浸设置 | 只改变 `SystemBarProperty`，最终显示还受 owner 仲裁和 SceneBoard 产品策略控制 |

### `setWindowLayoutFullScreen` 的分支顺序

1. NAPI 先校验 boolean 和窗口生命周期。若处于 PC/Pad 自由多窗且不是兼容沉浸适配应用，直接调用
   `SetIgnoreSafeArea(status)`并返回，不进入常规 `SetLayoutFullScreen()`，见
   `interfaces/kits/napi/window_runtime/window_napi/js_window.cpp:3782-3846`。
2. 统一架构常规入口先拒绝无效 session；系统窗口采用“`WM_OK`但不改变状态”的兼容行为，见
   `wm/src/window_scene_session_impl.cpp:3904-3914`。
3. PC/Pad 自由多窗的兼容沉浸适配应用只应用 safe-area 状态，并可同步兼容模式标题按钮样式；该分支不写
   `property_->IsLayoutFullScreen`或 `enableImmersiveMode_`，见同文件 `:3915-3924`。
4. 其余路径写 `IsLayoutFullScreen`并通知 SceneBoard。普通 Phone/Pad 不因该设置自动改变 window mode；
   非 Phone/Pad 的主窗口会检查 fullscreen mode support，若 `IsFullScreenDisabled()`为 true 则返回成功但不继续应用，
   否则发送 maximize 类 session event 并切换到 `WINDOW_MODE_FULLSCREEN`，见同文件 `:3925-3947`。
5. 最后按应用 `apiCompatibleVersion`分流：API compatible >= 10 写本地 ignore 状态并通知 UIContent；
   API compatible < 10 则增删 `WINDOW_FLAG_NEED_AVOID`并通知 server，见同文件 `:3852-3901`。

因此以下返回值都需要结合状态复核：系统窗口的成功是 no-op；`IsFullScreenDisabled()`分支可成功返回但没有执行
safe-area 设置；自由多窗旁路可能已经改变实际布局，却没有同步偏好态或常规 `layoutFullScreen`属性。

### `setImmersiveModeEnabledState` 的分支顺序

NAPI 明确只允许主窗口和子窗口，见
`interfaces/kits/napi/window_runtime/window_napi/js_window.cpp:10406-10454`。PC/Pad 自由多窗在 NAPI 层直接调用
`SetIgnoreSafeArea(enable)`后返回；由于未调用 native `SetImmersiveModeEnabledState()`，此路径不更新
`enableImmersiveMode_`。

进入统一架构 native 后，还要求 session/host 有效、窗口支持 fullscreen mode。通过后先保存偏好并通知
SceneBoard；非 PC 或已经处于 fullscreen mode 时继续调用 `SetLayoutFullScreen()`，PC 非 fullscreen 时只保存偏好，
见 `wm/src/window_scene_session_impl.cpp:8418-8441`。因此设置值、当前 mode、实际 ignore-safe-area 状态必须分别看。

分离架构存在明确差异：`WindowImpl::SetLayoutFullScreen()`要求窗口有效且支持 fullscreen mode，并总是先切换到
`WINDOW_MODE_FULLSCREEN`；`SetImmersiveModeEnabledState()`仅在当前已经 fullscreen 时应用布局，否则只保存偏好，
见 `wm/src/window_impl.cpp:1256-1300`、`:2659-2679`。分离架构没有覆盖 `IsImmersiveLayout()`，不能用该接口验证实际状态。

### 为什么需要 `setImmersiveModeEnabledState`

分离架构的 `WindowImpl::SetLayoutFullScreen()` 会先调用 `SetWindowMode(WINDOW_MODE_FULLSCREEN)`，再改变安全区处理，
见 `wm/src/window_impl.cpp:1256-1300`。这使“改变布局范围”和“改变窗口模式/尺寸”在旧实现中耦合。

`WindowImpl::SetImmersiveModeEnabledState()`先保存 `enableImmersiveMode_`，仅当窗口已经是 `FULLSCREEN` 时才调用
`SetLayoutFullScreen()`，见 `wm/src/window_impl.cpp:2659-2687`。统一架构也保留该偏好态：PC 非全屏时先记录，
符合设备/窗口模式条件时再应用，见 `wm/src/window_scene_session_impl.cpp:8418-8451`。

因此：

- `getImmersiveModeEnabledState()`回答“请求/偏好态是什么”；
- `isImmersiveLayout()`回答“此刻实际绘制布局是否覆盖整个窗口”；
- 两者在 PC 自由窗口、模式切换或延迟应用阶段可能不同。

## `setWindowLayoutFullScreen` 主链路

统一架构的主要链路是：

```text
ArkTS Window.setWindowLayoutFullScreen(bool)
  -> JsWindow::OnSetWindowLayoutFullScreen
  -> WindowSceneSessionImpl::SetLayoutFullScreen
       -> property_->SetIsLayoutFullScreen
       -> hostSession->OnLayoutFullScreenChange       (通知 SceneBoard)
       -> SetLayoutFullScreenByApiVersion
            -> API compatible >= 10
                 -> UIContent::SetIgnoreViewSafeArea  (本知识链路的 ArkUI 截止点)
            -> API compatible < 10
                 -> WINDOW_FLAG_NEED_AVOID + OnNeedAvoid
```

当前证据：

- NAPI 入口与自由多窗分支：`interfaces/kits/napi/window_runtime/window_napi/js_window.cpp:3782-3846`；
- 客户端状态、SceneBoard 通知和产品/模式判断：`wm/src/window_scene_session_impl.cpp:3904-3955`；
- API 10 兼容分支：`wm/src/window_scene_session_impl.cpp:3852-3901`；
- ArkUI 截止点：关联仓 `arkui_ace_engine/adapter/ohos/entrance/ui_content_impl.cpp:4392-4411`。

这里应止于 `UIContent::SetIgnoreViewSafeArea()`。后续 `PipelineContext`、`SafeAreaManager` 和组件布局属于 ArkUI 的
布局消费实现；只有诊断窗口状态为何没有被消费时才继续进入，不应把这些代码写成 WindowManager 对沉浸式语义的定义。

### 重要分支

- API compatible >= 10：窗口侧保存 `isIgnoreSafeArea_`，有 UIContent 时调用 `SetIgnoreViewSafeArea(status)`。
- API compatible < 10：通过 `WINDOW_FLAG_NEED_AVOID` 和 `OnNeedAvoid` 保持旧应用兼容行为。
- 系统窗口调用统一架构 `SetLayoutFullScreen()`返回成功但不生效，见
  `wm/src/window_scene_session_impl.cpp:3908-3914`。
- PC/Pad 自由多窗且不满足兼容沉浸适配时，NAPI 会直接走 `SetIgnoreSafeArea()`，见
  `interfaces/kits/napi/window_runtime/window_napi/js_window.cpp:3823-3829`。
- 上述 NAPI 旁路不写 `enableImmersiveMode_`；因此该分支下偏好查询与实际绘制结果可能不同，应优先用
  `isImmersiveLayout()`判断实际状态。
- 统一架构的普通 Phone/Pad 路径不会仅因布局沉浸而改变窗口模式；非 Phone/Pad 主窗口存在进入
  `WINDOW_MODE_FULLSCREEN` 的产品分支，见 `wm/src/window_scene_session_impl.cpp:3931-3947`。
- 分离架构的旧实现会先进入 `WINDOW_MODE_FULLSCREEN`，这是排查“接口导致窗口大小变化”时必须区分的路径。

## 更旧的 `setFullScreen`

`setFullScreen()`不是当前沉浸式布局定义的同义词。统一架构实现会：

1. 必要时切换到 `WINDOW_MODE_FULLSCREEN`；
2. 调用 `SetLayoutFullScreenByApiVersion()`；
3. 更新 decor；
4. 将状态栏 `enable_` 设为 `!status`。

见 `wm/src/window_scene_session_impl.cpp:4428-4477`。当前统一架构函数只显式改状态栏；分离架构的
`WindowImpl::SetFullScreen()`会同时处理状态栏和旧导航栏，见 `wm/src/window_impl.cpp:1216-1242`、`:1303-1321`。
因此它同时混合窗口模式、布局安全区和系统栏显隐，且架构间细节不同，已经不适合作为新能力的概念边界。
新代码按目标分别使用布局接口和系统栏接口。

## 相关但独立的辅助控制

### 侧边返回手势

- `setGestureBackEnabled(boolean)` / `isGestureBackEnabled()`自 API 13 提供；
- 它们改变手势返回能力，不改变窗口模式、安全区或系统栏属性；
- 统一架构非 PC 路径会保存本地状态、同步 UIExtension 属性并通知 host session；PC 原生应用返回不支持，
  PC 兼容态设置返回成功但不下发，查询固定为 false；
- SceneSessionManager 还会结合当前全屏、前台/活跃、焦点、recent/PIP 等状态派生最终有效值，见
  `window_scene/session_manager/src/scene_session_manager.cpp:15030-15063`。

见 `wm/src/window_scene_session_impl.cpp:8690-8726`。不要把“关闭侧滑返回”当成“进入沉浸式”的判据。

### PC 标题栏和 Dock 热区悬停

`setTitleAndDockHoverShown(isTitleHoverShown?, isDockHoverShown?)`自 API 14 提供，两个可选参数默认均为 true。它控制
PC/Pad 自由多窗下全屏时鼠标悬停热区是否允许唤出标题栏和 Dock，并不直接执行 show/hide。仅主窗口且产品形态支持时
有效；Pad 非自由多窗的 PC 兼容应用返回成功但不生效，见 `wm/src/window_scene_session_impl.cpp:3970-3996`。
它属于 PC 沉浸体验策略，不等同于布局安全区状态。

## 常见误判

- `WindowMode::FULLSCREEN` 不等于沉浸式布局。
- `enableImmersiveMode_ == true` 不保证当前实际布局已经覆盖整个窗口。
- 分离架构的 `WindowImpl`当前没有覆盖 `IsImmersiveLayout()`；基类会返回设备不支持，见
  `interfaces/innerkits/wm/window.h:5131-5135`。
- 隐藏状态栏不等于忽略安全区；透明状态栏也不等于隐藏状态栏。
- `layoutFullScreen` 的应用真实值与 SceneBoard 派生的产品策略有效值应分开观察。
- `window_scene_board/feature/immersivekeyguard` 是锁屏/SystemUI 功能，不是本文的窗口沉浸主链路。
- ArkUI 自身的 `expandSafeArea`、`ignoreLayoutSafeArea` 等组件能力不是 WindowManager 北向接口的实现定义。
