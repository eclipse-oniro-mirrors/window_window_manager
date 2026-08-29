# 功能方案设计：窗口最小尺寸限制按工作区比例封顶

## 1. 背景与问题

某产品上出现应用将自身 WindowLimits 的最小宽度/高度设置过大，导致窗口在屏幕内显示过满且无法向下缩放的问题。

本方案针对该类设备，在窗口 limits 计算漏斗中增加一道"按屏幕工作区比例封顶最小尺寸"的处理，使窗口始终保有向下缩放的空间。

### 1.1 处理规则（需求原始定义）

设屏幕工作区为 WA（宽 waW、高 waH），封顶系数 k（配置值，如 0.81），
capW = k × waW，capH = k × waH。

前置条件（配置门控，最高优先级）：`limitsThresholdEnabled.enable = true` 且
percentage ∈ (0, 100)（k = percentage / 100）。任一不满足时，本规则整体不
生效——不查询工作区、不做触发判断、不做任何封顶计算，漏斗行为与主线完全一致。

触发条件（前置条件满足后）：应用设置的最小宽度/高度中任意一项超出工作区
宽高的 k 比例。

1. 窗口宽高的最小限制都不能超过屏幕工作区宽高的 k 比例；
2. 若应用设置的窗口最小宽度超过 capW，则窗口可缩放到 capW；
3. 若应用设置的窗口最小高度超过 capH，则窗口可缩放到 capH；
4. 若应用设置了宽高比 R，先使窗口最小高度不超过 capH，再根据 R 反推最小宽度；
   - 4.1 若反推出的最小宽度超过 capW，则限制最小宽度为 capW，再根据 R 反算最小高度
     （极端比例下反算结果仍超过 capH 时，k 比例上限优先，ratio 允许近似偏差）；
   - 4.2 若反推出的最小宽度未超过 capW，则按反推结果决定最小宽度。

计算形式化：

```text
前置门控:
    enabled == false 或 k ∉ (0, 1)  →  整体短路，后续通路均不执行

触发: newLimits.minWidth_ > capW || newLimits.minHeight_ > capH

无 ratio:
    minW' = min(minW, capW)
    minH' = min(minH, capH)                      // 规则 1/2/3

有 ratio R:
    minH1 = min(minH, capH)                      // 规则 4
    minW1 = ceil(minH1 × R)
    if minW1 > capW:                             // 规则 4.1
        minW' = capW
        minH' = min(ceil(capW / R), capH)        // 无解时 81% 上限优先
    else:
        minW' = minW1                            // 规则 4.2
        minH' = minH1
```

### 1.2 关键决策记录（已与需求方确认）

| 决策点 | 结论 |
|---|---|
| 规则 4 的"宽高比"来源 | 应用通过 set 接口（setAspectRatio / setContentAspectRatio）设置的优先；未设置则使用 limits 内置 ratio 字段 |
| "屏幕工作区"取值 | 通过 DMS 已有接口获取（`Display::GetAvailableArea`） |
| 生效窗口范围 | 全部窗口类型 |
| 设备开关机制 | window_manager_config.xml 新增两个字段：bool 开关 + int 百分比（后续演进为 system parameter 方案，见 3.2） |
| 触发判断对象 | 计算漏斗输出的最终生效值（而非应用设置的原始值） |
| 规则 4.1 反算后仍超 capH 的极端场景 | k 比例上限优先，ratio 允许近似偏差 |
| 配置门控语义 | 开关关闭或 k 非法时前置短路，工作区查询、触发判断、封顶计算整条通路均不执行 |

## 2. 现状分析

### 2.1 相关代码路径

统一架构（window_manager_use_sceneboard = true）下，所有窗口类型的 limits 计算收敛于客户端单一漏斗：

