# 闪控球（FloatingBall）知识库

> 本知识库基于华为 HarmonyOS 官方文档整理，涵盖闪控球的概念、设计规范、API 参考和开发指南。

## 文档来源

| 文档名称 | 链接 |
|:---------|:----|
| 闪控球开发指导 | https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/floatingball-guide |
| 闪控球和闪控窗设计指导 | https://developer.huawei.com/consumer/cn/doc/design-guides/floatview-0000002624640357 |
| @ohos.window.floatingBall API 参考 | https://developer.huawei.com/consumer/cn/doc/harmonyos-references/js-apis-floatingball |

## 版本信息

- **起始 API 版本**：API version 20
- **DevEco Studio 最低版本**：6.0.1 Release（模拟器支持）
- **模块名**：`@ohos.window.floatingBall`
- **导入方式**：`import { floatingBall } from '@kit.ArkUI';`
- **系统能力**：`SystemCapability.Window.SessionManager`
- **所需权限**：`ohos.permission.USE_FLOAT_BALL`（受限权限，需申请 ACL）

## 目录

- [一、闪控球概述](#一闪控球概述)
- [二、闪控球设计规范](#二闪控球设计规范)
- [三、闪控球 API 参考](#三闪控球-api-参考)
- [四、闪控球开发指南](#四闪控球开发指南)

---

# 一、闪控球概述

## 什么是闪控球

闪控球是一种在设备屏幕上悬浮的非全屏应用窗口，为应用提供临时的全局能力，完成跨应用交互。应用可以将关键信息以小窗（闪控球）模式呈现。切换为小窗（闪控球）模式后，用户可以进行其他界面操作，提升使用体验。

闪控球以悬浮小组件形式显示在其他应用之上，即时呈现应用的关键信息，适用于跨应用的题目搜索、账单记录、商品比价、抢单、翻译场景，以及金融类应用的实时盯盘场景。

## 适用场景

闪控球和闪控窗是悬浮于应用界面上层的轻量化窗口，支持信息展示和轻量任务处理，可实现灵活、安全、便捷地跨应用操作，适配各类高频交互场景。

| 类别 | 定位 | 体验价值 | 适用场景 |
|:-----|:-----|:---------|:---------|
| 闪控球 | 展示少量关键信息，支持应用外一键操作，自动拉起应用 | 实现跨应用快捷操作 | 抢单、记账、比价等 |
| 闪控窗 | 展示更多关键信息，支持应用外便捷操作、实时监测 | 实现跨应用高频、轻量交互操作 | 金融盯盘、游戏直播等 |

### 开放权限的场景

当前仅对以下场景开放 `ohos.permission.USE_FLOAT_BALL` 权限：

- 跨应用的题目搜索
- 账单记录
- 商品比价
- 抢单
- 翻译场景
- 金融类应用的实时盯盘场景

> **注意**：接入后需在以上场景范围内使用，否则将会进行相关处罚与限制。

## 约束与限制

1. **前台启动**：基于安全考虑，仅允许应用在前台时启动闪控球。
2. **权限要求**：需要具有 `ohos.permission.USE_FLOAT_BALL` 权限（受限权限，需通过 ACL 申请）。
3. **数量限制**：
   - 同一个应用只能启动一个闪控球。
   - 同一个设备最多同时存在两个闪控球。
   - 超出最大个数限制时，打开新的闪控球会替换最早启动的闪控球。
4. **设备支持**：
   - 支持在 Phone、Tablet、PC/2in1 设备上使用。
   - HarmonyOS 7.0.0 之前：支持 Tablet 设备的非电脑模式、Phone 设备使用。
   - HarmonyOS 7.0.0 起：支持 Phone、PC/2in1、Tablet 设备使用。
5. **API 版本**：从 API version 20 开始支持。
6. **模拟器支持**：DevEco Studio 6.0.1 Release 及以上版本。

## 闪控球与闪控窗对比

### 共同点

闪控球和闪控窗均为一种特殊的应用辅助窗口，具备在应用主窗口和对应 UIAbility 退至后台后仍然可以在前台显示的能力。可以用于应用退至后台后，使用其继续显示 UI。

### 区别

| 对比维度 | 闪控球 | 闪控窗 |
|:---------|:-------|:-------|
| 显示形式 | 以小圆球的形式展现，适用于展示关键信息 | 以小型窗口展示，展示区域较大，可持续展示应用内容或提供快捷操作 |
| 位置限制 | 只能贴边展示 | 没有贴边限制 |
| UI 定制 | 模板固定，应用不能定制 UI | 存在模板并由系统管理统一绘制 UI，但提供可绘制区域供应用加载指定页面内容 |
| 数量限制 | 系统最多 2 个，单应用最多 1 个 | 系统最多 1 个 |
| 适用场景 | 抢单、记账、比价等 | 金融盯盘、游戏直播等 |

### 联动使用

本模块可与 `@ohos.window.floatView`（闪控窗）联合使用。通过 `floatView.bind` 接口将闪控窗控制器与闪控球控制器绑定后：

- 用户点击闪控球可展开为闪控窗
- 点击闪控窗左上角的缩小按钮可收起为闪控球
- 实现两种窗口形态的相互切换

### 闪控窗与系统悬浮窗区别

- **闪控窗**：承载应用内单一功能或局部信息，无法覆盖全应用业务；支持应用内悬浮或跨应用悬浮；窗口仅限执行本任务相关功能。
- **系统悬浮窗**：承载应用全量功能，在同一个悬浮窗内可进行应用内不同功能的切换和跳转。

---

# 二、闪控球设计规范

## 闪控球基础交互

### 闪控球创建

应用接入闪控球功能后，用户可通过应用内指定入口手动开启闪控球。点击闪控球，跳转至原应用对应界面。

### 闪控球数量限制

系统最多支持 2 个闪控球，一个应用内仅支持创建一个闪控球。

### 闪控球最小化

闪控球支持拖拽收入侧边条。

### 闪控球删除

用户可以将单个或多个闪控球整体拖拽至底部垃圾桶删除，也可以长按后通过菜单删除闪控球。

## 交互方式详解

闪控球提供以下交互方式：

| 交互方式 | 说明 |
|:---------|:-----|
| 单击闪控球 | 触发闪控球点击事件 |
| 长按闪控球 | 长按闪控球震动变为待删除态，可以点击图标单个删除或全部删除 |
| 拖动闪控球 | 可以手动拖拽闪控球改变位置，拖拽时自动避让状态栏、固定态软键盘、导航条等其他组件。设备处于横屏场景时不会自动避让输入法。拖拽松手时闪控球自动吸附在最近的侧边，拖拽到垃圾桶区域（底部中部区域）松手即可删除 |
| 位置记忆 | 关闭闪控球会记录当前位置，下一次打开功能时自动展示在上次关闭时的位置。旋转屏幕或重启设备会恢复到默认位置，默认位置位于屏幕右上侧 |

## 闪控球模板结构

支持四种闪控球模板布局，应用可根据业务诉求选择接入模板。对应枚举 `FloatingBallTemplate`。

| 模板 | 枚举值 | 支持内容 | 必传参数 |
|:-----|:-------|:---------|:---------|
| 静态布局 | `STATIC` (1) | 图标和标题 | title、icon |
| 普通文本布局 | `NORMAL` (2) | 标题和内容 | title |
| 强调文本布局 | `EMPHASIC` (3) | 图标、标题和内容 | title |
| 纯文本布局 | `SIMPLE` (4) | 仅标题（可双行展示） | title |

### 各模板样式说明

- **静态布局（STATIC）**：支持图标和标题。该布局在创建后无法修改（无法更新）。
- **普通文本布局（NORMAL）**：支持标题和内容。
- **强调文本布局（EMPHASIC）**：支持图标、标题和内容。
- **纯文本布局（SIMPLE）**：仅支持标题，可双行展示。

### 模板布局示意

不同闪控球模板与样式布局示意如下，不同语言或内容以实际显示效果为准：

- 图1：静态布局
- 图2：静态布局-超长文本标题
- 图3：普通文本布局
- 图4：普通文本布局-超长文本内容
- 图5：强调文本布局
- 图6：强调文本布局-超长文本内容
- 图7：强调文本布局-图标
- 图8：强调文本布局-图标和超长文本内容
- 图9：纯文本布局
- 图10：纯文本布局-超长文本标题

### 合并展示

当有两个应用启动了闪控球后，闪控球将合并展示，整体高度为 76vp。

## 闪控球规格

目前闪控球的规格为：

- 整体尺寸宽为 70vp ~ 98vp 之间
- 高为 40vp
- 标题和内容不支持自定义字体大小

## 不同设备尺寸

闪控球宽度会根据传入内容进行宽度档位切换，高度不变。

### 闪控球设备尺寸表

| 设备类型 | 默认尺寸 (vp) | 档位1 | 档位2 | 档位3 | 档位4 |
|:---------|:-------------|:------|:------|:------|:------|
| 直板机 (竖屏) | 70*40 (包含底板) | 70*40 | 80*40 | 90*40 | 98*40 |
| 直板机 (横屏) | 70*40 (包含底板) | 70*40 | 80*40 | 90*40 | 98*40 |
| 双折叠展开态 | 70*40 (包含底板) | 70*40 | 80*40 | 90*40 | 98*40 |
| 三折叠展开态 | 70*40 (包含底板) | 70*40 | 80*40 | 90*40 | 98*40 |
| 平板/电脑 | 104*60 (包含底板) | 104*60 | 114*60 | 124*60 | 132*60 |

> 上方数据为包含外部材质底板宽度，内部底板距离左右边距泛手机为 4vp，平板/电脑为 6vp。

## 附录：闪控窗设计规范

> 以下为闪控窗的设计规范，作为闪控球联动使用的补充参考。

### 闪控窗基础交互

- **创建**：应用接入闪控窗功能后，用户可通过应用内指定入口手动开启闪控窗。点击闪控窗，跳转至原应用对应界面。系统内最多可显示一个闪控窗。
- **与闪控球切换**：若应用同时接入闪控窗和闪控球，闪控窗和闪控球之间可以互相切换。
- **最小化**：闪控窗支持拖拽收入侧边条，也可以通过"最小化"按钮收入侧边条。
- **删除**：用户可将闪控窗拖拽至底部垃圾桶删除，也可以直接通过关闭按钮删除闪控窗。

### 闪控窗窗口显示状态

| 显示状态 | 窗口样式 | 应用场景 |
|:---------|:---------|:---------|
| 常规态 | 矩形，在最大最小尺寸范围内可自由设定宽高 | 适用于大多数使用场景 |
| Mini态 | 横向细长条矩形 | 窗口对于页面遮挡较小，适用于游戏直播、盯盘等场景 |

### 闪控窗不同设备尺寸

闪控窗尺寸有最大、最小尺寸限制，可按需在区间内自由定义窗口尺寸大小与比例。

| 设备类型 | 最小宽度 | 最小高度 | 最大宽度 | 最大高度 |
|:---------|:---------|:---------|:---------|:---------|
| 直板机 (竖屏) | 屏幕宽度 30% | 窗口宽度*75% | 屏幕宽度*90% | 窗口宽度*75% |
| 直板机 (横屏) | 直板机(竖屏)屏幕宽度 30% | 窗口宽度*75% | 屏幕宽度*45% | 窗口宽度*75% |
| 双折叠展开态 | 折叠态(竖屏)屏幕宽度 30% | 窗口宽度*75% | 屏幕宽度*55% | 窗口宽度*75% |
| 三折叠展开态 | 折叠态(竖屏)屏幕宽度 30% | 窗口宽度*75% | 屏幕宽度 50% | 窗口宽度*75% |
| 平板/电脑 | 屏幕宽度*10% | 窗口宽度*75% | 屏幕宽度*50% | 窗口宽度*75% |

### Mini态尺寸

Mini态窗口高度限定为 32vp，宽度同常规态规格。

---

# 三、闪控球 API 参考

> 模块：`@ohos.window.floatingBall`
> 导入：`import { floatingBall } from '@kit.ArkUI';`
> 系统能力：`SystemCapability.Window.SessionManager`
> 起始版本：API version 20

## 接口一览表

| 接口名 | 描述 | 所需权限 |
|:-------|:-----|:---------|
| `isFloatingBallEnabled(): boolean` | 判断当前设备是否支持闪控球功能 | 无 |
| `create(config: FloatingBallConfiguration): Promise<FloatingBallController>` | 创建闪控球控制器 | 无 |
| `startFloatingBall(params: FloatingBallParams): Promise<void>` | 启动闪控球 | `ohos.permission.USE_FLOAT_BALL` |
| `updateFloatingBall(params: FloatingBallParams): Promise<void>` | 更新闪控球 | 无 |
| `stopFloatingBall(): Promise<void>` | 停止闪控球 | 无 |
| `on(type: 'stateChange', callback: Callback<FloatingBallState>): void` | 开启闪控球生命周期状态监听 | 无 |
| `off(type: 'stateChange', callback?: Callback<FloatingBallState>): void` | 关闭闪控球生命周期状态监听 | 无 |
| `on(type: 'click', callback: Callback<void>): void` | 开启闪控球点击事件监听 | 无 |
| `off(type: 'click', callback?: Callback<void>): void` | 关闭闪控球点击事件监听 | 无 |
| `getFloatingBallWindowInfo(): Promise<FloatingBallWindowInfo>` | 获取闪控球窗口信息 | 无 |
| `restoreMainWindow(want: Want): Promise<void>` | 恢复应用主窗口，加载指定页面 | `ohos.permission.USE_FLOAT_BALL` |
| `setFloatingBallVisibilityInApp(isVisible: boolean): Promise<void>` | 设置闪控球在应用内是否可见（API 24+） | 无 |
| `onDestroy(callback: Callback<string>): void` | 注册闪控球销毁事件监听（API 26+） | 无 |
| `offDestroy(callback?: Callback<string>): void` | 取消闪控球销毁事件监听（API 26+） | 无 |

## 模块导入

```typescript
import { floatingBall } from '@kit.ArkUI';
```

## floatingBall.isFloatingBallEnabled

```typescript
isFloatingBallEnabled(): boolean
```

判断当前设备是否支持闪控球功能。

**返回值**：

| 类型 | 说明 |
|:-----|:-----|
| boolean | true 表示支持，false 表示不支持 |

**示例**：

```typescript
let enable: boolean = floatingBall.isFloatingBallEnabled();
console.info('Floating ball enabled is: ' + enable);
```

## floatingBall.create

```typescript
create(config: FloatingBallConfiguration): Promise<FloatingBallController>
```

创建闪控球控制器，使用 Promise 异步回调。

**参数**：

| 参数名 | 类型 | 必填 | 说明 |
|:-------|:-----|:-----|:-----|
| config | FloatingBallConfiguration | 是 | 创建闪控球控制器的参数。该参数不能为空，并且构造该参数的 context 不能为空 |

**返回值**：

| 类型 | 说明 |
|:-----|:-----|
| Promise<FloatingBallController> | 返回当前创建的闪控球控制器 |

**错误码**：

| 错误码 | 说明 |
|:-------|:-----|
| 801 | Capability not supported（设备能力不支持） |
| 1300019 | Wrong parameters for operating the floating ball |
| 1300023 | Floating ball internal error |

**设备行为差异**：
- HarmonyOS 7.0.0 之前：Tablet 设备非电脑模式、Phone 设备可正常调用，其他返回 801。
- HarmonyOS 7.0.0 起：Phone、PC/2in1、Tablet 设备可正常调用，其他返回 801。

**示例**：

```typescript
import { BusinessError } from '@kit.BasicServicesKit';
import { common } from '@kit.AbilityKit';

let floatingBallController: floatingBall.FloatingBallController | undefined = undefined;
let ctx = this.getUIContext().getHostContext() as common.UIAbilityContext;
let config: floatingBall.FloatingBallConfiguration = {
  context: ctx,
};
try {
  floatingBall.create(config).then((data: floatingBall.FloatingBallController) => {
    floatingBallController = data;
    console.info(`Succeeded in creating floating ball controller. Data: ${data}`);
  }).catch((err: BusinessError) => {
    console.error(`Failed to create. Cause:${err.code}, message:${err.message}`);
  });
} catch (e) {
  console.error(`Failed to create. Cause:${e.code}, message:${e.message}`);
}
```

## FloatingBallController.startFloatingBall

```typescript
startFloatingBall(params: FloatingBallParams): Promise<void>
```

启动闪控球，使用 Promise 异步回调。

**需要权限**：`ohos.permission.USE_FLOAT_BALL`

**参数**：

| 参数名 | 类型 | 必填 | 说明 |
|:-------|:-----|:-----|:-----|
| params | FloatingBallParams | 是 | 启动闪控球的参数，用于配置标题、内容或背景色等 |

**错误码**：

| 错误码 | 说明 |
|:-------|:-----|
| 201 | Permission verification failed |
| 1300019 | Wrong parameters for operating the floating ball |
| 1300020 | Failed to create the floating ball window |
| 1300021 | Failed to start multiple floating ball windows |
| 1300022 | Repeated floating ball operation |
| 1300023 | Floating ball internal error |
| 1300024 | The floating ball window state is abnormal |
| 1300025 | The floating ball state does not support this operation |
| 1300034 | This operation conflicts with other floating windows（App 已启动 float view，适用版本 26.0.0+） |

**示例**：

```typescript
let startParams: floatingBall.FloatingBallParams = {
  template: floatingBall.FloatingBallTemplate.EMPHATIC,
  title: 'title',
  content: 'content'
};
try {
  floatingBallController.startFloatingBall(startParams).then(() => {
    console.info('Succeeded in starting floating ball.');
  }).catch((err: BusinessError) => {
    console.error(`Failed. Cause:${err.code}, message:${err.message}`);
  });
} catch (e) {
  console.error(`Failed. Cause:${e.code}, message:${e.message}`);
}
```

## FloatingBallController.updateFloatingBall

```typescript
updateFloatingBall(params: FloatingBallParams): Promise<void>
```

更新闪控球，使用 Promise 异步回调。必须先调用 `startFloatingBall()` 启动闪控球后，才能调用此方法。

> **注意**：调用此接口更新闪控球时，模板类型 `template` 字段不可更改。

**错误码**：

| 错误码 | 说明 |
|:-------|:-----|
| 1300002 | This window state is abnormal |
| 1300003 | This window manager service works abnormally |
| 1300004 | Unauthorized operation |
| 1300019 | Wrong parameters for operating the floating ball |
| 1300023 | Floating ball internal error |
| 1300024 | The floating ball window state is abnormal |
| 1300025 | The floating ball state does not support this operation |
| 1300027 | When updating the floating ball, the template type cannot be changed |
| 1300028 | Updating static template-based floating balls is not supported |

## FloatingBallController.stopFloatingBall

```typescript
stopFloatingBall(): Promise<void>
```

停止闪控球，使用 Promise 异步回调。

> **注意**：stop 是异步流程，需要通过 `stateChange` 状态回调获取实际删除结果。

**错误码**：

| 错误码 | 说明 |
|:-------|:-----|
| 1300022 | Repeated floating ball operation |
| 1300023 | Floating ball internal error |
| 1300024 | The floating ball window state is abnormal |

## FloatingBallController.on('stateChange')

```typescript
on(type: 'stateChange', callback: Callback<FloatingBallState>): void
```

注册闪控球生命周期状态变化的监听事件。不再使用时，取消监听以避免内存泄漏。

## FloatingBallController.off('stateChange')

```typescript
off(type: 'stateChange', callback?: Callback<FloatingBallState>): void
```

取消闪控球生命周期状态变化的监听事件。若传入参数，则停止该监听；若未传入参数，则停止所有监听。

## FloatingBallController.on('click')

```typescript
on(type: 'click', callback: Callback<void>): void
```

注册闪控球的点击监听事件，不使用时取消监听以避免内存泄漏。

## FloatingBallController.off('click')

```typescript
off(type: 'click', callback?: Callback<void>): void
```

取消闪控球点击的监听事件。

## FloatingBallController.getFloatingBallWindowInfo

```typescript
getFloatingBallWindowInfo(): Promise<FloatingBallWindowInfo>
```

获取闪控球窗口信息，使用 Promise 异步回调。

**返回值**：

| 类型 | 说明 |
|:-----|:-----|
| Promise<FloatingBallWindowInfo> | 返回闪控球窗口信息 |

## FloatingBallController.restoreMainWindow

```typescript
restoreMainWindow(want: Want): Promise<void>
```

恢复应用主窗口并加载指定页面。使用 Promise 异步回调。

> **注意**：仅支持在点击闪控球后调用；若应用拥有 `ohos.permission.AUTO_RESTORE_MAIN_WINDOW` 权限，可以无需点击直接调用该接口。

**需要权限**：`ohos.permission.USE_FLOAT_BALL`

**参数**：

| 参数名 | 类型 | 必填 | 说明 |
|:-------|:-----|:-----|:-----|
| want | Want | 是 | 加载指定页面的 Want |

**错误码**：

| 错误码 | 说明 |
|:-------|:-----|
| 201 | Permission verification failed |
| 1300002 | This window state is abnormal |
| 1300003 | This window manager service works abnormally |
| 1300004 | Unauthorized operation |
| 1300019 | Wrong parameters for operating the floating ball |
| 1300023 | Floating ball internal error |
| 1300024 | The floating ball window state is abnormal |
| 1300025 | The floating ball state does not support this operation |
| 1300026 | Failed to restore the main window（bundleName 不匹配或缺少 AUTO_RESTORE_MAIN_WINDOW 权限且无用户点击） |

## FloatingBallController.setFloatingBallVisibilityInApp (API 24+)

```typescript
setFloatingBallVisibilityInApp(isVisible: boolean): Promise<void>
```

设置闪控球在应用内是否可见。

**行为说明**：
- 当应用处于多任务界面时（生命周期状态为 PAUSED），闪控球不可见。
- 默认情况（未调用此接口）和传入 `true` 时：除多任务界面外，闪控球均可见。
- 传入 `false` 时：当应用处于前台（SHOWN/RESUMED）时闪控球不可见；当应用处于后台（HIDDEN）时闪控球可见。

**模型约束**：仅可在 Stage 模型下使用。

## FloatingBallController.onDestroy (API 26+)

```typescript
onDestroy(callback: Callback<string>): void
```

注册闪控球销毁事件的监听。当闪控球销毁时，回调函数会接收到销毁原因的字符串。

**销毁原因**：

| 原因字符串 | 说明 |
|:-----------|:-----|
| `APP_STOP` | 应用主动停止 |
| `DUMPSTER_STOP` | 拖动到垃圾桶触发停止 |
| `LONG_PRESS_SINGLE_STOP` | 长按单个闪控球触发停止 |
| `LONG_PRESS_ALL_STOP` | 长按全部闪控球触发停止 |
| `MAIN_WINDOW_DESTROY_STOP` | context 关联的主窗口被销毁后触发停止 |
| `SQUEEZE` | 超出设备闪控球数量上限，被其他闪控球挤占停止 |
| `FLOAT_VIEW_STOP` | 与闪控窗绑定后，绑定状态下跟随闪控窗停止 |
| `STOP_IN_SIDEBAR` | 在侧边栏中被停止 |

## FloatingBallController.offDestroy (API 26+)

```typescript
offDestroy(callback?: Callback<string>): void
```

取消闪控球销毁事件的监听。

## 数据结构

### FloatingBallConfiguration

创建闪控球控制器时需要提供的参数配置。

| 名称 | 类型 | 只读 | 可选 | 说明 |
|:-----|:-----|:-----|:-----|:-----|
| context | BaseContext | 否 | 否 | 上下文环境，用于创建闪控球控制器时关联应用的主窗口。不能为空，通常传入 UIAbilityContext 对象 |

### FloatingBallParams

启动和更新闪控球的配置参数。

| 名称 | 类型 | 只读 | 可选 | 说明 |
|:-----|:-----|:-----|:-----|:-----|
| template | FloatingBallTemplate | 否 | 否 | 闪控球模板。不同模板对其他参数有不同要求 |
| title | string | 否 | 否 | 闪控球标题，不可为空字符串，大小不超过 64 字节 |
| content | string | 否 | 是 | 闪控球内容，大小不超过 64 字节。不传入时默认为空字符串 |
| backgroundColor | string | 否 | 是 | 闪控球背景颜色，不带透明度的十六进制颜色格式（如 `#008EF5` 或 `#FF008EF5`）。不传入时跟随系统深浅色模式默认背景色 |
| titleColor | string | 否 | 是 | 标题文字颜色，十六进制颜色格式。不传入时根据背景色色度自动填充（亮色填充黑色 `#E5000000`，暗色填充白色 `#E5FFFFFF`）。配置时必须同时配置 backgroundColor。（API 26+，Stage 模型） |
| contentColor | string | 否 | 是 | 内容文字颜色，十六进制颜色格式。不传入时根据背景色色度自动填充（亮色填充黑色 `#99000000`，暗色填充白色 `#99FFFFFF`）。配置时必须同时配置 backgroundColor。（API 26+，Stage 模型） |
| icon | image.PixelMap | 否 | 是 | 闪控球图标，图标像素总字节数不超过 192KB。建议图标像素宽高为 128px*128px |
| textUpdateAnimationType | FloatingBallTextUpdateAnimationType | 否 | 是 | 文本更新时的动画类型，默认为 `ANIMATION_NONE`。（API 26+，Stage 模型） |

### FloatingBallState

闪控球生命周期状态的枚举。

| 名称 | 值 | 说明 |
|:-----|:---|:-----|
| STARTED | 1 | 表示闪控球启动 |
| STOPPED | 2 | 表示闪控球停止 |

### FloatingBallTemplate

闪控球模板类型的枚举。

| 名称 | 值 | 说明 | 必传参数 |
|:-----|:---|:-----|:---------|
| STATIC | 1 | 静态布局，支持标题和图标 | title、icon |
| NORMAL | 2 | 普通文本布局，支持标题和内容 | title |
| EMPHATIC | 3 | 强调文本布局，支持图标、标题和内容 | title |
| SIMPLE | 4 | 纯文本布局，只支持标题 | title |

### FloatingBallWindowInfo

闪控球窗口信息。

| 名称 | 类型 | 只读 | 可选 | 说明 |
|:-----|:-----|:-----|:-----|:-----|
| windowId | number | 是 | 否 | 闪控球窗口 ID |

### FloatingBallTextUpdateAnimationType (API 26+)

闪控球文本更新动画类型的枚举。

| 名称 | 值 | 说明 |
|:-----|:---|:-----|
| ANIMATION_NONE | 0 | 无动画 |
| ANIMATION_OPACITY | 1 | 淡入淡出动画 |

## 错误码汇总

| 错误码 | 错误信息 | 适用接口 |
|:-------|:---------|:---------|
| 201 | Permission verification failed | startFloatingBall, restoreMainWindow |
| 801 | Capability not supported | create |
| 1300002 | This window state is abnormal | updateFloatingBall, getFloatingBallWindowInfo, restoreMainWindow |
| 1300003 | This window manager service works abnormally | updateFloatingBall, getFloatingBallWindowInfo, restoreMainWindow, setFloatingBallVisibilityInApp |
| 1300004 | Unauthorized operation | updateFloatingBall, getFloatingBallWindowInfo, restoreMainWindow |
| 1300019 | Wrong parameters for operating the floating ball | create, startFloatingBall, updateFloatingBall, on/off stateChange, on/off click, restoreMainWindow, onDestroy |
| 1300020 | Failed to create the floating ball window | startFloatingBall |
| 1300021 | Failed to start multiple floating ball windows | startFloatingBall |
| 1300022 | Repeated floating ball operation | startFloatingBall, stopFloatingBall, on stateChange, on click, onDestroy |
| 1300023 | Floating ball internal error | create, startFloatingBall, updateFloatingBall, stopFloatingBall, on/off stateChange, on/off click, getFloatingBallWindowInfo, restoreMainWindow, setFloatingBallVisibilityInApp, onDestroy, offDestroy |
| 1300024 | The floating ball window state is abnormal | startFloatingBall, updateFloatingBall, stopFloatingBall, on/off stateChange, on/off click, getFloatingBallWindowInfo, restoreMainWindow, setFloatingBallVisibilityInApp, onDestroy, offDestroy |
| 1300025 | The floating ball state does not support this operation | startFloatingBall, updateFloatingBall, getFloatingBallWindowInfo, restoreMainWindow |
| 1300026 | Failed to restore the main window | restoreMainWindow |
| 1300027 | When updating, the template type cannot be changed | updateFloatingBall |
| 1300028 | Updating static template-based floating balls is not supported | updateFloatingBall |
| 1300034 | Conflicts with other floating windows (App 已启动 float view) | startFloatingBall (26.0.0+) |

---

# 四、闪控球开发指南

## 前置准备

### 1. 权限申请

闪控球需要 `ohos.permission.USE_FLOAT_BALL` 权限，该权限为受限权限，需通过 ACL（Access Control List）方式申请。

在 `module.json5` 中声明权限：

```json
{
  "module": {
    "requestPermissions": [
      {
        "name": "ohos.permission.USE_FLOAT_BALL",
        "reason": "$string:reason_for_float_ball",
        "usedScene": {
          "abilities": ["EntryAbility"],
          "when": "inuse"
        }
      }
    ]
  }
}
```

### 2. 确认设备支持

使用前先判断设备是否支持闪控球功能：

```typescript
import { floatingBall } from '@kit.ArkUI';

let enable: boolean = floatingBall.isFloatingBallEnabled();
if (!enable) {
  console.warn('当前设备不支持闪控球功能');
  return;
}
```

## 开发步骤

### 步骤总览

1. 导入模块并声明闪控球控制器。
2. 使用 `create()` 接口创建闪控球控制器实例后注册点击事件回调和状态变化事件回调，通过 `startFloatingBall()` 接口启动闪控球。
3. 通过 `updateFloatingBall()` 更新闪控球信息，以此控制闪控球展示的内容。
4. 通过 `stopFloatingBall()` 停止闪控球。当不再需要显示闪控球时，可根据业务需要关闭闪控球。

### 步骤详解

#### 步骤 1：导入模块

```typescript
import { floatingBall } from '@kit.ArkUI';
import image from '@ohos.multimedia.image';
import { BusinessError } from '@kit.BasicServicesKit';
import { Want } from '@kit.AbilityKit';
```

#### 步骤 2：创建控制器并启动闪控球

```typescript
// 声明闪控球控制器实例
let floatingBallController: floatingBall.FloatingBallController | undefined = undefined;

// 在组件内获取 context
let ctx = this.getUIContext().getHostContext() as common.UIAbilityContext;

// 创建控制器
floatingBall.create({ context: ctx }).then((controller) => {
  floatingBallController = controller;

  // 注册点击事件回调
  floatingBallController.on('click', () => {
    let want: Want = {
      bundleName: 'com.example.myapp',
      abilityName: 'EntryAbility'
    };
    floatingBallController.restoreMainWindow(want);
  });

  // 注册状态变化回调
  floatingBallController.on('stateChange', (state: floatingBall.FloatingBallState) => {
    if (state === floatingBall.FloatingBallState.STOPPED) {
      floatingBallController?.off('click');
      floatingBallController?.off('stateChange');
      floatingBallController = undefined;
    }
  });

  // 启动闪控球
  let startParams: floatingBall.FloatingBallParams = {
    template: floatingBall.FloatingBallTemplate.NORMAL,
    title: '我的闪控球',
    content: '内容信息',
    backgroundColor: '#0ff77c'
  };
  floatingBallController.startFloatingBall(startParams);
});
```

#### 步骤 3：更新闪控球

```typescript
let updateParams: floatingBall.FloatingBallParams = {
  template: floatingBall.FloatingBallTemplate.NORMAL,  // 模板不可更改
  title: '新标题',
  content: '新内容',
  backgroundColor: '#f6ea0a'
};
floatingBallController?.updateFloatingBall(updateParams);
```

> **注意**：
> - 更新时模板类型 `template` 不可更改，否则返回错误码 1300027。
> - 静态布局（STATIC）不支持更新，否则返回错误码 1300028。

#### 步骤 4：停止闪控球

```typescript
floatingBallController?.stopFloatingBall().then(() => {
  console.info('Succeeded in stopping floating ball.');
});
```

> **注意**：stop 是异步流程，需要通过 `stateChange` 状态回调获取实际删除结果。

## 完整示例代码

### Utils.ts — 工具类

```typescript
import hilog from '@ohos.hilog';
import image from '@ohos.multimedia.image';
import { BusinessError } from '@kit.BasicServicesKit';
import { floatingBall } from '@kit.ArkUI';
import { Want } from '@kit.AbilityKit';
import { ContextUtil } from './ContextUtil';

const DOMAIN: number = 0xF811;
const TAG: string = '[Sample_FloatingBall]';
const BUNDLE_NAME: string = ContextUtil.context.abilityInfo.bundleName;

export class Utils {
    public static getRawfilePixelMapSync(path: string): image.PixelMap {
        try {
            const BUFFER = ContextUtil.context.resourceManager.getRawFileContentSync(path);
            const IMAGE_SOURCE: image.ImageSource = image.createImageSource(BUFFER.buffer as ArrayBuffer);
            hilog.debug(DOMAIN, TAG, `Get rawfile pixelMap path '${path}' successfully`);
            return IMAGE_SOURCE.createPixelMapSync();
        } catch (e) {
            hilog.error(DOMAIN, TAG, `Get rawfile pixelMap path '${path}' failed, error: ${e}`);
            throw e as Error;
        }
    }

    // 闪控球启动逻辑
    public static async onClickCreateFloatingBall(
        floatingBallController: floatingBall.FloatingBallController | undefined,
        template: floatingBall.FloatingBallTemplate,
        onActiveRowChange: (value: number) => void,
        title: string = 'title',
        content: string = 'content',
        backgroundColor: string = '#0ff77c',
        icon?: image.PixelMap): Promise<void> {
        // 注册监听点击回调事件
        try {
            floatingBallController?.on('click', () => {
                hilog.debug(DOMAIN, TAG, `FloatingBall onClickEvent`);
                let want: Want = {
                    bundleName: BUNDLE_NAME,
                    abilityName: 'MainAbility'
                }
                floatingBallController?.restoreMainWindow(want)
                .then(() => {
                    hilog.debug(DOMAIN, TAG, `Success in restoring FloatingBall main window`);
                }).catch((err: BusinessError) => {
                    hilog.error(DOMAIN, TAG, `failed to restore. code: ${err.code}, message: ${err.message}`);
                })
            })
        } catch (e) {
            hilog.error(DOMAIN, TAG, `Failed to register click listener: ${e}`);
        }
        // 注册监听状态变化事件
        try {
            floatingBallController?.on('stateChange',
            (state: floatingBall.FloatingBallState) => {
                hilog.debug(DOMAIN, TAG, `FloatingBall stateChange: ${state}`);
                if(state === floatingBall.FloatingBallState.STOPPED) {
                    floatingBallController?.off('click')
                    floatingBallController?.off('stateChange')
                    floatingBallController = undefined;
                    onActiveRowChange?.(-1);
                }
            })
        } catch (e) {
            hilog.error(DOMAIN, TAG, `Failed to register stateChange listener: ${e}`);
        }
        // 启动闪控球
        let startParams: floatingBall.FloatingBallParams = icon? {
            template: template,
            title: title,
            content: content,
            backgroundColor: backgroundColor,
            icon: icon
        } : {
            template: template,
            title: title,
            content: content,
            backgroundColor: backgroundColor
        }
        try {
            floatingBallController?.startFloatingBall(startParams)
            .then(() => {
                hilog.debug(DOMAIN, TAG, `succeed in starting FloatingBall`);
            }).catch((err: BusinessError) => {
                hilog.error(DOMAIN, TAG, `failed to start. code: ${err.code}, message: ${err.message}`);
            })
        } catch (e) {
            console.error('startFloatingBall Error', e)
        }
    }

    // 闪控球更新逻辑
    public static onClickUpdateFloatingBall(
        floatingBallController: floatingBall.FloatingBallController | undefined,
        template: floatingBall.FloatingBallTemplate,
        title: string = 'newTitle',
        content: string = 'newContent',
        icon?: image.PixelMap): void {
        let random_string: string = Math.floor(Math.random() * 100).toString();
        let updateParams: floatingBall.FloatingBallParams = icon ? {
            template: template,
            title: title + random_string,
            content: content + random_string,
            backgroundColor: '#f6ea0a',
            icon: icon
        } : {
            template: template,
            title: title + random_string,
            content: content + random_string,
            backgroundColor: '#f6ea0a',
        }
        try {
            floatingBallController?.updateFloatingBall(updateParams).then(() => {
                hilog.debug(DOMAIN, TAG, `Succeed in updating FloatingBall`);
            }).catch((err: BusinessError) => {
                hilog.error(DOMAIN, TAG, `failed to update. code: ${err.code}, message: ${err.message}`);
            })
        } catch (e) {
            console.error('updateFloatingBall Error:', e)
        }
    }

    // 闪控球停止逻辑
    public static onClickStopFloatingBall(
        floatingBallController: floatingBall.FloatingBallController | undefined): void {
        floatingBallController?.stopFloatingBall().then(() => {
            hilog.debug(DOMAIN, TAG, `Succeed in stopping FloatingBall`);
        }).catch((err: BusinessError) => {
            hilog.error(DOMAIN, TAG, `failed to stop. code: ${err.code}, message: ${err.message}`);
        })
    }
}
```

### Index.ets — 主页面

```typescript
import hilog from '@ohos.hilog';
import image from '@ohos.multimedia.image';
import { floatingBall } from '@kit.ArkUI';
import { Utils } from '../util/Utils';

const DOMAIN: number = 0xF811;
const TAG: string = '[Sample_FloatingBall]';

@Entry
@Component
struct Index {
  @State private activeRow: number = -1;
  private floatingBallController: floatingBall.FloatingBallController | undefined = undefined;
  private cachedIcon1: image.PixelMap | undefined = undefined;
  private cachedIcon2: image.PixelMap | undefined = undefined;

  private activeRowChange = (value: number) => { this.activeRow = value };

  private isEnabled(rowInex: number): boolean {
    return this.activeRow === -1 || this.activeRow === rowInex;
  }

  build() {
    Column({ space: 12 }) {
      // 静态布局（支持标题和图标，创建后无法修改）
      Row({ space: 6 }) {
        Button('STATIC').onClick(async () => {
          if (!this.floatingBallController) {
            this.floatingBallController = await floatingBall.create({
              context: this.getUIContext().getHostContext()
            })
          }
          if (this.floatingBallController) {
            if (!this.cachedIcon1) {
              let pixelMap = Utils.getRawfilePixelMapSync('books.png');
              if (pixelMap) {
                this.cachedIcon1 = pixelMap;
              }
            }
            Utils.onClickCreateFloatingBall(this.floatingBallController,
              floatingBall.FloatingBallTemplate.STATIC, this.activeRowChange,
              'title', 'content', '#0ff77c', this.cachedIcon1)
            this.activeRow = 0;
          }
        }).enabled(this.isEnabled(0))
        Button('Update1').enabled(false)  // 静态布局无法更新
        Button('Close1').onClick(() => {
          Utils.onClickStopFloatingBall(this.floatingBallController);
          this.activeRow = -1;
        }).enabled(this.isEnabled(0))
      }.width('100%').justifyContent(FlexAlign.Center)

      // 普通文本布局（支持标题和内容）
      Row({ space: 6 }) {
        Button('NORMAL').onClick(async () => {
          if (!this.floatingBallController) {
            this.floatingBallController = await floatingBall.create({
              context: this.getUIContext().getHostContext()
            })
          }
          if (this.floatingBallController) {
            Utils.onClickCreateFloatingBall(this.floatingBallController,
              floatingBall.FloatingBallTemplate.NORMAL, this.activeRowChange,
              'title', 'content')
            this.activeRow = 1;
          }
        }).enabled(this.isEnabled(1))
        Button('Update2').onClick(() => Utils.onClickUpdateFloatingBall(
          this.floatingBallController, floatingBall.FloatingBallTemplate.NORMAL))
          .enabled(this.isEnabled(1))
        Button('Close2').onClick(() => {
          Utils.onClickStopFloatingBall(this.floatingBallController);
          this.activeRow = -1;
        }).enabled(this.isEnabled(1))
      }.width('100%').justifyContent(FlexAlign.Center)

      // 强调文本布局（支持标题、图标和内容）
      Row({ space: 6 }) {
        Button('EMPHATIC').onClick(async () => {
          if (!this.floatingBallController) {
            this.floatingBallController = await floatingBall.create({
              context: this.getUIContext().getHostContext()
            })
          }
          if (this.floatingBallController) {
            if (!this.cachedIcon2) {
              let pixelMap = Utils.getRawfilePixelMapSync('video.png');
              if (pixelMap) {
                this.cachedIcon2 = pixelMap;
              }
            }
            Utils.onClickCreateFloatingBall(this.floatingBallController,
              floatingBall.FloatingBallTemplate.EMPHATIC, this.activeRowChange,
              '16', 'Min', '#0ff77c', this.cachedIcon2)
            this.activeRow = 2;
          }
        }).enabled(this.isEnabled(2))
        Button('Update3').onClick(() => Utils.onClickUpdateFloatingBall(
          this.floatingBallController, floatingBall.FloatingBallTemplate.EMPHATIC,
          '', 'Min', this.cachedIcon2)).enabled(this.isEnabled(2))
        Button('Close3').onClick(() => {
          Utils.onClickStopFloatingBall(this.floatingBallController);
          this.activeRow = -1;
        }).enabled(this.isEnabled(2))
      }.width('100%').justifyContent(FlexAlign.Center)

      // 纯文本布局（只支持标题）
      Row({ space: 6 }) {
        Button('SIMPLE').onClick(async () => {
          if (!this.floatingBallController) {
            this.floatingBallController = await floatingBall.create({
              context: this.getUIContext().getHostContext()
            })
          }
          if (this.floatingBallController) {
            Utils.onClickCreateFloatingBall(this.floatingBallController,
              floatingBall.FloatingBallTemplate.SIMPLE, this.activeRowChange, 'title')
            this.activeRow = 3;
          }
        }).enabled(this.isEnabled(3))
        Button('Update4').onClick(() => Utils.onClickUpdateFloatingBall(
          this.floatingBallController, floatingBall.FloatingBallTemplate.SIMPLE))
          .enabled(this.isEnabled(3))
        Button('Close4').onClick(() => {
          Utils.onClickStopFloatingBall(this.floatingBallController);
          this.activeRow = -1;
        }).enabled(this.isEnabled(3))
      }.width('100%').justifyContent(FlexAlign.Center)
    }.width('100%').height('100%').justifyContent(FlexAlign.Center)
  }
}
```

## 最佳实践

### 1. 控制器生命周期管理

- 创建控制器后及时注册 `stateChange` 监听，在 `STOPPED` 状态时取消所有监听并置空控制器引用，避免内存泄漏。
- `stopFloatingBall()` 是异步操作，需通过 `stateChange` 回调确认实际停止结果。

### 2. 图标缓存优化

- 图标 `PixelMap` 加载开销较大，建议缓存后复用，避免每次启动闪控球都重新加载。

### 3. Context 获取

- 请在组件内通过 `this.getUIContext().getHostContext()` 获取 context，确保返回结果为 `UIAbilityContext`。

### 4. 模板选择建议

| 场景 | 推荐模板 | 原因 |
|:-----|:---------|:-----|
| 需要图标+标题，内容固定不变 | STATIC | 静态布局，创建后不可更新 |
| 需要标题+内容，内容会变化 | NORMAL | 支持更新 |
| 需要图标+标题+内容 | EMPHATIC | 信息最丰富，支持更新 |
| 仅需简短文字提示 | SIMPLE | 最轻量，支持双行标题 |

### 5. 颜色配置

- `backgroundColor` 不传入时跟随系统深浅色模式默认背景色。
- `titleColor` / `contentColor` 不传入时根据背景色色度自动填充（亮色背景填充黑色，暗色背景填充白色）。
- 配置 `titleColor` / `contentColor` 时必须同时配置 `backgroundColor`，否则返回错误码 1300019。

### 6. 与闪控窗联动

通过 `floatView.bind` 接口可将闪控窗控制器与闪控球控制器绑定，实现点击闪控球展开为闪控窗、点击闪控窗缩小按钮收起为闪控球的相互切换。

### 7. 错误处理

- 始终使用 `try-catch` 包裹同步调用，使用 `.catch()` 处理 Promise 异步调用。
- 关注关键错误码：201（权限不足）、801（设备不支持）、1300022（重复操作）、1300027（模板不可更改）。

## 示例代码仓库

完整示例代码可参考：[闪控球示例](https://gitcode.com/HarmonyOS_Samples/guide-snippets/tree/master/ArkUISample/FloatingBall)