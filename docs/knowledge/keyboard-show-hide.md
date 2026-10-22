# 输入法窗口（输入法面板）显示与隐藏

## 文档定位

本文是 OpenHarmony 输入法窗口（软键盘面板）显示/隐藏机制的领域知识，覆盖从北向 API 到窗口管理服务端的完整链路。回答以下问题：

- 输入法面板的 `PanelType`/`PanelFlag` 如何映射到窗口管理层的 `WindowType` 和显隐行为；
- 软键盘弹起时，窗口管理如何计算避让区域并通知应用调整布局；
- 统一架构与分离架构下输入法窗口显示/隐藏的实现路径分别是什么；
- 输入法窗口的权限校验、Z-Order 提升和同步事务机制如何工作。

相关参考文档：
- `js-apis-inputmethod-panel.md`：PanelInfo/PanelType/PanelFlag 数据定义
- `capi-inputmethod-text-avoid-info-capi-h.md`：InputMethod_TextAvoidInfo 避让信息 C API
- `window_window_manager/`：窗口管理组件代码与文档

---

## 证据快照

| 仓库 | 关键路径 | source_level | confidence |
|---|---|---|---|
| `js-apis-inputmethod-panel.md` | `@ohos.inputMethod.Panel` 模块 | firsthand | high |
| `capi-inputmethod-text-avoid-info-capi-h.md` | `inputmethod_text_avoid_info_capi.h` | firsthand | high |
| `window_window_manager` | `wm_common.h`, `keyboard_session.cpp/h`, `scene_session_manager.cpp`, `window_controller.cpp`, `window_node_container.cpp`, `avoid_area_controller.cpp`, `window_zorder_policy.h`, `window.h`, `window_scene_session_impl.cpp`, `window_helper.h`, `session_permission.h`, `window_manager_service.cpp` | firsthand | high |

`updated_at`: 2026-09-14

---

## 概述

输入法窗口（输入法面板）的显示与隐藏是 OpenHarmony 窗口管理子系统与输入法框架（IMF）协作的关键链路。输入法应用通过 `InputMethodAbility.createPanel()` 创建面板，面板的 `PanelType` 和 `PanelFlag` 决定了面板的类型和显隐控制方式；窗口管理服务负责将面板映射为系统窗口类型（`WINDOW_TYPE_INPUT_METHOD_FLOAT` 等），管理其生命周期、Z-Order、避让区域计算和动画。

### 涉及的窗口类型

| WindowType | 值 | 说明 |
|---|---|---|
| `WINDOW_TYPE_INPUT_METHOD_FLOAT` | 2103（`ABOVE_APP_SYSTEM_WINDOW_BASE + 3`） | 输入法软键盘浮动窗口，主要面板类型 |
| `WINDOW_TYPE_INPUT_METHOD_STATUS_BAR` | 2122（`ABOVE_APP_SYSTEM_WINDOW_BASE + 22`） | 输入法状态栏窗口 |
| `WINDOW_TYPE_KEYBOARD_PANEL` | 2182（`ABOVE_APP_SYSTEM_WINDOW_BASE + 82`） | 键盘面板窗口（KeyboardPanel，较新类型） |

定义位置：`interfaces/innerkits/wm/wm_common.h:153`、`:172`、`:182`。

### 面板类型与窗口类型映射

| PanelType (IME Kit) | PanelFlag | 映射到的 WindowType | 显隐控制方 |
|---|---|---|---|
| `SOFT_KEYBOARD` (0) | `FLAG_FIXED` (0) | `WINDOW_TYPE_INPUT_METHOD_FLOAT` | 系统控制 |
| `SOFT_KEYBOARD` (0) | `FLAG_FLOATING` (1) | `WINDOW_TYPE_INPUT_METHOD_FLOAT`（gravity=FLOAT） | 系统控制 |
| `SOFT_KEYBOARD` (0) | `FLAG_CANDIDATE` (2) | `WINDOW_TYPE_INPUT_METHOD_FLOAT` 或 `WINDOW_TYPE_KEYBOARD_PANEL` | 开发者控制（`Panel.show()`/`Panel.hide()`） |
| `STATUS_BAR` (1) | 任意 | `WINDOW_TYPE_INPUT_METHOD_STATUS_BAR` | 系统控制 |