```text
WindowSceneSessionImpl::UpdateWindowSizeLimits (wm/src/window_scene_session_impl.cpp:1895)
 ├─ CalculateNewLimitsByLimits (:1701)      系统限制 + 自定义值（userLimits_ / configLimitsVP_）钳制
 │   ├─ GetSystemSizeLimits (:1634)         系统边界：maxFloatingWindowSize_ 为 max，
 │   │   │                                  最小值按窗口类型取 miniWidthOfMainWindow_ 等
 │   │   └─ ApplyForcibleLimits (:1603)     强制模式突破系统限制
 │   ├─ ProcessVirtualPixelLimits (:1762)   VP 单位钳制路径
 │   └─ ProcessPhysicalPixelLimits (:1787)  PX 单位钳制路径
 ├─ CalculateNewLimitsByRatio (:1817)       ratio 融合与尺寸反算
 ├─ 系统窗 1px 补丁 (:1932)
 ├─ SetLimitsForAttachedWindows (:1944)     挂靠通知基线
 ├─ CalculateAttachedWindowLimitsIntersection (:1952)  挂靠窗口交集
 └─ SetWindowLimits / SetWindowLimitsVP (:1962)        结果写回 property
```

该漏斗的触发路径覆盖：窗口创建（主窗口 :740 / 子窗口与系统窗 :745）、`SetWindowLimits`（:7425）、
resize 时 vpr 变化（:3091）、密度变化（:7813）、挂靠窗口约束更新（:9009-9112）。
计算结果经 `ACTION_UPDATE_WINDOW_LIMITS` IPC 同步到服务端（`SceneSession::HandleActionUpdateWindowLimits`，
window_scene/session/host/src/scene_session.cpp:7787，纯拷贝不重算），布局
（`LayoutController`）与拖拽（`MoveDragController`）等消费端只读 property。

### 2.2 代码事实

- 代码库中**没有**现成的"工作区（WorkArea）"概念；DMS 提供现成接口：
  `Display::GetAvailableArea(DMRect&)`（interfaces/innerkits/dm/display.h:177，语义为
  "扣除 dock 和状态栏的屏幕区域"），以及 `RegisterAvailableAreaChangedListener`
  变更监听（interfaces/innerkits/dm/display_manager.h:678）。
- 产品差异化配置走 `window_scene_config.xml` →
  `SceneSessionManager::ConfigWindowSizeLimits`（window_scene/session_manager/src/scene_session_manager.cpp:2020）
  → `WindowSystemConfig`（window_scene/common/include/window_session_property.h:976），
  经 Marshalling/Unmarshalling（:1030/:1135）IPC 同步到客户端。本需求使用
  window_manager_config.xml，加载机制同类。

## 3. 方案设计

### 3.1 总体思路

在 `UpdateWindowSizeLimits` 漏斗内、`CalculateNewLimitsByRatio` 之后插入新步骤
`AdjustMinLimitsByWorkArea`，在 PX 侧完成封顶计算后换算 VP 侧。不改动任何消费端
（LayoutController / MoveDragController / UpdateFloatingWindowSizeBySizeLimits），
服务端同步与 `NotifySessionWindowLimitsChange` 通知复用现有机制。

插入位置：

```text
CalculateNewLimitsByLimits (:1904)   系统限制 + 自定义值钳制
CalculateNewLimitsByRatio (:1920)    ratio 融合
★ AdjustMinLimitsByWorkArea          ← 新增：触发判断 + 最小值封顶（PX 计算，VP 换算）
系统窗 1px 补丁 (:1932)
SetLimitsForAttachedWindows (:1944)
attached 交集 (:1952)
SetWindowLimits / SetWindowLimitsVP (:1962)
```

选点理由：

1. **唯一收敛点**：所有触发路径（创建 / setWindowLimits / resize / 密度 / 挂靠更新）
   均经过此漏斗，一处改动覆盖全部窗口类型；
2. **PX/VP 双份自动一致**：漏斗输出同时写 limits_ / limitsVP_，避免双单位不同步；
3. **置于 attached 交集之前一次即可**：交集对 min 取 max(本窗 min, 挂靠 min)，
   若各窗漏斗输出均已 ≤ cap，则 max 后仍 ≤ cap，不变式自动保持；
