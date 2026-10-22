# 系统虚拟导航栏无法使用

### 基本信息
- 来源问题单：2025092559823
- 问题域：窗口管理
- 适用仓/模块：window_window_manager / `wm`（WindowSessionImpl）
- 适用版本/分支：OpenHarmony 6.0 master

### 一、问题现象
悬浮三键导航窗口（`WINDOW_TYPE_FLOAT_NAVIGATION`，由 systemui 创建）调用 `show()` 后，`windowSizeChange` 监听不触发，导航栏 ArkUI 侧拿不到尺寸变化事件、无法完成布局刷新，表现为系统虚拟导航栏不可用（按钮不显示/布局异常）。

### 二、根因摘要
`Show()` 路径中 `PreLayoutOnShow` 会在客户端本地预置矩形：`property_->SetWindowRect(requestRect)` 并同步 `UpdateClientRect` 到服务端。之后服务端矩形回调到达时，`UpdateRectForOtherReasonTask` 的去重条件 `(wmRect != preRect) || (wmReason != lastSizeChangeReason_) || !postTaskDone_` 三项全部为 false（矩形相同、reason 未变、已 post 过任务），导致 `NotifySizeChange` 被跳过——客户端"提前把 rect 抹平"使服务端的回调被去重逻辑吞掉。

### 三、code_map
- 关键文件：
  - wm/src/window_session_impl.cpp（核心：`SetNotifySizeChangeFlag`、`UpdateRectForOtherReasonTask`、`NotifySizeChange`）
  - wm/src/window_scene_session_impl.cpp（`PreLayoutOnShow`，wm/src/window_scene_session_impl.cpp:2197）
  - wm/include/window_session_impl.h（`notifySizeChangeFlag_` 原子成员声明）
  - wm/test/unittest/window_session_impl_test3.cpp / test5.cpp（回归用例）
- 文件职责：
  - window_session_impl.cpp：客户端窗口会话基类，负责服务端 rect 回调的分发与 sizeChange 通知去重
  - window_scene_session_impl.cpp：sceneboard 架构下的窗口实现，负责 Show 时序（预布局）
- 高风险入口：`WindowSessionImpl::UpdateRectForOtherReasonTask`（矩形通知去重判定）、`WindowSceneSessionImpl::PreLayoutOnShow`

### 四、routing
- 正确主链路：
  1. 客户端：`Show()` → `PreLayoutOnShow(type)` → `SetNotifySizeChangeFlag(true)` → `property_->SetWindowRect(requestRect)` → `hostSession->UpdateClientRect(wsRect)`
  2. 服务端：布局后经 SessionProxy 回传矩形 → `UpdateRectForOtherReason`（handler_ 投递任务）→ `UpdateRectForOtherReasonTask`
  3. 通知：`notifySizeChangeFlag_ == true` 穿透去重条件 → `NotifySizeChange`（触发 `IWindowSizeChangeListener::OnSizeChange`）→ `SetNotifySizeChangeFlag(false)` 复位 → `UpdateViewportConfig` 刷新 UIContent
- 参数透传要求：
  - `SetNotifySizeChangeFlag(true)` 必须在 `SetWindowRect(requestRect)` **之前**调用（否则 `GetRect() == GetRequestRect()` 恒成立，flag设不上，源码 wm/src/window_scene_session_impl.cpp:2225 有注释强调）
  - flag 为一次性标志：`NotifySizeChange` 后必须立即复位，避免后续回调全部穿透

### 五、expert
- 禁改区域：
  - `UpdateRectForOtherReasonTask` 的去重机制本身（防 rect 抖动重复通知）不可删除
  - `NotifySizeChange` 不得改为无条件调用
- 架构约束：
  - 通知逻辑必须留在客户端 `WindowSessionImpl`，不得下沉到服务端 sceneboard
  - `notifySizeChangeFlag_` 必须为 `std::atomic_bool`（IPC 线程与 handler_ 任务线程并发访问）
- 常见错误改法：
  1. 删除去重 `if`，所有窗口每次回调都通知 → 应用侧重复布局、性能劣化
  2. 在 `PreLayoutOnShow` 里直接调 `NotifySizeChange` → 时序错误（viewport 未更新）且绕过任务队列
  3. 把 `SetNotifySizeChangeFlag(true)` 放在 `SetWindowRect` 之后 → flag 判空直接 return，修复失效
  4. 忘记复位 flag → 退化成错误改法 1

### 六、verify
- 最小编译命令：`./build.sh --product-name rk3568 --build-target window_window_manager`
- 最小测试命令：编译并运行单测目标 `wm_window_session_impl_test3`、`wm_window_session_impl_test5`（属于 `wm:test` 套件），关键用例：`UpdateRectForOtherReasonTask`（test3）、`NotifySizeChangeFlag`（test5）
- 验证通过标准：
  - 单测：`notifySizeChangeFlag_` 置位/复位断言全部通过（非 FLOAT_NAVIGATION 窗口、rect 相同场景 flag 不置位）
  - 真机：拉起悬浮三键导航栏，`on('windowSizeChange')` 在 show 后有回调，导航栏正常显示可点击
- 关键观测点：
  - HiLog tag `WMS_LAYOUT`：`Set notify size change flag is true`；`UpdateRectForOtherReasonTask id:xx rectChanged:0 ... notifyFlag:1`
  - `hidumper -s WindowManagerService -a "-a"` 查看导航栏窗口 rect/可见性

### 七、关联提交
- Commit：`bc537271ac`

### 八、不适用场景
- 手势导航模式（不存在悬浮导航窗口）
- `WINDOW_TYPE_INPUT_METHOD_FLOAT` 输入法窗口（`PreLayoutOnShow` 中显式走键盘预布局分支，不设置该 flag）
- 应用主窗/子窗常规尺寸变化（`wmRect != preRect` 本来就会通知）
- 服务端主动改布局导致的尺寸变化（回调时矩形不同，天然穿透去重）
- 分离架构 `wmserver`（旧架构客户端不走此链路）