---

## 一、code_map（代码地图）

### 关键文件与职责

#### 公共数据结构

| 文件 | 职责 |
|---|---|
| `interfaces/innerkits/wm/wm_common.h` | 定义 `WindowType` 枚举（含 `WINDOW_TYPE_INPUT_METHOD_FLOAT`、`WINDOW_TYPE_INPUT_METHOD_STATUS_BAR`、`WINDOW_TYPE_KEYBOARD_PANEL`）、`KeyboardPanelInfo` 结构体（`:1482`）、`KeyboardEffectOption`、`KeyboardLayoutParams`、`CallingWindowInfo` |
| `utils/include/window_helper.h` | 提供 `IsInputWindow(type)`（`:201`）、`IsKeyboardWindow(type)`（`:207`）、`IsOverlayWindow(type)`（`:187`）等辅助判断函数 |

#### 北向接口（客户端）

| 文件 | 职责 |
|---|---|
| `interfaces/innerkits/wm/window.h` | 定义 `Window` 抽象接口，含 `ShowKeyboard()`（`:5368`）、`ChangeKeyboardEffectOption()`（`:5380`）、`RegisterKeyboardPanelInfoChangeListener()`（`:4814`）、`IKeyboardPanelInfoChangeListener`（`:909`）、`OnKeyboardDidShow/Hide` 回调（`:957`/`:970`） |
| `wm/src/window_scene_session_impl.cpp` | 统一架构下 `WindowSceneSessionImpl::ShowKeyboard()` 实现（`:2380`）、`Hide()` 实现（`:2444`）、`NotifyKeyboardPanelInfoChange()` |
| `wm/src/window_impl.cpp` | 分离架构下 `WindowImpl::ShowKeyboard()` 实现（`:2133`） |
| `wm/include/window_session_impl.h` | `NotifyKeyboardDidShow/Hide`、`WriteKeyboardInfoToWant/ReadKeyboardInfoFromWant` 声明（`:301-303`、`:947-948`） |
| `wm/include/window_scene_session_impl.h` | `NotifyKeyboardPanelInfoChange()` 覆写声明（`:320`）、`IKeyboardPanelInfoChangeListener` 注册/注销（`:795-796`） |

#### 统一架构（合一架构）服务端

| 文件 | 职责 |
|---|---|
| `window_scene/session/host/include/keyboard_session.h` | `KeyboardSession` 类声明，继承 `SystemSession`；定义 `Show()`/`Hide()`/`Disconnect()` 覆写、`SystemKeyboardAvoidChangeReason` 枚举、`KeyboardSessionCallback` 回集结构 |
| `window_scene/session/host/src/keyboard_session.cpp` | `KeyboardSession::Show()`（`:83`）和 `KeyboardSession::Hide()`（`:158`）核心实现 |
| `window_scene/session_manager/include/scene_session_manager.h` | `SceneSessionManager` 声明，含 `isKeyboardPanelEnabled_`（`:1028`）、`CreateKeyboardPanelSession()`（`:1774`）、`RequestKeyboardPanelSession()`（`:1775`）、`RequestInputMethodCloseKeyboard()` |
| `window_scene/session_manager/src/scene_session_manager.cpp` | 键盘会话管理核心：`GetKeyboardSession()`（`:3216`）、`HandleKeyboardAvoidChange()`（`:3235`）、`IsKeyboardForeground()`（`:4166`）、`RequestInputMethodCloseKeyboard()`（`:4179`）、`CheckSystemWindowPermission()`（`:5750`）、`NotifyCreateSpecificSession()`（`:6271`） |
| `window_scene/session_manager/src/scene_session_dirty_manager.cpp` | 键盘窗口触摸热区更新（`:368`、`:392`） |
| `window_scene/common/include/session_permission.h` | `SessionPermission::IsStartedByInputMethod()`（`:46`）、`IsKeyboardCallingProcess()`（`:47`）权限校验 |

#### 分离架构服务端