4. **置于系统窗 1px 补丁之前**：避免补丁（min 强制 1px）与封顶相互覆盖，
   1px 远小于 cap，先封顶后补丁不冲突。

### 3.2 配置层（system parameter 方案，已实现）

window_manager_config.xml 的 `<windowLayout>` 节点下新增（嵌套结构）：

```xml
<windowLayout>
    <windowLimitsThreshold>
        <limitsThresholdEnabled enable="true">
            <limitsThresholdPercentage>81</limitsThresholdPercentage>
        </limitsThresholdEnabled>
    </windowLimitsThreshold>
</windowLayout>
```

数据链路：

```
xml（ConfigWindowLayout → ConfigWindowLimitsThreshold → ConfigWindowLimitsPercentage）
  → WindowLimitsThreshold::SaveLimitsThresholdConfig
      → system parameter 持久化:
          persist.windowlayout.windowLimitsThreshold.enable
          persist.windowlayout.windowLimitsThreshold.limitsthresholdpercentage
      → 进程内静态缓存 limitsThresholdConfig_（optional）
  → 客户端 AdjustMinLimitsByWorkArea 读取（LimitsThresholdEnabled + LoadLimitsThresholdConfig）
```

配置结构（window_scene/common/include/window_limits_threshold.h，
静态工具类 window_limits_threshold.cpp 随 window_scene_common 编译）：

| 字段 | 类型 | 语义 | 默认值 |
|---|---|---|---|
| enable | bool | 功能开关 | false |
| limitsThresholdPercentage | int32_t | 封顶百分比（0-100），k = percentage / 100 | 0（无效） |

方案决策记录（相比初版 SystemSessionConfig 序列化方案的变更）：

1. **初版方案**将两字段挂在 SystemSessionConfig 上随 Connect IPC 下发——实测暴露
   ABI/序列化布局混布风险：SystemSessionConfig 的 Marshalling 是头文件内联代码，
   被多个 so 各自编译副本，设备上部分 so 更新部分未更新时（如
   libscenesessionmanager_napi 读旧布局对象）直接 cppcrash（曾导致
   com.ohos.sceneboard 无法开机，见 6.4）。
2. **现方案**改用 system parameter：服务端解析 xml 后写 parameter，客户端进程
   读 parameter——配置不经过任何自定义 parcel 结构，**天然免疫 so 混布的序列化
   布局问题**；且 parameter 全设备共享，客户端无需等待 Connect 即可读取。
3. 解析侧防御：enable 属性缺失/非 bool、percentage 非 INTS/数量非 1/越界
   (0, 100) 时打 TLOGW 并保持默认（功能关闭）；xml 未配置该节点时整链跳过。
4. 客户端读取路径：`LimitsThresholdEnabled()` 与 `LoadLimitsThresholdConfig()`
   均可用；热路径建议统一走 `GetLimitsThresholdConfig()`（缓存命中，免重复读参）。

### 3.3 计算实现（核心新增）

`window_scene_session_impl.h` 新增声明（`AdjustMinLimitsByWorkArea` /
`GetEffectiveAspectRatio` 及成员标志 `isMinLimitsAdjusted_`），
`wm/src/window_scene_session_impl.cpp` 实现：

