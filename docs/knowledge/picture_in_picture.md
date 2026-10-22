# HarmonyOS 画中画（PiP）知识库

> 本知识库整合自华为 HarmonyOS 官方开发者文档，涵盖画中画功能的概念、开发指南、API 参考、设计规范与常见问题。
>
> 来源文档：
> - [画中画开发概述](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/pipwindow-overview)
> - [使用XComponent实现画中画功能开发](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/pipwindow-xcomponent)
> - [使用typeNode实现画中画功能开发](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/pipwindow-typenode)
> - [使用NDK接口实现画中画功能开发](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/pipwindow-native)
> - [画中画常见问题](https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/pip-faqs)
> - [@ohos.PiPWindow (画中画窗口) API参考](https://developer.huawei.com/consumer/cn/doc/harmonyos-references/js-apis-pipwindow)
> - [画中画设计规范](https://developer.huawei.com/consumer/cn/doc/design-guides/pip-0000001927422624)

---

## 目录

- [1. 概述](#1-概述)
  - [1.1 场景介绍](#11-场景介绍)
  - [1.2 三种开发方式](#12-三种开发方式)
  - [1.3 约束与限制（通用）](#13-约束与限制通用)
- [2. 交互方式与控制层](#2-交互方式与控制层)
  - [2.1 窗口交互方式](#21-窗口交互方式)
  - [2.2 控制层功能](#22-控制层功能)
  - [2.3 配置控制层可选控件](#23-配置控制层可选控件)
  - [2.4 自定义UI](#24-自定义ui)
  - [2.5 更新控件状态](#25-更新控件状态)
- [3. 开发方式详解](#3-开发方式详解)
  - [3.1 使用XComponent实现（ArkTS）](#31-使用xcomponent实现arkts)
  - [3.2 使用typeNode实现（ArkTS）](#32-使用typenode实现arkts)
  - [3.3 使用NDK接口实现（C/C++）](#33-使用ndk接口实现cc)
- [4. API 参考：@ohos.PiPWindow](#4-api-参考ohospipwindow)
  - [4.1 模块导入](#41-模块导入)
  - [4.2 PiPWindow.isPiPEnabled](#42-pipwindowispipenabled)
  - [4.3 PiPWindow.create](#43-pipwindowcreate)
  - [4.4 PiPConfiguration 参数说明](#44-pipconfiguration-参数说明)
  - [4.5 枚举类型](#45-枚举类型)
  - [4.6 PiPController 接口](#46-pipcontroller-接口)
  - [4.7 事件监听接口](#47-事件监听接口)
  - [4.8 NDK 接口一览](#48-ndk-接口一览)
- [5. 设计规范](#5-设计规范)
  - [5.1 触发方式](#51-触发方式)
  - [5.2 窗口大小调节](#52-窗口大小调节)
  - [5.3 窗口最小化](#53-窗口最小化)
  - [5.4 控制面板结构](#54-控制面板结构)
  - [5.5 视觉规格](#55-视觉规格)
- [6. 常见问题](#6-常见问题)

---

## 1. 概述

### 1.1 场景介绍

应用在视频播放、视频会议、视频通话等场景下，可以使用画中画能力将视频内容以小窗（画中画）模式呈现。切换为小窗模式后，用户可以进行其他界面操作，提升使用体验。

画中画的常见使用场景：

- 视频播放
- 视频通话
- 视频会议
- 直播

### 1.2 三种开发方式

| 开发方式 | 适用场景 | 特点 |
|:---------|:---------|:-----|
| **XComponent** | 应用通过 Navigation 管理页面或 Ability 单页面 | 无需应用管理页面 |
| **typeNode** | 所有场景（推荐） | 灵活性高，需应用自行管理页面 |
| **NDK 接口** | 依赖 NDK 接口开发的应用 | 需应用自行管理页面 |

### 1.3 约束与限制（通用）

- 基于安全考虑，应用处于后台时不允许通过 `startPiP` 启动画中画。针对应用返回后台时需要启动画中画的场景，建议使用 `setAutoStartEnabled(true)` 实现自动启动。
- 系统能力：`SystemCapability.Window.SessionManager`，使用前请先用 `canIUse()` 判断当前设备是否支持。
- 本模块首批接口从 API version 11 开始支持。后续版本新增接口采用上角标单独标记。

---

## 2. 交互方式与控制层

### 2.1 窗口交互方式

画中画窗口提供以下交互方式：

- **单击**：如果控制层未显示，则显示控制层，3秒后自动隐藏；如果已显示，则隐藏控制层。
- **双击**：放大或缩小画中画窗口。
- **拖动**：可将窗口拖动到屏幕任意位置。拖动到屏幕左右边缘时窗口自动隐藏，边缘显示隐藏图标，点击后恢复显示。
- **拖拽缩放**：拖拽窗口四边及四个对角缩放大小，不能超过默认最大档与最小档，超出限制会触发回弹效果。
- **拖动删除**：将窗口拖动到底部垃圾桶热区删除窗口。对于申请长时任务的应用，需主动监听 `STOPPED` 事件关闭任务或进程。

### 2.2 控制层功能

控制层提供以下功能：

- **窗口控制**：包括"关闭"和"恢复全屏窗口"功能。
- **内容控制**：根据不同场景呈现不同控件，应用可按需设置。

### 2.3 配置控制层可选控件

通过 `PiPConfiguration` 中的 `controlGroups`（`PiPControlGroup` 类型数组）配置控制层控件。各场景可选控件如下：

#### 视频播放场景（VideoPlayControlGroup）

| 控件组 | 值 | 说明 |
|:-------|:--|:-----|
| VIDEO_PREVIOUS_NEXT | 101 | 视频上一个/下一个控件组，与快进/后退互斥 |
| FAST_FORWARD_BACKWARD | 102 | 视频快进/后退控件组，与上一个/下一个互斥 |

#### 视频通话场景（VideoCallControlGroup）

| 控件组 | 值 | 说明 |
|:-------|:--|:-----|
| MICROPHONE_SWITCH | 201 | 打开/关闭麦克风 |
| HANG_UP_BUTTON | 202 | 挂断 |
| CAMERA_SWITCH | 203 | 打开/关闭摄像头 |
| MUTE_SWITCH | 204 | 静音 |

> 若不配置，视频通话模板默认无任何按钮，点击画中画窗口即可启动还原。

#### 视频会议场景（VideoMeetingControlGroup）

| 控件组 | 值 | 说明 |
|:-------|:--|:-----|
| HANG_UP_BUTTON | 301 | 挂断 |
| CAMERA_SWITCH | 302 | 打开/关闭摄像头 |
| MUTE_SWITCH | 303 | 静音 |
| MICROPHONE_SWITCH | 304 | 打开/关闭麦克风 |

> 若不配置，视频会议模板默认无任何按钮，点击画中画窗口即可启动还原。

#### 直播场景（VideoLiveControlGroup）

| 控件组 | 值 | 说明 |
|:-------|:--|:-----|
| VIDEO_PLAY_PAUSE | 401 | 播放/暂停直播 |
| MUTE_SWITCH | 402 | 静音 |

> `controlGroups` 最多可选三个控件，超出三个 `create` 接口抛出 401 错误码。`controlGroups` 必须与 `templateType` 匹配，否则抛出 401 错误码。

### 2.4 自定义UI

通过 `PiPConfiguration` 中传入 `customUIController`（`NodeController` 类型）可在画中画内容上方展示自定义UI。

> **注意**：自定义显示的UI无法响应交互事件。

### 2.5 更新控件状态

- `updatePiPControlStatus(controlType, status)`：更新控件功能状态（如播放→暂停）。
- `setPiPControlEnabled(controlType, enabled)`：设置控件使能状态（如可点击→不可点击）。

---

## 3. 开发方式详解

### 3.1 使用XComponent实现（ArkTS）

> 适用于应用通过 Navigation 管理页面或 Ability 单页面的场景，无需应用管理页面。

#### 约束与限制

- HarmonyOS 6.0.0 之前，支持在 Phone、Tablet 设备使用；从 6.0.0 开始支持 Phone、PC/2in1、Tablet。
- 仅支持以 XComponent 作为媒体流播放组件，`type` 必须为 `XComponentType.SURFACE`。
- UIAbility 使用 Navigation 管理页面时，需设置 Navigation 控件的 id 属性并传给画中画控制器，确保还原时恢复正常恢复原页面。
- 如果应用主窗口不在前台，不建议在画中画回调方法中执行 UI 操作（如页面 push/pop）。
- 关闭画中画时需检查自定义组件节点是否释放，避免内存泄漏。

#### 开发步骤

**步骤1：创建画中画控制器，配置参数**

```typescript
import { PiPWindow } from '@kit.ArkUI';

function getControlGroups(templateType: PiPWindow.PiPTemplateType): PiPWindow.PiPControlGroup[] {
  switch (templateType) {
    case PiPWindow.PiPTemplateType.VIDEO_PLAY:
      return [PiPWindow.VideoPlayControlGroup.VIDEO_PREVIOUS_NEXT];
    case PiPWindow.PiPTemplateType.VIDEO_CALL:
      return [PiPWindow.VideoCallControlGroup.MICROPHONE_SWITCH,
        PiPWindow.VideoCallControlGroup.HANG_UP_BUTTON, PiPWindow.VideoCallControlGroup.CAMERA_SWITCH];
    case PiPWindow.PiPTemplateType.VIDEO_MEETING:
      return [PiPWindow.VideoMeetingControlGroup.MICROPHONE_SWITCH,
        PiPWindow.VideoMeetingControlGroup.HANG_UP_BUTTON, PiPWindow.VideoMeetingControlGroup.CAMERA_SWITCH];
    case PiPWindow.PiPTemplateType.VIDEO_LIVE:
      return [PiPWindow.VideoLiveControlGroup.VIDEO_PLAY_PAUSE,
        PiPWindow.VideoLiveControlGroup.MUTE_SWITCH];
    default:
      return [];
  }
}

@Component
export struct Page1 {
  @State currentTemplateType: PiPWindow.PiPTemplateType = PiPWindow.PiPTemplateType.VIDEO_PLAY;
  private pipController: PiPWindow.PiPController | undefined = undefined;
  private mXComponentController: XComponentController = new XComponentController();
  private navId: string = 'page_1';

  startPip() {
    if (!PiPWindow.isPiPEnabled()) {
      console.error('picture in picture disabled for current OS');
      return;
    }
    let config: PiPWindow.PiPConfiguration = {
      context: this.getUIContext().getHostContext() as Context,
      componentController: this.mXComponentController,
      navigationId: this.navId,           // Navigation管理页面时需设置
      templateType: this.currentTemplateType,
      contentWidth: 1920,                  // 可选，设置画中画窗口比例
      contentHeight: 1080,                 // 可选
      controlGroups: getControlGroups(this.currentTemplateType),
      // customUIController: this.nodeController,  // 可选，自定义UI
    };
    PiPWindow.create(config).then((controller: PiPWindow.PiPController) => {
      this.pipController = controller;
      this.initPipController();
      this.pipController.startPiP().then(() => {
        console.info('Succeeded in starting pip.');
      }).catch((err: BusinessError) => {
        console.error(`Failed to start pip. Cause:${err.code}, message:${err.message}`);
      });
    }).catch((err: BusinessError) => {
      console.error(`Failed to create pip controller. Cause:${err.code}, message:${err.message}`);
    });
  }

  initPipController() {
    // 注册生命周期事件
    this.pipController?.on('stateChange', (state: PiPWindow.PiPState, reason: string) => {
      this.onStateChange(state, reason);
    });
    // 注册控制事件
    this.pipController?.on('controlPanelActionEvent', (event: PiPWindow.PiPActionEventType, status?: number) => {
      this.onActionEvent(event, status);
    });
  }
}
```

**步骤2：设置自动启动（可选）**

```typescript
this.pipController.setAutoStartEnabled(true); // 应用主窗退后台时自动启动画中画，默认false
```

> 开启自动拉起时，若应用主窗为智慧多窗悬浮窗状态且被收入侧边栏，虽退后台但不会自动拉起画中画。
> 结合 Navigation 路由管理时，首次调用 `setAutoStartEnabled(true)`，系统会缓存 NavigationId 的栈顶信息。

**步骤3：更新媒体源尺寸**

```typescript
this.pipController.updateContentSize(900, 1600); // 媒体源切换后更新尺寸
```

**步骤4：关闭画中画**

```typescript
stopPip() {
  this.pipController?.stopPiP().then(() => {
    console.info('Succeeded in stopping pip.');
    this.pipController?.off('stateChange');
    this.pipController?.off('controlPanelActionEvent');
  }).catch((err: BusinessError) => {
    console.error(`Failed to stop pip. Cause:${err.code}, message:${err.message}`);
  });
}
```

#### 自定义UI实现

```typescript
class TextNodeController extends NodeController {
  private message: string;
  private textNode: BuilderNode<[Params]> | null = null;

  constructor(message: string) {
    super();
    this.message = message;
  }

  makeNode(context: UIContext): FrameNode | null {
    this.textNode = new BuilderNode(context);
    this.textNode.build(wrapBuilder<[Params]>(buildText), new Params(this.message));
    return this.textNode.getFrameNode();
  }

  update(message: string) {
    if (this.textNode !== null) {
      this.textNode.update(new Params(message));
    }
  }

  dispose() {
    if (this.textNode !== null) {
      this.textNode.dispose();
    }
  }
}

class Params {
  public text: string = '';
  constructor(text: string) { this.text = text; }
}

@Builder
function buildText(params: Params) {
  Column() {
    Text(params.text).fontSize(20).fontColor(Color.Red)
  }.width('100%').height('100%')
}
```

---

### 3.2 使用typeNode实现（ArkTS）

> 适用于任意场景，灵活性高，**推荐使用**。从 API version 12 开始支持。
>
> 设备支持：Phone、Tablet、PC/2in1、TV、Car。

#### 约束与限制

- 构造 `PiPConfiguration` 时，建议传入 `contentWidth` 和 `contentHeight`，否则系统以 16:9 呈现。
- `contentNode` 支持 `XComponentType.SURFACE` 类型，创建 typeNode 时必须指定为 "XComponent" 类型。
- 关闭画中画时需检查自定义组件节点是否释放，避免内存泄漏。

#### 开发步骤

**步骤1：创建typeNode节点**

typeNode 可选择是否添加到布局中：

*方式A：作为自由节点，不添加到布局（简单场景）*

```typescript
makeTypeNode(ctx: UIContext): void {
  if (this.xComponent === null || this.xComponent === undefined) {
    this.xComponent = typeNode.createNode(ctx, 'XComponent', {
      type: XComponentType.SURFACE,
      controller: this.getXComponentController(),
    });
  }
}
```

*方式B：添加到布局中（主页预览→画中画切换场景）*

```typescript
export class XCNodeController extends NodeController {
  public xComponent: typeNode.XComponent | null = null;
  private node: FrameNode | null = null;
  private canAddNode: boolean = true;

  setCanAddNode(canAddNode: boolean): void { this.canAddNode = canAddNode; }

  makeNode(context: UIContext): FrameNode | null {
    this.node = new FrameNode(context);
    if (this.xComponent === null || this.xComponent === undefined) {
      this.xComponent = typeNode.createNode(context, 'XComponent', {
        type: XComponentType.SURFACE,
        controller: PipManager.getInstance().getXComponentController(),
      });
    }
    if (this.canAddNode) {
      try { this.xComponent.getParent()?.removeChild(this.xComponent); } catch (e) {}
      try { this.node.appendChild(this.xComponent); } catch (e) {}
    }
    return this.node;
  }
}

// 在页面中使用 NodeContainer 添加到布局
NodeContainer(PipManager.getInstance().getNodeController()).size({ width: '100%', height: '800px' })
```

**步骤2：创建画中画控制器（传入contentNode）**

```typescript
private createPipController(ctx: Context, node: typeNode.XComponent | null): void {
  if (!PiPWindow.isPiPEnabled()) { return; }
  const config: PiPWindow.PiPConfiguration = {
    context: ctx,
    componentController: this.getXComponentController(),
    templateType: PiPWindow.PiPTemplateType.VIDEO_PLAY,
    contentWidth: 1920,
    contentHeight: 1080,
  };
  PiPWindow.create(config, node).then((controller: PiPWindow.PiPController) => {
    this.pipController = controller;
    this.pipController.on('stateChange', (state, reason) => { this.onStateChange(state, reason); });
    this.pipController.on('controlEvent', (control) => { this.onActionEvent(control); });
  });
}
```

**步骤3：生命周期管理（添加到布局的typeNode）**

需根据导航方式（单界面Ability / Router / Navigation）在生命周期回调中管理节点：

```typescript
onStateChange(state: PiPWindow.PiPState, reason: string): void {
  this.xcNodeController.setCanAddNode(
    state === PiPWindow.PiPState.ABOUT_TO_STOP || state === PiPWindow.PiPState.STOPPED);
  switch (state) {
    case PiPWindow.PiPState.ABOUT_TO_START:
      this.xcNodeController.removeNode();   // 画中画启动前移除节点
      break;
    case PiPWindow.PiPState.ABOUT_TO_STOP:
      this.addNode();                        // 画中画关闭/还原时重新添加节点
      break;
    case PiPWindow.PiPState.ABOUT_TO_RESTORE:
      this.jumpNext();                       // 还原时跳回原界面（若启动时返回了上级）
      break;
  }
}
```

> **导航方式差异**：
> - **单界面Ability**：ABOUT_TO_START 移除节点，ABOUT_TO_STOP 重新添加。
> - **Router导航**：ABOUT_TO_START 可选返回上级页面，ABOUT_TO_RESTORE 需 push 回原界面。
> - **Navigation导航**：ABOUT_TO_START 可选 `pageInfos.pop()`，ABOUT_TO_RESTORE 需 `pageInfos.pushPath()`。

**步骤4：启动、更新、关闭**

```typescript
startPip(): void {
  this.pipController?.startPiP().then(() => { console.info('Succeeded in starting pip.'); });
}

updateContentSize(width: number, height: number): void {
  this.pipController?.updateContentSize(width, height);
}

stopPip(): void {
  this.pipController?.stopPiP().then(() => { console.info('Succeeded in stopping pip.'); });
}
```

---

### 3.3 使用NDK接口实现（C/C++）

> 适用于依赖 NDK 接口开发的应用。从 API version 20 开始支持。
>
> 设备支持：Phone、PC/2in1、Tablet。

#### 约束与限制

- 画中画窗口画面不通过传入 XComponent Controller 实现，而是通过渲染 `surfaceId`（在启动画中画回调中获取）对应的组件实现。
- 与 typeNode 方式相同，系统不缓存页面，需应用自行管理。
- API 版本 26.0.0 之前不支持设置自动启动；从 26.0.0 开始支持 `OH_PictureInPicture_SetAutoStartEnabled`。

#### 开发流程

1. **创建参数配置器**：`OH_PictureInPicture_CreatePipConfig`，并通过 `SetPipMainWindowId`、`SetPipTemplateType`、`SetPipRect`、`SetPipControlGroup`、`SetPipNapiEnv` 设置配置。
2. **创建控制器**：`OH_PictureInPicture_CreatePip`，缓存 `controllerId`。建议创建后立即 `DestroyPipConfig` 避免内存泄漏。
3. **注册回调**：`OH_PictureInPicture_RegisterStartPipCallback` 获取 `surfaceId` 渲染视频；按需注册生命周期、控制事件等回调。
4. **启动**：`OH_PictureInPicture_StartPip`。
5. **更新尺寸**：`OH_PictureInPicture_UpdatePipContentSize`。
6. **关闭**：`OH_PictureInPicture_StopPip`。
7. **解注册回调**：`OH_PictureInPicture_UnregisterStartPipCallback` 等，避免内存泄漏。

#### Node-API 模块注册示例（napi_init.cpp）

```cpp
#include "napi/native_api.h"
#include "window_manager/oh_window_pip.h"
#include "hilog/log.h"

napi_ref jsCallback;
napi_env env_;
napi_ref jsLifecycleCallback;
napi_env lifeEnv_;

// 启动画中画回调 - 获取surfaceId
void PipStartPipCallback(uint32_t controllerId, uint8_t requestId, uint64_t surfaceId) {
  if (jsCallback) {
    napi_value global = nullptr;
    napi_get_global(env_, &global);
    std::string tStr = std::to_string(surfaceId);
    napi_value str;
    napi_create_string_utf8(env_, tStr.c_str(), strlen(tStr.c_str()), &str);
    napi_value argv[1] = {str};
    napi_value jsCallbackValue;
    napi_get_reference_value(env_, jsCallback, &jsCallbackValue);
    napi_value result = nullptr;
    napi_call_function(env_, global, jsCallbackValue, 1, argv, &result);
  }
}

// 生命周期回调
void LifecycleCallback(uint32_t controllerId, PictureInPicture_PipState state, int32_t errcode) {
  if (jsLifecycleCallback) {
    napi_value global = nullptr;
    napi_get_global(lifeEnv_, &global);
    napi_value pipState = nullptr;
    napi_create_int32(lifeEnv_, static_cast<int32_t>(state), &pipState);
    napi_value argv[1] = {pipState};
    napi_value jsCallbackValue;
    napi_get_reference_value(lifeEnv_, jsLifecycleCallback, &jsCallbackValue);
    napi_value result = nullptr;
    napi_call_function(lifeEnv_, global, jsCallbackValue, 1, argv, &result);
  }
}

class PiPManager {
public:
  static napi_value CreatePip(napi_env env, napi_callback_info info);
  static napi_value StartPip(napi_env env, napi_callback_info info);
  static napi_value RegisterStartPip(napi_env env, napi_callback_info info);
  static napi_value StopPip(napi_env env, napi_callback_info info);
  static napi_value RegisterLifecycleListener(napi_env env, napi_callback_info info);
};

napi_value PiPManager::CreatePip(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1] = {nullptr};
  napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
  napi_value config = argv[0];

  uint32_t controllerId = -1;
  uint32_t mainWindowId = -1;
  PictureInPicture_PipTemplateType pipTemplateType = PictureInPicture_PipTemplateType::VIDEO_PLAY;
  uint32_t width = -1, height = -1;

  napi_value mainWindowIdValue, pipTemplateTypeValue, widthValue, heightValue, controlGroupValue;
  napi_get_named_property(env, config, "mainWindowId", &mainWindowIdValue);
  napi_get_named_property(env, config, "pipTemplateType", &pipTemplateTypeValue);
  napi_get_named_property(env, config, "width", &widthValue);
  napi_get_named_property(env, config, "height", &heightValue);
  napi_get_named_property(env, config, "controlGroup", &controlGroupValue);

  napi_get_value_uint32(env, mainWindowIdValue, &mainWindowId);
  napi_get_value_uint32(env, pipTemplateTypeValue, reinterpret_cast<uint32_t*>(&pipTemplateType));
  napi_get_value_uint32(env, widthValue, &width);
  napi_get_value_uint32(env, heightValue, &height);

  uint32_t size = 0;
  napi_get_array_length(env, controlGroupValue, &size);
  PictureInPicture_PipControlGroup controlGroup[size];
  for (uint32_t i = 0; i < size; i++) {
    napi_value element;
    napi_get_element(env, controlGroupValue, i, &element);
    uint32_t val;
    napi_get_value_uint32(env, element, &val);
    controlGroup[i] = static_cast<PictureInPicture_PipControlGroup>(val);
  }

  PictureInPicture_PipConfig pipConfig;
  OH_PictureInPicture_CreatePipConfig(&pipConfig);
  OH_PictureInPicture_SetPipMainWindowId(pipConfig, mainWindowId);
  OH_PictureInPicture_SetPipTemplateType(pipConfig, pipTemplateType);
  OH_PictureInPicture_SetPipRect(pipConfig, width, height);
  OH_PictureInPicture_SetPipControlGroup(pipConfig, controlGroup, size);
  OH_PictureInPicture_SetPipNapiEnv(pipConfig, env);
  OH_PictureInPicture_CreatePip(pipConfig, &controllerId);
  OH_PictureInPicture_DestroyPipConfig(&pipConfig);

  napi_value result = nullptr;
  napi_create_uint32(env, controllerId, &result);
  return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor desc[] = {
    {"createPip", nullptr, PiPManager::CreatePip, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"startPip", nullptr, PiPManager::StartPip, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"registerStartPip", nullptr, PiPManager::RegisterStartPip, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"stopPip", nullptr, PiPManager::StopPip, nullptr, nullptr, nullptr, napi_default, nullptr},
    {"registerLifecycleListener", nullptr, PiPManager::RegisterLifecycleListener, nullptr, nullptr, nullptr, napi_default, nullptr},
  };
  napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
  return exports;
}
EXTERN_C_END
```

#### Node-API 接口声明（Index.d.ts）

```typescript
export enum PiPControlGroup {
  VIDEO_PLAY_VIDEO_PREVIOUS_NEXT = 101,
  VIDEO_PLAY_FAST_FORWARD_BACKWARD = 102,
  VIDEO_CALL_MICROPHONE_SWITCH = 201,
  VIDEO_CALL_HANG_UP_BUTTON = 202,
  VIDEO_CALL_CAMERA_SWITCH = 203,
  VIDEO_CALL_MUTE_SWITCH = 204,
  VIDEO_MEETING_HANG_UP_BUTTON = 301,
  VIDEO_MEETING_CAMERA_SWITCH = 302,
  VIDEO_MEETING_MUTE_SWITCH = 303,
  VIDEO_MEETING_MICROPHONE_SWITCH = 304,
  VIDEO_LIVE_VIDEO_PLAY_PAUSE = 401,
  VIDEO_LIVE_MUTE_SWITCH = 402,
}
export interface PiPConfig {
  mainWindowId: number;
  pipTemplateType: number;
  width: number;
  height: number;
  controlGroup: Array<PiPControlGroup>;
}
export declare const createPip: (config: PiPConfig) => number;
export declare const startPip: (controllerId: number) => number;
export declare const registerStartPip: (controllerId: number, jsCallback: Function) => number;
export declare const stopPip: (controllerId: number) => number;
export declare const registerLifecycleListener: (controllerId: number, jsCallback: Function) => number;
```

#### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(MyApplication)
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(entry SHARED napi_init.cpp)
target_link_libraries(entry PUBLIC libace_napi.z.so libace_ndk.z.so libnative_window_manager.so libhilog_ndk.z.so)
```

#### ArkTS 侧调用示例

```typescript
import testNapi, { PiPConfig } from 'libentry.so';

// 创建画中画
let config: PiPConfig = {
  mainWindowId: windowId,
  pipTemplateType: PiPWindow.PiPTemplateType.VIDEO_PLAY,
  width: 1920,
  height: 1080,
  controlGroup: [PiPControlGroup.VIDEO_PLAY_VIDEO_PREVIOUS_NEXT]
};
this.controllerId = testNapi.createPip(config);
testNapi.registerStartPip(this.controllerId, this.changeSurface);
testNapi.registerLifecycleListener(this.controllerId, this.onStateChange);

// 启动 / 关闭
testNapi.startPip(this.controllerId);
testNapi.stopPip(this.controllerId);
```

---

## 4. API 参考：@ohos.PiPWindow

该模块提供画中画基础功能，包括判断设备是否支持画中画、创建画中画控制器用于启动或停止画中画等。

### 4.1 模块导入

```typescript
import { PiPWindow } from '@kit.ArkUI';
```

### 4.2 PiPWindow.isPiPEnabled

```typescript
isPiPEnabled(): boolean
```

判断当前设备是否支持画中画功能。返回 `true` 表示支持，`false` 表示不支持。

- 元服务API：从 API version 12 开始支持。
- 系统能力：`SystemCapability.Window.SessionManager`

```typescript
let enable: boolean = PiPWindow.isPiPEnabled();
console.info('isPiPEnabled:' + enable);
```

### 4.3 PiPWindow.create

#### create(config: PiPConfiguration): Promise\<PiPController\>

创建画中画控制器（XComponent 方式），使用 Promise 异步回调。

- 参数 `config` 不能为空，`context` 和 `componentController` 不能为空。
- `controlGroups` 需与 `templateType` 匹配，最多三个控件。
- 错误码：401（参数错误）、801（能力不支持）。

#### create(config: PiPConfiguration, contentNode: typeNode.XComponent): Promise\<PiPController\> ^12+

创建画中画控制器（typeNode 方式），为画中画添加自定义UI节点。

### 4.4 PiPConfiguration 参数说明

| 名称 | 类型 | 可选 | 说明 |
|:-----|:-----|:-----|:-----|
| context | BaseContext | 否 | 上下文环境 |
| componentController | XComponentController | 否 | 原始 XComponent 控制器 |
| navigationId | string | 是 | Navigation 控件 ID，不传则不缓存页面 |
| handleId ^22+ | number | 是 | Navigation 子页面 ID，默认 -1 恢复栈顶页面 |
| templateType | PiPTemplateType | 是 | 模板类型，默认视频播放 |
| contentWidth | number | 是 | 原始内容宽度（px），typeNode方式默认1920 |
| contentHeight | number | 是 | 原始内容高度（px），typeNode方式默认1080 |
| controlGroups ^12+ | Array\<PiPControlGroup\> | 是 | 控制面板可选控件组列表，最多三个 |
| customUIController ^12+ | NodeController | 是 | 自定义UI控制器 |
| localStorage ^17+ | LocalStorage | 是 | 页面级UI状态存储，多实例下跟踪主窗实例 |
| defaultWindowSizeType ^19+ | number | 是 | 第一次拉起窗口大小：0=不设置，1=小窗，2=大窗，默认0 |
| cornerAdsorptionEnabled ^22+ | boolean | 是 | 是否开启四角吸附，默认true（Phone/Tablet生效） |

### 4.5 枚举类型

#### PiPTemplateType（模板类型）

| 名称 | 值 | 说明 |
|:-----|:--|:-----|
| VIDEO_PLAY | 0 | 视频播放（默认存在播放/暂停控件） |
| VIDEO_CALL | 1 | 视频通话 |
| VIDEO_MEETING | 2 | 视频会议 |
| VIDEO_LIVE | 3 | 直播 |

#### PiPState（生命周期状态）

| 名称 | 值 | 说明 |
|:-----|:--|:-----|
| ABOUT_TO_START | 1 | 将要启动 |
| STARTED | 2 | 已经启动 |
| ABOUT_TO_STOP | 3 | 将要停止 |
| STOPPED | 4 | 已经停止 |
| ABOUT_TO_RESTORE | 5 | 将从小窗恢复到原始界面 |
| ERROR | 6 | 出现异常 |

> **reason 值（HarmonyOS 6.1.0+）**：`requestStart`、`autoStart`、`requestDelete`、`panelActionDelete`、`dragDelete`、`panelActionRestore`、`other`

#### PiPControlStatus ^12+（控件状态）

| 名称 | 值 | 说明 |
|:-----|:--|:-----|
| PLAY / OPEN | 1 | 播放/打开状态 |
| PAUSE / CLOSE | 0 | 暂停/关闭状态 |

#### PiPControlType ^12+（控件类型）

| 名称 | 值 | 说明 |
|:-----|:--|:-----|
| VIDEO_PLAY_PAUSE | 0 | 播放/暂停 |
| VIDEO_PREVIOUS | 1 | 上一个 |
| VIDEO_NEXT | 2 | 下一个 |
| FAST_FORWARD | 3 | 快进 |
| FAST_BACKWARD | 4 | 快退 |
| HANG_UP_BUTTON | 5 | 挂断 |
| MICROPHONE_SWITCH | 6 | 麦克风开关 |
| CAMERA_SWITCH | 7 | 摄像头开关 |
| MUTE_SWITCH | 8 | 静音开关 |

#### PiPActionEventType（动作事件类型）

- **视频播放**：`'playbackStateChanged'`、`'nextVideo'`、`'previousVideo'`、`'fastForward'`、`'fastBackward'`
- **视频通话**：`'hangUp'`、`'micStateChanged'`、`'videoStateChanged'`、`'voiceStateChanged'`
- **视频会议**：`'hangUp'`、`'voiceStateChanged'`、`'videoStateChanged'`、`'micStateChanged'`
- **直播**：`'playbackStateChanged'`、`'voiceStateChanged'`

#### PiPWindowSize ^15+

| 名称 | 类型 | 说明 |
|:-----|:-----|:-----|
| width | number | 窗口宽度（px） |
| height | number | 窗口高度（px） |
| scale | number | 窗口缩放比，取值 (0.0, 1.0] |

#### PiPWindowInfo ^15+

| 名称 | 类型 | 说明 |
|:-----|:-----|:-----|
| windowId | number | 画中画窗口 ID |
| size | PiPWindowSize | 画中画窗口大小 |

### 4.6 PiPController 接口

> 下列方法都需先通过 `PiPWindow.create()` 获取 PiPController 实例。

| 方法 | 说明 | 错误码 |
|:-----|:-----|:-------|
| `startPiP(): Promise<void>` | 启动画中画 | 1300012/1300013/1300014/1300015/1300034 |
| `stopPiP(): Promise<void>` | 停止画中画 | 1300011/1300012/1300015 |
| `setAutoStartEnabled(enable: boolean): void` | 设置退后台自动启动，默认false | - |
| `updateContentSize(width, height): void` | 更新媒体源尺寸 | 401 |
| `updatePiPControlStatus(controlType, status): void` ^12+ | 更新控件功能状态 | 401 |
| `setPiPControlEnabled(controlType, enabled): void` ^12+ | 设置控件使能状态 | 401 |
| `updateContentNode(contentNode): Promise<void>` ^18+ | 更新画中画节点内容 | 401/801/1300014 |
| `getPiPWindowInfo(): Promise<PiPWindowInfo>` ^15+ | 获取窗口信息 | 801/1300014 |
| `getPiPSettingSwitch(): Promise<boolean>` ^20+ | 获取设置中自动启动开关状态（Phone/Tablet） | 801/1300014 |
| `isPiPActive(): Promise<boolean>` ^23+ | 获取画中画可见状态（非STARTED时返回false） | 1300014 |

#### 错误码说明

| 错误码 | 含义 |
|:-------|:-----|
| 401 | 参数错误 |
| 801 | 能力不支持 |
| 1300011 | 销毁画中画窗口失败 |
| 1300012 | 画中画窗口状态异常 |
| 1300013 | 创建画中画窗口失败 |
| 1300014 | 画中画内部错误 |
| 1300015 | 重复画中画操作 |
| 1300034 | 与其他悬浮窗冲突（26.0.0+） |

### 4.7 事件监听接口

| 方法 | 说明 | 起始版本 |
|:-----|:-----|:---------|
| `on('stateChange', callback)` | 开启生命周期状态变化监听 | 11 |
| `off('stateChange')` | 关闭生命周期状态变化监听 | 11 |
| `on('controlPanelActionEvent', callback)` | 开启控制面板动作事件监听（推荐用controlEvent） | 11 |
| `off('controlPanelActionEvent')` | 关闭控制面板动作事件监听 | 11 |
| `on('controlEvent', callback)` ^12+ | 开启控制面板动作事件监听（推荐） | 12 |
| `off('controlEvent', callback?)` ^12+ | 关闭控制面板动作事件监听 | 12 |
| `on('pipWindowSizeChange', callback)` ^15+ | 开启窗口尺寸变化监听 | 15 |
| `off('pipWindowSizeChange', callback?)` ^15+ | 关闭窗口尺寸变化监听 | 15 |
| `on('activeStatusChange', callback)` ^22+ | 开启窗口可见状态变化监听 | 22 |
| `off('activeStatusChange', callback?)` ^22+ | 关闭窗口可见状态变化监听 | 22 |

#### on('controlEvent') 示例

```typescript
this.pipController.on('controlEvent', (control: PiPWindow.ControlEventParam) => {
  switch (control.controlType) {
    case PiPWindow.PiPControlType.VIDEO_PLAY_PAUSE:
      if (control.status === PiPWindow.PiPControlStatus.PAUSE) { /* 停止视频 */ }
      else if (control.status === PiPWindow.PiPControlStatus.PLAY) { /* 播放视频 */ }
      break;
    case PiPWindow.PiPControlType.VIDEO_NEXT: /* 下一个视频 */ break;
    case PiPWindow.PiPControlType.VIDEO_PREVIOUS: /* 上一个视频 */ break;
    case PiPWindow.PiPControlType.FAST_FORWARD: /* 快进 */ break;
    case PiPWindow.PiPControlType.FAST_BACKWARD: /* 快退 */ break;
  }
});
```

#### ControlEventParam ^12+

| 名称 | 类型 | 说明 |
|:-----|:-----|:-----|
| controlType | PiPControlType | 控件类型 |
| status | PiPControlStatus | 控件状态（挂断等无状态控件返回-1） |

### 4.8 NDK 接口一览

| 接口名 | 描述 |
|:-------|:-----|
| `OH_PictureInPicture_CreatePipConfig(pipConfig)` | 创建参数配置器 |
| `OH_PictureInPicture_DestroyPipConfig(pipConfig)` | 销毁参数配置器 |
| `OH_PictureInPicture_CreatePip(pipConfig, controllerId)` | 创建控制器 |
| `OH_PictureInPicture_DeletePip(controllerId)` | 删除控制器 |
| `OH_PictureInPicture_StartPip(controllerId)` | 启动画中画 |
| `OH_PictureInPicture_StopPip(controllerId)` | 停止画中画 |
| `OH_PictureInPicture_UpdatePipContentSize(controllerId, width, height)` | 更新媒体源尺寸 |
| `OH_PictureInPicture_UpdatePipControlStatus(controllerId, controlType, status)` | 更新控件状态 |
| `OH_PictureInPicture_SetPipControlEnabled(controllerId, controlType, enabled)` | 设置控件使能 |
| `OH_PictureInPicture_RegisterStartPipCallback(controllerId, callback)` | 注册启动回调（获取surfaceId） |
| `OH_PictureInPicture_UnregisterStartPipCallback(controllerId, callback)` | 解注册启动回调 |
| `OH_PictureInPicture_RegisterLifecycleListener(controllerId, callback)` | 注册生命周期监听 |
| `OH_PictureInPicture_UnregisterLifecycleListener(controllerId, callback)` | 解注册生命周期监听 |
| `OH_PictureInPicture_RegisterControlEventListener(controllerId, callback)` | 注册控制事件监听 |
| `OH_PictureInPicture_UnregisterControlEventListener(controllerId, callback)` | 解注册控制事件监听 |
| `OH_PictureInPicture_RegisterResizeListener(controllerId, callback)` | 注册窗口尺寸变化监听 |
| `OH_PictureInPicture_UnregisterResizeListener(controllerId, callback)` | 解注册窗口尺寸变化监听 |

> NDK 详细参考：[oh_window_pip.h](https://developer.huawei.com/consumer/cn/doc/harmonyos-references/capi-oh-window-pip-h)

---

## 5. 设计规范

### 5.1 触发方式

画中画有多种触发方式，业务可根据场景选择：

| 触发方式 | 说明 | 前置条件 |
|:---------|:-----|:---------|
| **固定入口** | 通过应用内按钮触发 | 无 |
| **应用返回** | 点击"返回"按钮或手势返回上一级时开启 | 需在系统设置开启"自动开启画中画" |
| **返回桌面** | 在应用内返回桌面时开启 | 需在系统设置开启"自动开启画中画" |

### 5.2 窗口大小调节

1. **双击窗口**放大。
2. **拖拽窗口左下角或右下角**，等比例缩放。
3. **手指捏合或张开**，在最大和最小尺寸之间等比例缩放。

### 5.3 窗口最小化

拖动画中画窗口到屏幕边缘，画中画收起到侧边悬浮条，任务不中断。点击侧边条可恢复画中画窗口。

### 5.4 控制面板结构

系统定义画中画窗口显示区域、按钮数量和功能，针对不同业务场景提供对应控制模板。

**控制面板交互**：
- 默认小窗启动，显示控制面板，3秒不操作自动隐藏。
- 控制面板隐藏后，单击窗口显示控制面板。
- 2in1 端鼠标悬停到窗口上显示控制面板。

**各场景控制按钮配置**：

| 业务场景 | 窗口控制区 | 内容控制（必选） | 内容控制（可选） | 备注 |
|:---------|:-----------|:-----------------|:-----------------|:-----|
| 视频播放 | 关闭、还原 | 播放/暂停 | 上一个&下一个、快进&快退 | 可选控件只能成对出现；上一个&下一个与快进&快退不可同时出现 |
| 直播 | 关闭、还原 | 无 | 外放静音、播放/暂停 | - |
| 视频通话 | 关闭、还原（配置任意可选按钮后显示） | 无 | 麦克风、挂断、摄像头、外放静音 | 默认无按钮，点击窗口任意位置还原；选择任意按钮后还原和关闭自动显示 |
| 视频会议 | 关闭、还原（配置任意可选按钮后显示） | 无 | 麦克风、挂断、摄像头、外放静音 | 同视频通话 |

### 5.5 视觉规格

#### 窗口默认大小与位置

- 默认显示最小窗口尺寸。
- 手机端、折叠屏和平板端默认显示在屏幕**右上角**。
- 2in1 端默认显示在屏幕**左上角**。

#### 画中画窗口尺寸

系统定义竖向和横向两种比例窗口，各有最小（默认）和最大两个尺寸：

| 设备状态 | 竖向-默认 | 竖向-最大 | 横向-默认 | 横向-最大 |
|:---------|:----------|:----------|:----------|:----------|
| 直板机/折叠屏折叠态 | 窗口短边为屏幕短边的30% | 窗口宽度占3栅格 | 窗口短边为屏幕短边的30% | 窗口宽度占4栅格 |
| 折叠屏展开态 | 窗口短边为屏幕折叠态短边的30% | 窗口宽度占3栅格 | 窗口短边为屏幕折叠态短边的30% | 窗口宽度占5栅格 |
| 平板 | 窗口尺寸为默认悬浮窗的30% | 窗口宽度占4栅格 | 窗口宽度为屏幕短边的30% | 窗口宽度占5栅格 |

---

## 6. 常见问题

### Q1: 画中画如何实现多个视频流播放？

通过自定义节点将视频流封装为 `NodeController`，创建画中画时配置为 `customUIController`，实现多个视频流播放。

### Q2: 画中画如何实现节点切换？

使用 typeNode 方案，通过 `updateContentNode(contentNode)` 方法（API version 18+）更新画中画节点。

### Q3: 拖动删除或点击关闭按钮关闭后，后台声音仍然存在怎么办？

对于申请了长时任务的应用，画中画拖动删除或点击关闭后仅删除画中画窗口，并不会结束应用进程。开发者需要开启画中画生命周期监听，在 `STOPPED` 生命周期中主动关闭任务或进程。

---

## 附录：接口版本支持汇总

| 接口/特性 | 起始版本 | 说明 |
|:----------|:---------|:-----|
| PiPWindow 模块 | API 11 | 首批接口 |
| isPiPEnabled / create / startPiP / stopPiP | API 11 | 基础接口 |
| typeNode 方式 / controlGroups / customUIController / on('controlEvent') | API 12 | 元服务支持 |
| PiPWindowSize / PiPWindowInfo / getPiPWindowInfo / on('pipWindowSizeChange') | API 15 | 窗口信息 |
| NDK 接口 | API 20 | C/C++ 开发 |
| getPiPSettingSwitch | API 20 | 获取设置开关状态 |
| handleId / cornerAdsorptionEnabled / on('activeStatusChange') | API 22 | 子页面ID/四角吸附/可见状态 |
| isPiPActive | API 23 | 获取可见状态 |
| OH_PictureInPicture_SetAutoStartEnabled | API 26 | NDK自动启动 |
| 1300034 错误码 | API 26 | 悬浮窗冲突 |
| reason 切换原因 | HarmonyOS 6.1.0 | stateChange 回调 reason 值 |
| XComponent 方式 PC/2in1 支持 | HarmonyOS 6.0.0 | 设备扩展 |