| 文件 | 职责 |
|---|---|
| `wmserver/src/window_manager_service.cpp` | `CheckSystemWindowPermission()`（`:934`）校验输入法窗口创建权限、`UpdateProperty()` 中输入法窗口操作权限校验（`:1420-1425`） |
| `wmserver/src/window_controller.cpp` | `AddWindowNode()` 中对 INPUT_METHOD_FLOAT 调用 `RelayoutKeyboard()`（`:341`、`:356`）、`RemoveWindowNode()` 中调用 `RestoreCallingWindowSizeIfNeed()`（`:645`）、`ResizeRect()` 处理键盘窗口尺寸变更（`:729`） |
| `wmserver/src/window_node_container.cpp` | `AddNodeOnRSTree()` 键盘窗口动画与同步事务（`:815-822`）、`RunInInputMethodSyncTransaction()`（`:737`）、`NotifyIfKeyboardRegionChanged()`（`:1385`）、`RaiseInputMethodWindowPriorityIfNeeded()`（`:2234`） |
| `wmserver/src/avoid_area_controller.cpp` | `AddOrRemoveKeyboard()` 键盘避让区域管理（`:105`） |
| `wmserver/include/window_zorder_policy.h` | Z-Order 优先级定义：`WINDOW_TYPE_INPUT_METHOD_FLOAT` = 108（`:60`） |
| `wmserver/include/display_zoom_controller.h` | 显示缩放控制器中包含 INPUT_METHOD_FLOAT（`:58`） |

### 高风险入口函数

| 入口函数 | 位置 | 风险点 |
|---|---|---|
| `WindowSceneSessionImpl::ShowKeyboard()` | `wm/src/window_scene_session_impl.cpp:2380` | 客户端入口，设置 callingSessionId 和 effectOption 后调用 `Show()` |
| `KeyboardSession::Show()` | `keyboard_session.cpp:83` | 服务端显示入口，触发避让计算、黑名单设置和前台调度 |
| `KeyboardSession::Hide()` | `keyboard_session.cpp:158` | 服务端隐藏入口，触发会话失活、后台调度和避让恢复 |
| `SceneSessionManager::RequestInputMethodCloseKeyboard()` | `scene_session_manager.cpp:4179` | 应用冷启动时强制隐藏键盘 |
| `SceneSessionManager::HandleKeyboardAvoidChange()` | `scene_session_manager.cpp:3235` | 系统键盘与非系统键盘避让区域切换 |
| `WindowNodeContainer::RaiseInputMethodWindowPriorityIfNeeded()` | `window_node_container.cpp:2234` | 键盘窗口 Z-Order 提升 |
| `AvoidAreaController::AddOrRemoveKeyboard()` | `avoid_area_controller.cpp:105` | 键盘避让区域添加/移除 |
| `WindowController::RelayoutKeyboard()` | `window_controller.cpp:409` | 键盘窗口布局计算 |

---

## 二、routing（知识路由）

### 正确主链路

#### 统一架构：显示链路

```
输入法应用 Panel.show()
  → InputMethodAbility → IMF 服务
  → Window::ShowKeyboard(callingWindowId, targetDisplayId, effectOption)
    → WindowSceneSessionImpl::ShowKeyboard()          [wm/src/window_scene_session_impl.cpp:2380]
      → 设置 CallingSessionId / KeyboardTargetDisplayId / KeyboardEffectOption
      → 调用 Show()
        → IPC → SceneSessionManager
          → KeyboardSession::Show(property)           [keyboard_session.cpp:83]
            → CheckPermissionWithPropertyAnimation()
            → PostTask:
              → 通知 SystemKeyboardAvoidChangeReason::KEYBOARD_SHOW（若 gravity=BOTTOM）
              → 设置 KeyboardEffectOption 和 KeyboardTargetDisplayId
              → UseFocusIdIfCallingSessionIdInvalid()
              → SetSessionBlackListWhenShow() / SetSkipEventOnCastPlus()
              → SceneSession::Foreground(property)     → 前台显示
```

#### 统一架构：隐藏链路