```cpp
bool WindowSceneSessionImpl::AdjustMinLimitsByWorkArea(WindowLimits& newLimits,
    WindowLimits& newLimitsVP, float vpr)
{
    // 前置门控：开关关闭时直接短路返回，
    // 后续通路（工作区查询、触发判断、封顶计算）均不执行
    if (!WindowLimitsThreshold::LimitsThresholdEnabled()) {
        return false;
    }
    float k = WindowLimitsThreshold::LoadLimitsThresholdConfig().limitsThresholdPercentage / 100.0f;
    if (k <= 0.0f || k >= 1.0f || MathHelper::NearZero(vpr)) {
        return false;
    }

    // 工作区：DMS GetAvailableArea，失败则跳过（保持现行为）
    auto display = SingletonContainer::Get<DisplayManager>().GetDisplayById(
        property_->GetDisplayId());
    DMRect area = { 0, 0, 0, 0 };
    if (display == nullptr ||
        display->GetAvailableArea(area) != DMError::DM_OK ||
        area.width_ == 0 || area.height_ == 0) {
        TLOGW(WmsLogTag::WMS_LAYOUT, "id:%{public}d, get available area failed",
            GetPersistentId());
        return false;
    }
    uint32_t capW = static_cast<uint32_t>(area.width_ * k);
    uint32_t capH = static_cast<uint32_t>(area.height_ * k);

    // 触发判断：漏斗输出最终生效值
    if (newLimits.minWidth_ <= capW && newLimits.minHeight_ <= capH) {
        return false;
    }

    // ratio 来源：set 接口设置值优先，未设置用 limits 内置 ratio 下界
    float R = GetEffectiveAspectRatio(newLimits);

    uint32_t preMinWidth = newLimits.minWidth_;
    uint32_t preMinHeight = newLimits.minHeight_;
    if (MathHelper::NearZero(R)) {
        // 规则 1/2/3：宽高独立封顶
        newLimits.minWidth_ = std::min(newLimits.minWidth_, capW);
        newLimits.minHeight_ = std::min(newLimits.minHeight_, capH);
    } else {
        // 规则 4
        uint32_t minH1 = std::min(newLimits.minHeight_, capH);
        uint32_t minW1 = static_cast<uint32_t>(std::ceil(minH1 * R));
        if (minW1 > capW) {
            // 规则 4.1：宽度封顶后反算高度，无解时 k 上限优先
            newLimits.minWidth_ = capW;
            newLimits.minHeight_ = std::min(
                static_cast<uint32_t>(std::ceil(capW / R)), capH);
        } else {
            // 规则 4.2
            newLimits.minWidth_ = minW1;
            newLimits.minHeight_ = minH1;
        }
    }

    // 防御：封顶只降不升，min <= max 不变式天然保持；失败则回滚原值
    if (!newLimits.IsValid()) {
        TLOGE(WmsLogTag::WMS_LAYOUT, "id:%{public}d, invalid limits after capping, rollback",
            GetPersistentId());
        newLimits.minWidth_ = preMinWidth;
        newLimits.minHeight_ = preMinHeight;
        return false;
    }

    TLOGI(WmsLogTag::WMS_LAYOUT, ...);   // 触发前后值、cap、R 打点
    // VP 侧换算（与漏斗其他步骤模式一致：PX 计算、VP 换算）
    RecalculateVpLimitsByPx(newLimits, newLimitsVP, vpr);
    return true;                          // 封顶实际生效
}

// ratio 两级回退：set 接口设置值 -> limits 内置 minRatio_ 下界
float WindowSceneSessionImpl::GetEffectiveAspectRatio(const WindowLimits& limits) const
{
    float aspectRatio = property_->GetAspectRatio();
    if (!MathHelper::NearZero(aspectRatio)) {
        return aspectRatio;
    }
    return limits.minRatio_;
}
```

返回值约定：`true` 表示封顶实际生效（含值修改 + VP 换算完成），
`false` 表示未生效（门控短路 / 工作区获取失败 / 未触发 / 防御回滚）。
漏斗内将该返回值写入成员标志 `isMinLimitsAdjusted_`（见 3.7），
`UpdateWindowSizeLimits` 本身保持 void 签名不变，对既有调用点与 TDD 零影响。

实现要点：

- **PX 侧计算、VP 侧换算**：工作区为物理像素语义，与漏斗现有
  ProcessPhysicalPixelLimits → RecalculateVpLimitsByPx 模式保持一致；
- **封顶只降 min、不动 max**：min ≤ max 不变式保持；
- **规则 4.2 的语义提示**：minW1 直接覆盖原 minW，可能小于应用原值
  （ratio 耦合的必然结果，符合规则字面"根据计算后的结果决定"），也可能大于
  原值（minH 未超但 R 较大时，ratio 约束的正常耦合）；
