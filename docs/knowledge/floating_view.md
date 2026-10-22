# 闪控窗（FloatView）知识库

> 本知识库基于华为 HarmonyOS 官方文档整理：
> - 开发指南：https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/float-view-guide
> - API 参考：https://developer.huawei.com/consumer/cn/doc/harmonyos-references/js-apis-floatview
>
> 模块：`@ohos.window.floatView`，导入方式：`import { floatView } from '@kit.ArkUI';`
> 起始版本：**API 26.0.0**；系统能力：`SystemCapability.Window.SessionManager`；模型约束：**仅 Stage 模型**。

---

## 一、概述

### 1.1 定义
闪控窗是悬浮在桌面或其他应用界面上的小型窗口，为应用提供灵活的窗口管理能力。应用可以在小窗口中展示内容或提供快捷操作，用户可以在进行其他界面操作的同时查看闪控窗内容。闪控窗由**系统管理并统一绘制 UI**，动效高端精致，同时提供可绘制区域供应用加载指定页面内容。

### 1.2 适用场景
- **股市盯盘**：用户浏览其他应用时实时查看股票行情，无需频繁切换应用。
- **手机直播**：主播在直播过程中使用闪控窗展示自定义互动面板或控制界面，方便实时操作和互动。

### 1.3 闪控窗 vs 闪控球

| 对比项 | 闪控窗 | 闪控球 |
|:------|:------|:------|
| 显示形式 | 小型窗口，展示区域较大 | 小圆球，展示关键信息 |
| 贴边限制 | 无贴边限制 | 只能贴边展示 |
| UI 定制 | 存在模板，系统统一绘制，但提供可绘制区域加载页面 | 模板固定，应用不能定制 UI |
| 共同点 | 均为特殊应用辅助窗口，主窗口/UIAbility 退至后台后仍可前台显示 | 同左 |

### 1.4 闪控窗 vs 全局悬浮窗

| 对比项 | 闪控窗 | 全局悬浮窗 |
|:------|:------|:------|
| UI 绘制 | 系统管理并统一绘制，动效高端精致 | 开发者管理并实现 UI 绘制，无统一 UI 及动效 |
| 闪控球联动 | 支持与闪控球绑定联合使用 | 不支持 |
| 设备支持 | Phone、Tablet、2in1 | 仅 2in1 设备 |
| 共同点 | 均为特殊应用辅助窗口，主窗口/UIAbility 退至后台后仍可前台显示 | 同左 |

---

## 二、约束与限制

1. **权限**：需具有 `ohos.permission.FLOAT_VIEW`（user_grant 权限，需声明并向用户申请授权）。
2. **前台启动**：基于安全考虑，仅允许应用在前台时启动闪控窗。
3. **单实例**：同一应用只能启动一个闪控窗，启动多个返回错误码 `1300033`。
4. **互斥**：同一应用已启动闪控球或画中画窗口时，无法启动闪控窗，需先停止闪控球或画中画悬浮窗口（错误码 `1300034`）。
5. **模型约束**：仅支持 Stage 模型。
6. **设备判断**：使用前建议调用 `floatView.isFloatViewEnabled()` 或 `canIUse('SystemCapability.Window.SessionManager')` 判断设备是否支持。

---

## 三、交互方式

| 交互方式 | 说明 |
|:---------|:-----|
| 拖动窗口 | 手动拖拽拖动热区改变位置，自动避让状态栏、导航条、输入法键盘等系统组件 |
| 标题栏操作 | 点击关闭按钮关闭闪控窗；与闪控球绑定时点击缩小按钮切换为闪控球；未绑定时点击最小化按钮收起到侧边栏 |
| 垃圾桶删除 | Phone（非自由多窗模式）和 Tablet（非自由多窗模式、非电脑模式）支持拖拽到垃圾桶区域松手删除 |
| 侧边栏功能 | Phone（非自由多窗模式）和 Tablet（非自由多窗模式、非电脑模式）可进入系统侧边栏暂存 |
| 与闪控球切换 | 绑定状态下点击触发互相切换；切换为闪控球后状态变为 `IN_FLOATING_BALL` |

---

## 四、模板类型

| 枚举值 | 名称 | 值 | 说明 |
|:------|:------|:-|:-----|
| `FloatViewTemplateType.ROUNDED_RECTANGLE` | 圆角矩形 | 0 | 圆角矩形模板 |
| `FloatViewTemplateType.HORIZONTAL_BAR` | 水平条状矩形 | 1 | 水平的条状矩形模板 |