```
输入法应用 Panel.hide() 或系统请求隐藏
  → Window::Hide() / WindowSceneSessionImpl::Hide()
    → IPC → SceneSessionManager
      → KeyboardSession::Hide()                       [keyboard_session.cpp:158]
        → CheckPermissionWithPropertyAnimation()
        → PostTask:
          → SetActive(false)                          → 会话失活
          → SceneSession::Background()                → 后台
          → NotifyKeyboardPanelInfoChange({0,0,0,0}, false)  → 通知面板信息变更
          → PC/虚拟屏: RestoreCallingSession() 或 NotifySystemKeyboardAvoidChange(KEYBOARD_HIDE)
          → SetCallingSessionId(INVALID_WINDOW_ID)
```

#### 统一架构：应用冷启动隐藏键盘

```
应用冷启动
  → SceneSessionManager::RequestInputMethodCloseKeyboard(persistentId)  [scene_session_manager.cpp:4179]
    → 检查 processOptions（startupVisibility != HIDE）
    → GetKeyboardSession(displayId, false)
    → 检查 !IsSessionValid() && IsKeyboardForeground() && !SCREEN_LOCKED
    → sceneSession->RequestHideKeyboard(true)         → 请求隐藏键盘
```

#### 分离架构：显示链路

```
Window::Show() → IPC → WindowManagerService
  → WindowController::AddWindowNode()
    → RelayoutKeyboard(node)                          [window_controller.cpp:409]
      → 读取 gravity，若非 FLOAT 则计算键盘位置（考虑导航栏高度）
    → WindowRoot::AddWindowNode()
    → 若 INPUT_METHOD_FLOAT: ResizeSoftInputCallingWindowIfNeed(node)
    → WindowNodeContainer::AddNodeOnRSTree()
      → 若 INPUT_METHOD_FLOAT 且 gravity != FLOAT:
        RunInInputMethodSyncTransaction()              [window_node_container.cpp:737]
          → AutoRSSyncTransaction 包裹
          → RSNode::Animate(keyboardAnimationIn_)     → 键盘弹出动画
    → RaiseInputMethodWindowPriorityIfNeeded(node)     [window_node_container.cpp:2234]
    → NotifyIfKeyboardRegionChanged()                  [window_node_container.cpp:1385]
      → 计算 keyboard 与 callingWindow 的 overlap
      → callingWindow->GetWindowToken()->UpdateOccupiedAreaChangeInfo(TYPE_INPUT, ...)
```

#### 分离架构：隐藏链路

```
Window::Hide() → IPC → WindowManagerService
  → WindowController::RemoveWindowNode()
    → 若 INPUT_METHOD_FLOAT: RestoreCallingWindowSizeIfNeed()  [window_controller.cpp:645]
    → WindowNodeContainer::RemoveNodeFromRSTree()
      → 若 INPUT_METHOD_FLOAT 且 gravity != FLOAT:
        RunInInputMethodSyncTransaction()
          → RSNode::Animate(keyboardAnimationOut_)     → 键盘收起动画
    → AvoidAreaController::AddOrRemoveKeyboard(node, false)  → 移除键盘避让
```

### 避让区域计算链路

```
键盘窗口 Show/Hide 或尺寸变更
  → 分离架构: AvoidAreaController → GetAvoidAreaByType(TYPE_KEYBOARD) → UpdateAvoidAreaIfNeed()
  → 统一架构: KeyboardSession → ProcessKeyboardOccupiedAreaInfo() → NotifyOccupiedAreaChanged()
    → 计算 keyboard rect 与 callingSession rect 的 overlap
    → 生成 OccupiedAreaChangeInfo(TYPE_INPUT, overlapRect, textFieldPositionY, textFieldHeight)
    → 通知 callingSession 的 WindowToken
      → 应用层收到 on('avoidAreaChange') 回调 / OccupiedAreaChangeInfo
```

### PanelFlag 与显隐控制路由