- **`isMinLimitsAdjusted_` 为 `std::atomic<bool>` 成员标志**：仅
  `LimitWindowSize` 路径消费（消费前显式清零、用后清除），其他调用点
  置位后不消费也无副作用——后续这些路径均自带 `UpdateProperty` 同步，
  标志残留至下次漏斗重算被覆盖，不产生多余 IPC。

### 3.4 内置 ratio 的取值策略（已按推荐方案实现）

limits 内置 ratio 为区间 [minRatio_, maxRatio_]（来源 module.json5 的
max/minWindowRatio），非固定值。实现采用推荐取法：

- 取 minRatio_（区间下界）反推——最小尺寸落在下界比例时对应最小可行宽度，
  与"允许向下缩放"的目标一致（`GetEffectiveAspectRatio` 回退路径）；
- 若规则 4.1 触发，反算高度时使用同一 R，并保证结果仍落在区间内，否则退回
  k 上限优先。

曾评估的备选方案：区间场景退化为规则 1/2/3 独立封顶（不做反推），ratio
约束完全交给消费端（LayoutController::AdjustRectByAspectRatio /
SessionUtils::AdjustRectByAspectRatio）处理。实现更简单，但最小尺寸点可能
不满足应用设定的 ratio 下界，未采用。

### 3.5 触发后的窗口尺寸回收（复用现有机制，无新增代码）

- 当前窗口 rect 超界时：SetWindowLimits 路径已有 `UpdateNewSize()`（:7431）钳回；
- "显示过满"场景：min 缩小后不强制缩窗（当前 rect 仍 >= min，合法），用户获得
  向下缩放空间，符合"可以缩放到 k 比例"的需求语义；
- 创建路径：首次布局即使用封顶后的值。

### 3.6 与 aspectRatio 校验的关系

`SetAspectRatio` 的合法性校验 `CheckAspectRatioValid`（scene_session.cpp:1840）
基于封顶后的 min 计算可行区间，方向一致，无需改动。

已知缝隙（本功能不引入新问题）：limits 后续独立变化时不重新校验已设置的
ratio。本规则触发时 min 变小、可行区间变宽，只会放宽合法性，不会产生新的
非法状态，维持现状。

### 3.7 服务端同步缝隙（现状问题，随本需求修复）

漏斗各调用路径的同步现状（核实结论）：

| 路径 | 调用点 | 同步情况 |
|---|---|---|
| 窗口创建（主/子/系统窗） | :745 / :1554 | 已有 UpdateProperty |
| SetWindowLimits | :7511 | 已有 UpdateProperty |
| 密度变化 UpdateDensityInner | :7897 | 已有 UpdateProperty（:7930）；PX 单位 else 分支保留原换算语义，插入工作区封顶步骤（封顶生效时补 UpdateNewSize），小艺协同切显示场景生效 |
| 挂靠更新 4 处 | :9095-9198 | 已有 UpdateProperty |
| **resize 惰性重算 LimitWindowSize** | :3172 | **原无同步（缝隙）** |

历史背景：该路径仅在 vpr 变化时重算，输入未变时漏斗输出与已同步值一致，
缝隙无实际后果。本功能使漏斗输出依赖运行时工作区（会变化），且消费方正是
服务端拖拽边界计算（MoveDragController）——若不同步，用户依然缩不下去，
功能在目标场景失效。

修复方式（成员标志方案，替代曾评估的 bool 返回值方案）：不改
`UpdateWindowSizeLimits` 的 void 签名，封顶生效时由漏斗置位成员标志
`isMinLimitsAdjusted_`，`LimitWindowSize` 消费：

```cpp
if (!MathHelper::NearZero(vpr) || !MathHelper::NearZero(property_->GetLastLimitsVpr() - vpr)) {
    isMinLimitsAdjusted_ = false;               // 消费前清零，防历史残留
    UpdateWindowSizeLimits();
    if (isMinLimitsAdjusted_) {
        // Sync the work-area-capped limits to the server side for layout and drag consuming
        UpdateProperty(WSPropertyChangeAction::ACTION_UPDATE_WINDOW_LIMITS);
        isMinLimitsAdjusted_ = false;           // 用后清除
    }
}
```