---

## 五、API 详解

### 5.1 模块级接口

#### 5.1.1 floatView.isFloatViewEnabled()
判断当前设备是否支持闪控窗功能。

```typescript
let enable: boolean = floatView.isFloatViewEnabled();
```

#### 5.1.2 floatView.create(config: FloatViewConfiguration): Promise<FloatViewController>
创建闪控窗控制器。`config` 及其 `context` 字段不能为 null/undefined（否则抛 401），其他参数异常抛 1300016。

```typescript
let ctx = this.getUIContext().getHostContext() as common.UIAbilityContext;
let config: floatView.FloatViewConfiguration = {
  context: ctx,
  templateType: floatView.FloatViewTemplateType.ROUNDED_RECTANGLE
};
this.floatViewController = await floatView.create(config);
```

**错误码**：801（能力不支持）、1300002（窗口状态异常）、1300016（参数错误，模板类型无效）。

#### 5.1.3 floatView.bind(floatViewController, floatingBallController, floatingBallParams): Promise<void>
绑定闪控窗和闪控球。需先创建两个控制器且均未启动。
- 绑定后调用任一控制器启动接口均同时创建两个窗口，同一时刻仅展示其中一个（展示顺序取决于先调用哪个启动接口）。
- 绑定后用户点击可在两者间切换。
- 绑定后调用任一控制器停止接口均同时销毁两个窗口。

**权限**：`ohos.permission.USE_FLOAT_BALL` 和 `ohos.permission.FLOAT_VIEW`

```typescript
const ballParams: floatingBall.FloatingBallParams = {
  template: floatingBall.FloatingBallTemplate.EMPHATIC,
  title: "标题",
  content: "正文"
};
await floatView.bind(fvController, fbController, ballParams);
```

**错误码**：201（权限失败）、801、1300019（闪控球参数错误）、1300025（闪控球状态不支持）、1300031（闪控窗状态不支持）。

#### 5.1.4 floatView.unbind(floatViewController, floatingBallController): Promise<void>
解绑闪控窗和闪控球。需两个控制器均停止后才可解绑。

```typescript
await floatView.unbind(this.floatViewController, this.floatingBallController);
```

**错误码**：801、1300025、1300031。

#### 5.1.5 floatView.getFloatViewLimits(templateType: FloatViewTemplateType): FloatViewLimits
根据模板类型获取闪控窗窗口限制（单位 px），包括最大/最小尺寸和宽高比限制范围。

```typescript
let limits: floatView.FloatViewLimits =
  floatView.getFloatViewLimits(floatView.FloatViewTemplateType.ROUNDED_RECTANGLE);
```

**错误码**：801、1300002、1300003（窗口管理服务异常）、1300016。

---

### 5.2 实例级接口（FloatViewController）

下列接口均需先通过 `floatView.create()` 获取控制器实例。

#### 5.2.1 setUIContext(path: string, storage?: LocalStorage): Promise<void>
根据工程中指定页面路径为闪控窗加载页面内容，可通过 LocalStorage 传递状态属性。
- 建议在启动前调用；重复调用先销毁旧 UIContent 再加载新内容。
- 不支持跨包页面，跨包请用 `setUIContextByName()`。
- `path` 需添加到 `main_pages.json`，不支持相对路径。

```typescript
await this.floatViewController.setUIContext('pages/FloatViewPage');
```

**错误码**：1300002、1300016（路径无效）。

#### 5.2.2 setUIContextByName(name: string, storage?: LocalStorage): Promise<void>
根据命名路由页面名称加载页面。`name` 需与 `@Entry` 装饰器中 `routeName` 参数一致。

```typescript
await this.floatViewController.setUIContextByName(entryName);
```

**错误码**：1300002、1300016（名称无效）。

#### 5.2.3 setWindowSize(size: window.Size): Promise<void>
设置闪控窗窗口大小（单位 px）。建议先调用 `getFloatViewLimits()` 获取推荐范围。实际大小变化通过 `onRectChange` 监听。

```typescript
let size: window.Size = { width: 400, height: 600 };
await this.floatViewController.setWindowSize(size);
```

**错误码**：1300002、1300003、1300016（size <= 0）。

#### 5.2.4 switchTemplate(templateProperty: TemplateProperty): Promise<void>
切换闪控窗模板并改变窗口尺寸。建议先调用 `getFloatViewLimits()` 获取目标模板推荐范围。