| PanelFlag | 显示触发 | 隐藏触发 | 路由路径 |
|---|---|---|---|
| `FLAG_FIXED` | 系统检测到输入框获焦 | 系统检测到输入框失焦 / 应用切换 | IMF → Window::ShowKeyboard() → KeyboardSession::Show() |
| `FLAG_FLOATING` | 系统检测到输入框获焦 | 系统控制 | 同 FLAG_FIXED，但 gravity=FLOAT |
| `FLAG_CANDIDATE` | 开发者调用 `Panel.show()` | 开发者调用 `Panel.hide()` | 输入法应用自行管理，不经过系统自动调度 |

### TextAvoidInfo 路由

```
应用设置编辑框信息
  → OH_TextAvoidInfo_Create(positionY, height)        [C API, 起始版本 12]
  → OH_TextAvoidInfo_SetPositionY() / OH_TextAvoidInfo_SetHeight()
  → 写入 InputMethod_TextConfig（作为子属性）
    → OH_TextConfig_GetTextAvoidInfo() 获取
    → 输入法框架根据 positionY + height 计算避让区域
      → 窗口管理服务据此调整编辑框位置（上移或重新布局）
```

### 任务场景路由

| 场景 | 先读文件 | 先看代码 |
|---|---|---|
| 修改键盘弹出/收起动画 | `window_zorder_policy.h`、`keyboard_session.cpp` | `window_node_container.cpp:815-822`、`keyboard_session.cpp:83-129` |
| 修改键盘避让区域计算 | `wm_common.h`(AvoidArea/KeyboardPanelInfo) | `avoid_area_controller.cpp:105-139`、`keyboard_session.cpp`(`ProcessKeyboardOccupiedAreaInfo`) |
| 修改键盘窗口权限校验 | `session_permission.h` | `scene_session_manager.cpp:5750-5828`、`window_manager_service.cpp:934-965` |
| 修改键盘 Z-Order | `window_zorder_policy.h` | `window_node_container.cpp:2234-2265` |
| 修改键盘面板信息通知 | `window.h`(IKeyboardPanelInfoChangeListener) | `window_scene_session_impl.cpp`(`NotifyKeyboardPanelInfoChange`)、`keyboard_session.cpp:99` |
| 修改系统键盘避让切换 | `keyboard_session.h`(SystemKeyboardAvoidChangeReason) | `scene_session_manager.cpp:3235-3288` |
| 修改键盘面板创建 | `scene_session_manager.h:1774-1775` | `scene_session_manager.cpp:3309-3319`、`6291-6293` |
| 修改键盘触摸热区 | `scene_session_dirty_manager.cpp` | `scene_session_dirty_manager.cpp:368-418` |

---

## 三、expert（专家经验）

### 禁止事项

- **禁止** 非 `SessionPermission::IsStartedByInputMethod()` 的调用者创建或操作 `WINDOW_TYPE_INPUT_METHOD_FLOAT` / `WINDOW_TYPE_INPUT_METHOD_STATUS_BAR` 窗口。窗口管理服务端在 `CheckSystemWindowPermission()` 中会拒绝非输入法应用的创建请求（`scene_session_manager.cpp:5770-5775`、`window_manager_service.cpp:941-946`）。

- **禁止** 非输入法应用自身隐藏键盘窗口。`UpdateProperty()` 中明确校验：`WINDOW_TYPE_INPUT_METHOD_FLOAT` 和 `WINDOW_TYPE_INPUT_METHOD_STATUS_BAR` 类型的窗口只有 `Permission::IsStartByInputMethod()` 为 true 时才允许操作（`window_manager_service.cpp:1420-1425`）。

- **禁止** 系统键盘窗口被非虚拟键盘服务创建。`WINDOW_TYPE_INPUT_METHOD_FLOAT` + `IsSystemKeyboard() == true` 时需要 `ohos.permission.VIRTUAL_KEYBOARD_WINDOW` 权限（`scene_session_manager.cpp:5761-5768`）。

- **禁止** 在键盘 Show/Hide 的 PostTask 回调中同步阻塞等待其他 IPC。`KeyboardSession::Show()` 和 `Hide()` 均使用 `PostTask` 异步执行，在回调中调用同步 IPC 会死锁。

- **禁止** 修改 `WINDOW_TYPE_INPUT_METHOD_FLOAT` 的枚举值（2103）。该值已对外发布，修改将破坏 IPC 和 API 兼容性。