方案取舍记录：曾实现为 `UpdateWindowSizeLimits` 返回 bool（语义等价），为
避免签名变化影响既有调用点与 TDD，改为成员标志方案。另评估过调用点前后
diff property 值的方案（触发面更宽，可顺带覆盖 vpr 漂移导致的陈旧值）与
漏斗内直接 UpdateProperty 方案（会造成 7 个已同步路径重复 IPC），均未采用。

### 3.8 工作区变化联动（增强项，二期）

一期在创建 / setWindowLimits / resize / 密度变化等漏斗触发点惰性重取工作区。
二期可通过 `RegisterAvailableAreaChangedListener`（display_manager.h:678）
监听工作区变化（分屏、任务栏显隐、旋转），主动触发漏斗重算 + UpdateNewSize。

## 4. 测试计划（单测已实现，系统测试待设备环境执行）

按 docs/Testing.md 规范补充用例。单元测试已实现于
`wm/test/unittest/layout/window_scene_session_impl_layout_test.cpp`
（目标 `wm_layout_test`，13 个用例），配套在
`test/common/mock/mock_display_manager_adapter.h` 补充 2 参数
`GetDisplayInfo` 重载的 MOCK_METHOD（漏斗实际调用路径）。

Mock 方案：`SingletonMocker<DisplayManagerAdapter, MockDisplayManagerAdapter>`
接管 DMS 单例——`GetDisplayInfo` 返回 2000×1000 / vpr 2.0 的 DisplayInfo，
`GetAvailableArea` 返回 2000×1000 工作区（k=0.81 → capW=1620 / capH=810）；
单测编译带 `-Dprivate=public -Dprotected=public`（testcase_flags），可直接
访问 windowSystemConfig_ / property_ / isMinLimitsAdjusted_ 等成员。

### 4.1 单元测试（已实现）

挂 `wm/test/unittest/layout/window_scene_session_impl_layout_test.cpp`，
Mock `Display::GetAvailableArea`：

| 用例 | 覆盖点 |
|---|---|
| AdjustMinLimitsByWorkArea01 | 开关关闭：前置短路，值不变且 GetAvailableArea/GetDisplayInfo 零调用（Times(0) 验证） |
| AdjustMinLimitsByWorkArea02 | percentage 非法（0 / 150，即 k<=0 或 k>=1）：同上短路，零调用 |
| AdjustMinLimitsByWorkArea03 | GetAvailableArea 失败：跳过，保持现行为 |
| AdjustMinLimitsByWorkArea04 | minW、minH 均 <= cap：不触发 |
| AdjustMinLimitsByWorkArea05 | 双超（无 ratio）：规则 1/2/3 独立封顶 + PX/VP 双份一致性断言 |
| AdjustMinLimitsByWorkArea06 | 仅 minW 超：规则 2 |
| AdjustMinLimitsByWorkArea07 | 仅 minH 超：规则 3 |
| AdjustMinLimitsByWorkArea08 | aspectRatio=1.5 + minH 超，反推 minW=1215 <= capW：规则 4/4.2 |
| AdjustMinLimitsByWorkArea09 | aspectRatio=2.5，反推 minW=2025 > capW：规则 4.1（minW=1620，minH 反算=648） |
| AdjustMinLimitsByWorkArea10 | aspectRatio 未设置，回退内置 minRatio_=0.9 反推 + VP 一致性 |
| GetEffectiveAspectRatio01 | ratio 两级回退：set 接口值优先，未设置取 limits.minRatio_ |
| UpdateWindowSizeLimits_WorkArea01 | 漏斗集成：userLimits 超大 min（1800×900）经全漏斗后封顶为 1620×810，VP 同步 810×405，isMinLimitsAdjusted_ 置位 |
| UpdateWindowSizeLimits_WorkArea02 | 漏斗集成：开关关闭，输出与主线一致（1800×900），标志不置位 |
| UpdateDensityInner_PXAttach01 | PX 单位 + 挂靠交集 + 开关关闭：else 分支原换算语义回归（值原样保留，仅交集与 VP 刷新） |
| UpdateDensityInner_PXWorkArea01 | PX 单位 + 开关开启 + 超大 min（小艺协同场景）：密度变化触发封顶 1620×810、VP 视图 810×405、标志置位 |