```typescript
let templateProperty: floatView.TemplateProperty = {
  templateType: floatView.FloatViewTemplateType.HORIZONTAL_BAR,
  size: { width: 800, height: 100 }
};
await this.floatViewController.switchTemplate(templateProperty);
```

**错误码**：1300002、1300003、1300016（模板类型无效或 size <= 0）。

#### 5.2.5 start(): Promise<void>
启动闪控窗窗口。接口返回不代表启动完成，需通过 `onStateChange` 监听到 `STARTED` 判断启动成功。建议在 `setUIContext()` / `setUIContextByName()` 后调用。

**权限**：`ohos.permission.FLOAT_VIEW`

```typescript
await this.floatViewController.start();
```

**错误码**：201、1300002、1300003、1300030（重复操作，正在启动或已启动）、1300031（正在停止）、1300033（启动多个闪控窗或主窗口不在前台）、1300034（与其他悬浮窗冲突，已启动闪控球或画中画）。

#### 5.2.6 stop(): Promise<void>
停止闪控窗窗口。接口返回不代表停止完成，需通过 `onStateChange` 监听到 `STOPPED` 判断停止成功。

```typescript
await this.floatViewController.stop();
```

**错误码**：1300002、1300003、1300030（正在停止或已停止）、1300031（窗口未启动）。

#### 5.2.7 setFloatViewVisibilityInApp(isVisible: boolean): Promise<void>
设置应用在前台时闪控窗是否可见。创建后未调用前默认可见。

```typescript
await this.floatViewController.setFloatViewVisibilityInApp(true);
```

**错误码**：1300002、1300003。

#### 5.2.8 restoreMainWindow(wantParameters?: Record<string, Object>): Promise<void>
恢复闪控窗的主窗口到前台显示。若主窗口已在前台则抬升层级。
- 要求闪控窗处于 `STARTED` 状态，且只能在用户点击闪控窗窗口后调用。
- 主窗口处于 PAUSED 生命周期或多任务状态时抛 1300032。
- `wantParameters` 会在主窗口 `onNewWant` 回调中收到。

```typescript
let param: Record<string, Object> = { 'info': 'helloworld' };
await this.floatViewController.restoreMainWindow(param);
```

**错误码**：1300002、1300003、1300031（未启动时恢复）、1300032（恢复失败：用户未点击过/闪控窗不在前台/主窗口 PAUSED/多任务后台）。

#### 5.2.9 getWindowProperties(): FloatViewProperties
获取闪控窗窗口属性。需窗口已启动。

```typescript
let properties = this.floatViewController.getWindowProperties();
```

**错误码**：1300002、1300031（未启动/已停止/错误状态）。

#### 5.2.10 onStateChange / offStateChange
注册/取消注册闪控窗状态变化监听。不再使用时需取消监听避免内存泄漏。

```typescript
this.floatViewController.onStateChange((info: floatView.FloatViewStateChangeInfo) => {
  console.info(`State: ${info.state}, reason: ${info.stopReason}`);
});
```

**错误码**：1300002、1300030（回调已注册）。

#### 5.2.11 onRectChange / offRectChange
注册/取消注册闪控窗矩形区域（位置和大小）变化监听。

```typescript
this.floatViewController.onRectChange((info: floatView.FloatViewRectChangeInfo) => {
  console.info(`Rect: ${JSON.stringify(info.windowRect)}, reason: ${info.reason}`);
});
```

#### 5.2.12 onLimitsChange / offLimitsChange
注册/取消注册闪控窗限制变化监听。当限制规格变化时（如折叠展开导致屏幕宽度变化或切换模板）触发回调。

```typescript
this.floatViewController.onLimitsChange((limits: floatView.FloatViewLimits) => {
  console.info(`Limits: minSize=${limits.minSize}, maxSize=${limits.maxSize}`);
});
```

---

## 六、数据结构

### 6.1 FloatViewConfiguration
创建闪控窗控制器时的参数配置。

| 名称 | 类型 | 必填 | 说明 |
|:-----|:-----|:-|:-----|
| context | BaseContext | 是 | 上下文环境，必须传入有效的 UIAbilityContext 实例 |
| templateType | FloatViewTemplateType | 是 | 闪控窗模板类型 |
| isConfirmOnClose | boolean | 否 | 点击关闭按钮时是否需用户确认，默认 false |