- **禁止** 只修改分离架构或合一架构中的一条路径。输入法窗口的显隐行为在两套架构下必须保持一致。

- **禁止** 在键盘避让区域计算中混用未转换的窗口坐标系与屏幕坐标系。`positionY`（TextAvoidInfo）是相对于物理屏幕的绝对 Y 坐标。

### 架构约束

- **双架构必须同步评估**：输入法窗口的显示/隐藏链路在分离架构（`wmserver/`）和合一架构（`window_scene/`）中各有一套实现，修改公共语义时必须同时评估两条路径。

- **KeyboardSession 是合一架构的键盘核心类**：继承自 `SystemSession`，覆写了 `Show()`/`Hide()`/`Disconnect()` 等生命周期方法。分离架构没有对应的 KeyboardSession，直接在 `WindowController` 和 `WindowNodeContainer` 中处理。

- **KeyboardPanel 机制是较新的能力**：当 `isKeyboardPanelEnabled_` 为 true 时，`WINDOW_TYPE_INPUT_METHOD_FLOAT` 会创建为 `KeyboardSession` 并绑定 `KeyboardPanelSession`（`WINDOW_TYPE_KEYBOARD_PANEL`）；否则创建为普通 `SystemSession`（`scene_session_manager.cpp:6291-6293`）。

- **系统键盘与非系统键盘的避让切换**：在 PC 设备上，系统键盘（`IsSystemKeyboard() == true`）的避让区域有独立激活/去激活逻辑。系统键盘显示时激活其避让区域、去激活非系统键盘的避让区域；反之亦然（`scene_session_manager.cpp:3235-3288`）。

- **Z-Order 动态提升**：输入法窗口的 Z-Order 不是固定值 108。`RaiseInputMethodWindowPriorityIfNeeded()` 会根据以下条件动态调整：
  - 锁屏时：priority = keyguard + 2（高于锁屏和 show-when-locked 窗口）
  - calling window 是焦点窗口时：priority = max(108, callingWindowPriority + 1)
  - 焦点窗口是 PANEL 时：priority = panel + 1

- **同步事务保护**：键盘显示/隐藏动画在分离架构中使用 `RunInInputMethodSyncTransaction()`（内部使用 `AutoRSSyncTransaction`）包裹，确保 RS 树更新和动画在同一帧完成。合一架构使用 `RSTransaction` 实现类似效果。

### 隐式约束

- `KeyboardPanelInfo` 的序列化字段顺序不可变更。`Marshalling()` 中 rect、beginRect、endRect、gravity、isShowing 的写入顺序必须与 `Unmarshalling()` 的读取顺序一致（`wm_common.h:1489-1517`）。

- `KeyboardSession::Show()` 中的 `PostTask` 是异步的，调用方在 `Show()` 返回 `WS_OK` 后并不代表键盘已可见。需要在回调（`OnKeyboardDidShow`）中确认。

- `FLAG_CANDIDATE` 面板的显隐完全由开发者控制，系统不会主动调度。如果开发者未实现显隐逻辑，候选词面板将不会自动显示或隐藏。

- `TextAvoidInfo` 的 `positionY` 和 `height` 传入负值不会报错，但在实际避让计算中无意义。`OH_TextAvoidInfo_Create` 的返回值可能为 NULL（内存分配失败），后续 Set/Get 操作会返回 `IME_ERR_NULL_POINTER`。

### 常见错误改法

| 错误改法 | 正确做法 |
|---|---|
| 在输入法窗口的 Show/Hide 回调中直接调用同步 IPC 获取焦点窗口信息 | 使用 `KeyboardSessionCallback` 中预注册的 `onGetSceneSession` / `onGetFocusedSessionId` 回调获取 |
| 修改 `window_zorder_policy.h` 中 INPUT_METHOD_FLOAT 的优先级值来解决遮挡 | 使用 `RaiseInputMethodWindowPriorityIfNeeded()` 中已有的动态提升逻辑，或在产品配置中调整 |
| 在 `AvoidAreaController` 中直接修改 keyboard avoid area 而不通知 calling window | 必须通过 `UpdateOccupiedAreaChangeInfo()` 通知 calling window 的 WindowToken |
| 在 `KeyboardSession::Hide()` 中直接设置 `CallingSessionId = 0` 而不恢复 calling session | 必须在 PC/虚拟屏场景中调用 `RestoreCallingSession()` 或 `NotifySystemKeyboardAvoidChange(KEYBOARD_HIDE)` |
| 只在合一架构中测试键盘显示/隐藏，忽略分离架构 | 必须在两套架构下分别验证 |