测试注入方式：`WindowLimitsThreshold::SaveLimitsThresholdConfig({enable, percentage})`
更新静态缓存（同时写 system parameter，测试环境无副作用）；每个用例显式
Save 目标配置，避免跨用例缓存污染。Mock 方案不变（DisplayMocker 提供
displayInfo 与 workArea）。

说明：规则 4.1 中 `min(ceil(capW/R), capH)` 的 capH 钳制分支经数学推导不可达
（触发 4.1 需 R > capW/minH1 >= capW/capH，与 ceil(capW/R) > capH 即
R < capW/capH 矛盾），属纯防御代码，未单测覆盖；全窗口类型覆盖由
AdjustMinLimitsByWorkArea 本身不区分窗口类型保证（集成用例以主窗口为代表）。

### 4.2 系统测试

挂 `test/systemtest/wms/`：

1. 应用设置超大 min 后创建窗口，验证窗口可向下缩放至 capW/capH；
2. 应用 setWindowLimits 动态设置超大 min，验证已显示窗口获得缩放空间；
3. 设置 aspectRatio + 超大 min 的组合场景，验证缩放过程 ratio 保持；
4. 分辨率 / 密度变化后 cap 值随工作区更新；
5. 开关关闭设备回归：行为与主线完全一致。

## 5. 实现落点

| 改动 | 文件 | 内容 |
|---|---|---|
| 配置载体（新增） | window_scene/common/include/window_limits_threshold.h / src/window_limits_threshold.cpp | WindowLimitsThresholdConfig 结构 + WindowLimitsThreshold 静态类：SaveLimitsThresholdConfig（写 parameter + 缓存）/ LoadLimitsThresholdConfig / LimitsThresholdEnabled；随 window_scene_common 编译（BUILD.gn 已加 source） |
| xml 节点注册 | window_scene/session_manager/src/window_scene_config.cpp | configItemTypeMap_ 注册 windowLimitsThreshold(MAP) / limitsThresholdEnabled(MAP) / limitsThresholdPercentage(INTS) |
| 服务端解析 | window_scene/session_manager/src/scene_session_manager.cpp | ConfigWindowLayout → ConfigWindowLimitsThreshold → ConfigWindowLimitsPercentage → ParseLimitsThresholdPercentageConfig → SaveLimitsThresholdConfig |
| 漏斗接入 | wm/src/window_scene_session_impl.cpp | AdjustMinLimitsByWorkArea（门控/比例读 WindowLimitsThreshold）+ GetEffectiveAspectRatio，插入 CalculateNewLimitsByRatio 之后、系统窗 1px 补丁之前；封顶生效时置位成员标志 isMinLimitsAdjusted_（UpdateWindowSizeLimits 保持 void 签名不变） |
| 密度变化接入 | wm/src/window_scene_session_impl.cpp | UpdateDensityInner else 分支（PX 单位）在交集后插入 AdjustMinLimitsByWorkArea，封顶生效时补 UpdateNewSize（小艺协同场景）；VP 分支经漏斗天然覆盖 |
| 同步缝隙 | wm/src/window_scene_session_impl.cpp | LimitWindowSize 路径读取标志、补 UpdateProperty(ACTION_UPDATE_WINDOW_LIMITS) 后清除；经核实其余路径（创建 / SetWindowLimits / UpdateDensityInner / 挂靠）已有同步，无需修改 |

配置文件示例（window_manager_config.xml，挂在 windowLayout 下）：