### 6.2 TemplateProperty
切换模板并修改窗口尺寸时的参数配置。

| 名称 | 类型 | 必填 | 说明 |
|:-----|:-----|:-|:-----|
| templateType | FloatViewTemplateType | 是 | 闪控窗模板类型 |
| size | window.Size | 是 | 更新模板类型时的窗口尺寸 |

### 6.3 FloatViewProperties
闪控窗窗口属性。

| 名称 | 类型 | 说明 |
|:-----|:-----|:-----|
| templateType | FloatViewTemplateType | 模板类型 |
| windowId | number | 窗口 ID |
| displayId | number | 所在屏幕 ID |
| windowRect | window.Rect | 窗口矩形区域 |
| windowScale | number | 窗口缩放比例 |
| avoidArea | window.AvoidArea | 内容避让区域（避让区域内组件不响应手势事件） |
| inSidebar | boolean | 是否在侧边栏中 |

### 6.4 FloatViewLimits
闪控窗窗口限制。

| 名称 | 类型 | 说明 |
|:-----|:-----|:-----|
| minSize | window.Size | 最小尺寸 |
| maxSize | window.Size | 最大尺寸 |
| ratioLimits | Array<RatioLimit> | 宽高比限制范围数组 |

### 6.5 RatioLimit
宽高比限制范围（宽高比 = 窗口矩形区域宽 / 高）。

| 名称 | 类型 | 说明 |
|:-----|:-----|:-----|
| minRatio | number | 宽高比最小值 |
| maxRatio | number | 宽高比最大值 |

### 6.6 FloatViewStateChangeInfo
状态变化信息。

| 名称 | 类型 | 说明 |
|:-----|:-----|:-----|
| state | FloatViewState | 闪控窗状态 |
| stopReason | string | 停止原因（仅 STOPPED 状态有效） |

**stopReason 取值**：
- `"APP_STOP"`：应用主动停止
- `"STOP_IN_SIDEBAR"`：在侧边栏被关闭
- `"TITLE_BAR_STOP_CLICK"`：标题栏点击关闭按钮
- `"DUMPSTER_STOP"`：拖入垃圾桶停止
- `"REPLACE_STOP"`：被其他闪控窗挤占
- `"FLOATING_BALL_STOP"`：绑定状态下跟随闪控球停止
- `"MAIN_WINDOW_DESTROY_STOP"`：context 关联的主窗被销毁后停止

### 6.7 FloatViewState（枚举）

| 名称 | 值 | 说明 |
|:-----|:-|:-----|
| STARTED | 1 | 已启动并显示 |
| HIDDEN | 2 | 已隐藏（上滑多任务或 setFloatViewVisibilityInApp 设置前台隐藏） |
| STOPPED | 3 | 已停止 |
| IN_SIDEBAR | 4 | 在侧边栏中 |
| IN_FLOATING_BALL | 5 | 切换为闪控球 |
| ERROR | 6 | 发生异常 |

### 6.8 FloatViewRectChangeInfo
矩形区域变化信息。

| 名称 | 类型 | 说明 |
|:-----|:-----|:-----|
| windowRect | window.Rect | 窗口矩形区域 |
| windowScale | number | 窗口缩放比例 |
| reason | string | 变化原因：`"POSITION_CHANGE"` / `"SIZE_CHANGE"` / `"RECT_CHANGE"` |

---

## 七、错误码汇总

| 错误码 | 含义 | 常见触发场景 |
|:------|:-----|:-----------|
| 201 | 权限验证失败 | 未申请 FLOAT_VIEW / USE_FLOAT_BALL 权限 |
| 401 | 参数为 null/undefined | create 的 config 或 context 为空 |
| 801 | 能力不支持 | 设备不支持 SystemCapability.Window.SessionManager |
| 1300002 | 窗口状态异常 | 控制器对象为 null、上下文异常、系统错误 |
| 1300003 | 窗口管理服务异常 | 内部 IPC 错误 |
| 1300016 | 参数错误 | 模板类型无效、路径/名称无效、size <= 0 |
| 1300019 | 闪控球参数错误 | bind 时 floatingBallParams 无效 |
| 1300025 | 闪控球状态不支持操作 | 闪控球已启动未停止、已绑定 |
| 1300030 | 闪控窗重复操作 | 正在启动/已启动、正在停止/已停止、回调已注册 |
| 1300031 | 闪控窗状态不支持操作 | 正在停止、未启动、已绑定、未绑定、解绑时非绑定对 |
| 1300032 | 恢复主窗口失败 | 用户未点击过闪控窗、闪控窗不在前台、主窗口 PAUSED、多任务后台 |
| 1300033 | 启动闪控窗失败 | 启动多个闪控窗、主窗口不在前台 |
| 1300034 | 与其他悬浮窗冲突 | 已启动闪控球或画中画窗口 |