### 失败模式

- **键盘不弹出**：检查 `CheckPermissionWithPropertyAnimation()` 是否返回 `WS_ERROR_NOT_SYSTEM_APP`，确认调用者是否为输入法应用（`IsStartedByInputMethod()`）。
- **避让区域不更新**：检查 `callingWindow` 是否为 NULL（`NotifyIfKeyboardRegionChanged` 中 `FindWindowNodeById` 可能返回 nullptr），回退到焦点窗口。
- **键盘动画闪烁**：检查 `isAnimateTransactionEnabled_` 是否为 false，导致 `RunInInputMethodSyncTransaction` 直接执行 task 而不包裹同步事务。
- **系统键盘与非系统键盘避让冲突**：在 PC 设备上，检查 `HandleKeyboardAvoidChange` 是否正确处理了 `KEYBOARD_SHOW`/`KEYBOARD_HIDE`/`KEYBOARD_GRAVITY_BOTTOM`/`KEYBOARD_GRAVITY_FLOAT` 的切换。
- **TextAvoidInfo 内存泄漏**：`OH_TextAvoidInfo_Create` 后未调用 `OH_TextAvoidInfo_Destroy`。

---

## 四、verify（编译和测试方法）

### 最小编译命令

从 OpenHarmony 源码根目录执行：

```sh
# 完整窗口管理部件
./build.sh --product-name rk3568 --build-target window_manager --ccache

# 分离架构窗口服务
./build.sh --product-name rk3568 --build-target wmserver --ccache

# 合一架构窗口/屏幕服务
./build.sh --product-name rk3568 --build-target window_scene --ccache
```

### 最小测试命令

```sh
# 窗口客户端测试（含 ShowKeyboard 接口测试）
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 WindowInputMethodTest
# 测试文件: test/systemtest/wms/window_input_method_test.cpp

# 分离架构窗口服务测试
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 WindowControllerTest
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 WindowNodeContainerTest

# 合一架构测试
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 SceneSessionManagerTest
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 SceneSessionDirtyManagerTest
prebuilts/build-tools/linux-x86/bin/ninja -C out/rk3568 KeyboardSessionTest
```

GN 测试标签参考：

| 测试目标 | 标签 |
|---|---|
| 窗口客户端 | `//foundation/window/window_manager/wm:test` |
| 分离架构窗口服务 | `//foundation/window/window_manager/wmserver:test` |
| 合一架构窗口/屏幕服务 | `//foundation/window/window_manager/window_scene:test` |
| 避让区域控制器 | `//foundation/window/window_manager/wmserver:test`（`AvoidAreaControllerTest`） |
| fuzz 目标 | `//foundation/window/window_manager/test:fuzztest` |

### 验证通过标准

1. **编译通过**：至少构建受影响模块（`wm`/`wmserver`/`window_scene`）；公共接口或公共数据结构改动需构建所有直接消费者。
2. **单元测试通过**：对应单元测试通过，包括：
   - `WindowInputMethodTest.ShowKeyboard01`：验证键盘窗口创建和显示
   - `WindowControllerTest`：验证输入法窗口添加/移除/布局
   - `WindowNodeContainerTest`：验证 Z-Order 提升和同步事务
   - `AvoidAreaControllerTest`：验证键盘避让区域计算
   - `SceneSessionManagerTest`：验证键盘会话管理和避让切换
3. **板侧验证**：涉及键盘弹出/收起、避让区域、动画的改动需在真机上验证：
   - 弹出键盘，确认应用内容上移避让
   - 收起键盘，确认应用内容恢复
   - 横竖屏切换时键盘正确布局
   - 锁屏时键盘可弹出于锁屏上方
   - 多窗口/分屏下键盘避让正确