```xml
<windowLayout>
    <windowLimitsThreshold>
        <limitsThresholdEnabled enable="true">
            <limitsThresholdPercentage>81</limitsThresholdPercentage>
        </limitsThresholdEnabled>
    </windowLimitsThreshold>
</windowLayout>
```

实现确认事项：内置 ratio 回退取融合后的 limits.minRatio_ 下界（3.4）；
IsValid 失败回滚（6.1）；ratio 为 0（未设置且无内置约束）时退化为
规则 1/2/3 独立封顶；设备验证命令
`hdc shell param get persist.windowlayout.windowLimitsThreshold.enable`。

### 5.1 现有 TDD 影响评估

直接调用 `UpdateWindowSizeLimits` 的既有用例 3 个
（UpdateWindowSizeLimits_Test_By_WinType / UpdateWindowSizeLimits01 / 02，
wm/test/unittest/layout/window_scene_session_impl_layout_test.cpp）：

- 签名层面：保持 void 签名不变，用例调用方式无需任何修改；
- 行为层面：用例未注入 WindowLimitsThreshold 配置（parameter 默认 false），
  `AdjustMinLimitsByWorkArea` 前置门控短路返回，`GetAvailableArea` 零调用，
  漏斗输出与改动前逐字节一致，既有断言全部维持；
- 间接路径（rotation 用例经 CheckAndModifyWindowRect → LimitWindowSize）：
  门控关闭时标志恒 false，不发 UpdateProperty，行为与主线一致。

## 6. 风险与开放问题

### 6.1 封顶后 IsValid 校验失败的防御策略（已实现回滚）

封顶只降 min 不动 max，理论上不会破坏 min <= max。若因极端取整场景触发
IsValid 失败，采用回滚策略：恢复封顶前的 min 值并打 TLOGE 错误日志，
宁可不生效也不产出非法 limits。

### 6.2 遗留待确认项

1. ~~内置 ratio 为区间时的 R 取法~~（已定：minRatio_ 下界，见 3.4）；
2. 分离架构（window_manager_use_sceneboard = false，wmserver 平行实现，
   window_layout_policy.cpp:497）是否需要同步该规则——两架构编译互斥，
   若目标设备为统一架构可不改，需产品确认；
3. 工作区变化联动（二期，见 3.8）：当前为惰性重取，"工作区变化但窗口无
   任何触发漏斗的动作"场景下 cap 不更新，待二期 listener 补齐。

### 6.3 兼容性

- 开关默认关闭，其他产品零影响；
- 功能开启后，应用通过 getWindowLimits 查询到的 min 值即为封顶后实际生效值，
  与真实行为一致，无感知差异；
- setWindowLimits 返回值经 FillWindowLimits（:7433）回填实际生效值，应用可
  感知封顶结果，符合现有 API 语义。

### 6.4 部署经验记录（初版方案踩坑，现方案已规避）

初版将配置挂在 SystemSessionConfig（parcel 序列化随 Connect IPC 下发）实测
触发 sceneboard cppcrash 无法开机：SystemSessionConfig 的 Marshalling 为头文件
内联代码，libwm / libscene_session* / libscenesessionmanager_napi 等多个 so
各持一份副本；设备上部分 so 更新、部分未更新时，新代码按新内存布局读旧对象
（崩溃栈：GetFreeMultiWindowConfig 内拷贝 SystemSessionConfig 时 std::set
字段偏移错位）→ SIGSEGV。将字段移至结构体末尾只能缓解单侧越界，无法根治
parcel 布局不匹配。

教训与现方案收益：

1. **凡含内联序列化的跨 so 结构体，字段只能末尾追加，且所有引用 so 必须同批
   替换**（见 3.2 方案决策记录）；
2. 配置类数据优先走 system parameter 而非自定义 parcel——天然免疫混布、
   免编译期布局耦合、可用 param get 直接验证落盘；
3. 部署排查顺序：先 `sha256sum` 比对全部相关 so（含 napi kit），再查代码。