---

## 八、开发场景与示例

### 8.1 基础场景：单独操作闪控窗

完整流程：申请权限 → 创建控制器 → 注册回调 → 设置页面内容 → 获取尺寸限制并设置大小 → 启动 → （监听状态）→ 停止。

```typescript
import { floatView, window } from '@kit.ArkUI';
import { common } from '@kit.AbilityKit';
import { abilityAccessCtrl, bundleManager } from '@kit.AbilityKit';
import { BusinessError } from '@kit.BasicServicesKit';

@Entry
@Component
struct Index {
  private floatViewController: floatView.FloatViewController | undefined = undefined;
  @State result: string = '';

  // 申请权限
  requestPermission(): void {
    let atManager = abilityAccessCtrl.createAtManager();
    atManager.requestPermissionsFromUser(getContext(this), ['ohos.permission.FLOAT_VIEW'] as Permissions[])
      .then((data) => {
        this.result = data.authResults[0] === 0 ? '权限已授权' : '权限被拒绝';
      })
      .catch((reason: BusinessError) => {
        console.error(`requestPermissionsFromUser failed, ${reason?.code}, ${reason?.message}.`);
      });
  }

  // 创建并启动闪控窗
  async createWindow(): Promise<void> {
    if (!this.floatViewController) {
      let ctx = this.getUIContext().getHostContext() as common.UIAbilityContext;
      let floatConfig: floatView.FloatViewConfiguration = {
        context: ctx,
        templateType: floatView.FloatViewTemplateType.ROUNDED_RECTANGLE
      };
      this.floatViewController = await floatView.create(floatConfig);
      this.registerStateChangeCallback();
      this.registerLimitsChangeCallback();
    }
    await this.floatViewController.setUIContext('pages/FloatViewPage');
    let limits = floatView.getFloatViewLimits(floatView.FloatViewTemplateType.ROUNDED_RECTANGLE);
    await this.floatViewController.setWindowSize({ width: limits.maxSize.width, height: limits.maxSize.height });
    await this.floatViewController.start();
  }

  public registerLimitsChangeCallback(): void {
    this.floatViewController?.onLimitsChange((limits: floatView.FloatViewLimits) => {
      console.info(`Limits changed: minSize=${limits.minSize}, maxSize=${limits.maxSize}`);
    });
  }

  public registerStateChangeCallback(): void {
    this.floatViewController?.onStateChange((info: floatView.FloatViewStateChangeInfo) => {
      console.info(`State changed: ${info.state}, reason: ${info.stopReason}`);
      if (info.state === floatView.FloatViewState.STOPPED) {
        this.floatViewController?.offStateChange();
        this.floatViewController?.offLimitsChange();
        this.floatViewController = undefined;
      }
    });
  }

  // 停止闪控窗
  deleteAll(): void {
    if (this.floatViewController) {
      this.floatViewController.stop().then(() => {
        this.floatViewController = undefined;
      }).catch((err: BusinessError) => {
        this.floatViewController = undefined;
      });
    }
  }
}
```

### 8.2 复杂场景：与闪控球绑定使用

需同时具有 `ohos.permission.USE_FLOAT_BALL` 和 `ohos.permission.FLOAT_VIEW` 权限。

```typescript
import { floatingBall, floatView } from '@kit.ArkUI';

// 创建两个控制器
let ctx = this.getUIContext().getHostContext() as common.UIAbilityContext;
this.floatingBallController = await floatingBall.create({ context: ctx });
this.floatViewController = await floatView.create({
  context: ctx,
  templateType: floatView.FloatViewTemplateType.ROUNDED_RECTANGLE
});

// 绑定
const ballParams: floatingBall.FloatingBallParams = {
  template: floatingBall.FloatingBallTemplate.EMPHATIC,
  title: "标题",
  content: "正文"
};
await floatView.bind(this.floatViewController, this.floatingBallController, ballParams);

// 设置页面、尺寸并启动（绑定状态下同时启动闪控球）
await this.floatViewController.setUIContext('pages/FloatViewPage');
let limits = floatView.getFloatViewLimits(floatView.FloatViewTemplateType.ROUNDED_RECTANGLE);
await this.floatViewController.setWindowSize({ width: limits.maxSize.width, height: limits.maxSize.height });
await this.floatViewController.start();

// 解绑（需两个控制器均停止后）
await floatView.unbind(this.floatViewController, this.floatingBallController);
```