4. **双架构验证**：修改公共语义时，在分离架构和合一架构下分别验证。

### 关键观测点

| 观测点 | 日志 Tag | 位置 |
|---|---|---|
| 键盘 Show 调用入口 | `WmsLogTag::WMS_KEYBOARD` | `window_scene_session_impl.cpp:2383` |
| KeyboardSession::Show 执行 | `WmsLogTag::WMS_KEYBOARD` | `keyboard_session.cpp:119-125` |
| KeyboardSession::Hide 执行 | `WmsLogTag::WMS_KEYBOARD` | `keyboard_session.cpp:170` |
| 键盘会话查找 | `WmsLogTag::WMS_KEYBOARD` | `scene_session_manager.cpp:3219-3232` |
| 系统键盘避让切换 | `WmsLogTag::WMS_KEYBOARD` | `scene_session_manager.cpp:3235-3288` |
| 权限校验失败 | `WmsLogTag::WMS_KEYBOARD` | `scene_session_manager.cpp:5767-5768`、`window_manager_service.cpp:1422-1424` |
| 冷启动隐藏键盘 | `WmsLogTag::WMS_KEYBOARD` | `scene_session_manager.cpp:4198-4203` |
| 键盘区域变更通知 | `WmsLogTag::WMS_KEYBOARD` | `window_node_container.cpp:1447-1450` |
| 键盘面板信息变更 | `WmsLogTag::WMS_KEYBOARD` | `keyboard_session.cpp:179` |

### HiLog 过滤命令

```sh
# 过滤键盘相关日志
hilog -T WmsLogTag | grep -i "KEYBOARD"

# 过滤输入法窗口权限日志
hilog -T WmsLogTag | grep -i "INPUT_METHOD"

# 查看键盘避让区域变更
hilog | grep -E "OccupiedArea|AvoidArea|keyboard"
```

### HiDumper 查看键盘窗口

```sh
# 查看当前所有窗口
hidumper -s WindowManagerService -a "-a"

# 查看窗口树
hidumper -s WindowManagerService -a "-w"

# 查看特定窗口信息
hidumper -s WindowManagerService -a "-w <windowId>"
```

---

## 附录：相关术语

| 术语 | 含义 |
|---|---|
| **IME** | Input Method Engine，输入法引擎应用 |
| **IMF** | Input Method Framework，输入法框架服务 |
| **PanelInfo** | 输入法面板属性信息，含 `type`（PanelType）和 `flag`（PanelFlag） |
| **PanelType** | 面板类型枚举：`SOFT_KEYBOARD`(0)、`STATUS_BAR`(1) |
| **PanelFlag** | 面板状态枚举：`FLAG_FIXED`(0)、`FLAG_FLOATING`(1)、`FLAG_CANDIDATE`(2) |
| **KeyboardSession** | 合一架构中键盘会话类，继承 SystemSession，管理键盘窗口生命周期 |
| **KeyboardPanelSession** | 键盘面板会话，与 KeyboardSession 绑定，类型为 `WINDOW_TYPE_KEYBOARD_PANEL` |
| **TextAvoidInfo** | 输入框避让信息（C API），含 positionY 和 height，用于键盘避让计算 |
| **KeyboardPanelInfo** | 键盘面板信息结构体，含 rect/beginRect/endRect/gravity/isShowing，用于面板信息通知 |
| **SystemKeyboardAvoidChangeReason** | 系统键盘避让变更原因枚举：KEYBOARD_BEGIN/CREATED/SHOW/HIDE/DISCONNECT/GRAVITY_BOTTOM/GRAVITY_FLOAT/KEYBOARD_END |
| **OccupiedAreaChangeInfo** | 占用区域变更信息，键盘弹出时通知应用避让区域 |
| **CallingWindow / CallingSession** | 触发键盘弹出的应用窗口/会话 |
| **isKeyboardPanelEnabled_** | 是否启用键盘面板机制，为 true 时 INPUT_METHOD_FLOAT 创建为 KeyboardSession |