### 8.3 复杂场景：与防窥保护组合使用

闪控窗支持与防窥保护（DlpAntiPeep）联动。系统检测到非机主窥视时，在闪控窗拉起蒙层遮盖内容。

**前提条件**：
- 权限：`ohos.permission.FLOAT_VIEW`、`ohos.permission.DLP_GET_HIDE_STATUS`
- 系统设置 > 隐私与安全 > 防窥保护中打开应用开关
- 通过 `canIUse('SystemCapability.Security.DlpAntiPeep')` 判断设备是否支持

**核心步骤**：

1. 封装防窥保护工具类（`canUseAntiPeep`、`isAntiPeepOn`、`getAntiPeepInfo`、`showSystemMaskLayer`）
2. 注册防窥状态监听 `dlpAntiPeep.on('dlpAntiPeep', callback)`，状态为 `HIDE` 时调用 `dlpAntiPeep.setAntiPeepMaskLayer(windowId)` 拉起蒙层
3. 初始化防窥状态并注册回调，申请闪控窗权限
4. 创建并启动闪控窗
5. 停止闪控窗

```typescript
import { dlpAntiPeep } from '@kit.SecurityKit';

// 判断是否支持防窥保护
export function canUseAntiPeep(): boolean {
  return canIUse('SystemCapability.Security.DlpAntiPeep');
}

// 防窥状态回调处理
private async handleAntiPeepStatus(status: dlpAntiPeep.DlpAntiPeepStatus) {
  switch (status) {
    case dlpAntiPeep.DlpAntiPeepStatus.HIDE:
      let window: window.Window = AppStorage.get('MAIN_WINDOW') as window.Window;
      const windowId: number = window.getUIContext().getWindowId() as number;
      dlpAntiPeep.setAntiPeepMaskLayer(windowId)
        .then((result) => console.info(`setAntiPeepMaskLayer success + ${result}`))
        .catch((err: BusinessError) => console.error(`setAntiPeepMaskLayer failed: ${err.code}`));
      break;
    case dlpAntiPeep.DlpAntiPeepStatus.PASS:
      console.info('DlpAntiPeepStatus is PASS');
      break;
  }
}

// 注册防窥状态监听
dlpAntiPeep.on('dlpAntiPeep', (status: dlpAntiPeep.DlpAntiPeepStatus) => {
  this.handleAntiPeepStatus(status);
});
```

---

## 九、最佳实践

1. **先判断设备支持**：调用 `floatView.isFloatViewEnabled()` 判断设备是否支持闪控窗。
2. **先获取尺寸限制**：设置窗口大小前先调用 `getFloatViewLimits()` 获取推荐范围，超出范围系统会调整到范围内。
3. **监听限制变化**：通过 `onLimitsChange()` 监听限制变化（如折叠展开），动态调整窗口尺寸。
4. **监听实际尺寸**：通过 `onRectChange()` 监听窗口实际大小变化。
5. **正确判断启动/停止**：`start()` / `stop()` 返回不代表流程结束，需通过 `onStateChange` 监听 `STARTED` / `STOPPED`。
6. **及时取消监听**：不再使用时取消监听（`offStateChange`、`offRectChange`、`offLimitsChange`）避免内存泄漏。
7. **设置页面后再启动**：建议在 `setUIContext()` / `setUIContextByName()` 后调用 `start()`。
8. **避让区域注意**：通过 `setUIContext` 加载的页面中，避让区域内的组件不响应手势事件，添加手势响应组件时需避让这些区域。
9. **前台启动**：仅允许应用在前台时启动闪控窗，主窗口不在前台会返回 1300033。
10. **互斥处理**：启动闪控窗前确保未启动闪控球或画中画窗口，否则返回 1300034。
11. **绑定场景权限**：与闪控球绑定时需同时具有 `USE_FLOAT_BALL` 和 `FLOAT_VIEW` 权限。
12. **解绑时机**：`unbind()` 需在两个控制器均停止后才可调用。