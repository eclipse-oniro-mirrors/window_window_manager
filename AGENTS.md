# 窗口管理组件指引

## 项目定位

本仓库对应 OpenHarmony `foundation/window/window_manager`，为系统提供窗口生命周期、布局、焦点、输入窗口信息、显示/屏幕管理、截图和多屏协同能力。优先按这些目录定位问题：

- `wm/`、`dm/`、`dm_lite/`：窗口与显示管理客户端、IPC 适配和 Lite 接口实现。
- `wmserver/`、`dmserver/`：分离架构下的窗口管理服务与显示管理服务。
- `window_scene/`：合一架构下的 SceneSession、ScreenSession 及对应服务。
- `interfaces/innerkits/`、`interfaces/kits/`：Native、NAPI、ANI、CJ、NDK 和 JS 声明接口。
- `extension/`：WindowExtension 的连接、生命周期和窗口集成。
- `utils/`：窗口/显示公共数据结构、权限、日志、持久化和图形适配工具。
- `etc/`、`sa_profile/`、`resources/`、`product/`：系统能力、参数、资源和产品配置。
- `test/` 及各模块的 `test/`：单元测试、系统测试和 fuzz 目标。

窗口管理支持两套编译时架构，由 OpenHarmony 构建系统提供的 `window_manager_use_sceneboard` 决定：

| 配置值 | 架构 | 窗口服务实现 | 屏幕服务实现 |
| --- | --- | --- | --- |
| `false` | 分离架构 | `wmserver/` | `dmserver/` |
| `true` | 合一架构 | `window_scene/session/`、`window_scene/session_manager/` | `window_scene/screen_session_manager/` |

两套架构应保持对外 API 和可观察行为一致。修改公共窗口语义时，不要只验证当前启用的一条路径。

### 按任务类型定位代码

| 任务类型 | 首选目录 | 关键文件 |
| --- | --- | --- |
| 修改窗口客户端生命周期/属性 | `wm/` | `window.cpp`, `window_impl.cpp`, `window_session_impl.cpp`, `window_adapter.cpp` |
| 修改窗口管理监听与全局查询 | `wm/` | `window_manager.cpp`, `window_manager_agent.cpp`, `window_adapter.cpp` |
| 修改分离架构窗口服务 | `wmserver/` | `window_manager_service.cpp`, `window_controller.cpp`, `window_root.cpp` |
| 修改分离架构窗口树、布局、层级 | `wmserver/` | `window_node.cpp`, `window_node_container.cpp`, `window_layout_policy*.cpp`, `window_zorder_policy.cpp` |
| 修改合一架构会话生命周期 | `window_scene/session/host/` | `session.cpp`, `scene_session.cpp`, `main_session.cpp`, `sub_session.cpp` |
| 修改合一架构窗口服务、焦点或窗口信息刷新 | `window_scene/session_manager/` | `scene_session_manager.cpp`, `scene_session_dirty_manager.cpp`, `scene_input_manager.cpp` |
| 修改显示客户端 | `dm/`、`dm_lite/` | `display_manager.cpp`, `display_manager_adapter.cpp`, `screen_manager.cpp` |
| 修改分离架构显示服务 | `dmserver/` | `display_manager_service.cpp`, `display_manager_ipc_service.cpp`, `abstract_screen_controller.cpp` |
| 修改合一架构屏幕、旋转、折叠或多屏 | `window_scene/screen_session_manager/`、`window_scene/session/screen/` | `screen_session_manager.cpp`, `screen_session.cpp`, `multi_screen_*`, `fold_screen_controller/` |
| 修改 Native 公共接口或错误码 | `interfaces/innerkits/`、`window_scene/interfaces/` | `window.h`, `window_manager.h`, `wm_common.h`, `display_manager.h`, `dm_common.h`, `ws_common.h` |
| 修改 NAPI/ANI/CJ/NDK 接口 | `interfaces/kits/` | `napi/`, `ani/`, `cj/`, `ndk/`, `dmndk/` 对应模块 |
| 修改 SceneSession/ScreenSession 的系统接口 | `window_scene/interfaces/kits/` | `napi/scene_session_manager/`, `napi/screen_session_manager/`, `ani/scene_session_manager/` |
| 修改 IPC 接口 | 各模块 `include/zidl/`、`src/zidl/` 及模块根目录 IDL | `IDisplayManager.idl`, `IDisplayManagerLite.idl`, 对应 interface/proxy/stub |
| 修改画中画、悬浮球或浮窗 | `wm/` | `picture_in_picture_*`, `floating_ball_*`, `float_view_*`, `float_window_manager.cpp` |
| 修改 Extension 窗口集成 | `extension/` | `window_extension.cpp`, `window_extension_connection.cpp`, 对应 zidl 文件 |
| 修改截图或分辨率工具 | `snapshot/`、`setresolution/` | `snapshot_display.cpp`, `snapshot_utils.cpp`, `setresolution_screen.cpp` |
| 修改公共基础能力 | `utils/` | `window_property.cpp`, `permission.cpp`, `rs_adapter.cpp`, `persistent_storage.cpp` |

### 嵌套指引

本仓库没有目录级别的嵌套 `AGENTS.md`。根目录 `README_zh.md`、`docs/CodeStyle.md` 和 `docs/Testing.md` 是主要补充资料；修改前还应阅读目标目录的 `BUILD.gn` 和相邻测试。

## 构建和验证

构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行。完整部件可按产品构建：

```sh
./build.sh --product-name rk3568 --build-target window_manager --ccache
```

优先构建改动模块及其测试。常用 GN 测试标签如下：

| 模块 | 测试标签 |
| --- | --- |
| 窗口客户端 | `//foundation/window/window_manager/wm:test` |
| 分离架构窗口服务 | `//foundation/window/window_manager/wmserver:test` |
| 显示客户端 | `//foundation/window/window_manager/dm:test` |
| 分离架构显示服务 | `//foundation/window/window_manager/dmserver:test` |
| Lite 显示客户端 | `//foundation/window/window_manager/dm_lite:test` |
| 合一架构窗口/屏幕服务 | `//foundation/window/window_manager/window_scene:test` |
| 截图工具 | `//foundation/window/window_manager/snapshot:test` |
| WindowExtension | `//foundation/window/window_manager/extension/window_extension:test` |
| 公共工具 | `//foundation/window/window_manager/utils:test` |
| fuzz 目标 | `//foundation/window/window_manager/test:fuzztest` |

单个 `ohos_unittest` 目标名以相应 `test/**/BUILD.gn` 为准。测试文件使用 `*_test.cpp`，采用 `HWTEST_F`，并保持在 `OHOS::Rosen` 命名空间。非致命检查使用 `EXPECT_*`，后续代码依赖该条件时使用 `ASSERT_*`。

### 完成标准

任务被认为完成，当且仅当：

1. **改动范围完整** - 客户端、服务端、双架构、绑定层或 IPC 对端中受影响的部分已同步处理
2. **本地构建通过** - 至少构建受影响模块；公共接口或公共数据结构改动需构建所有直接消费者
3. **相关测试通过** - 对应单元测试通过，解析 IPC/外部输入的改动补充或运行 fuzz 测试
4. **板侧验证（如适用）** - 涉及窗口布局、焦点、动画、输入分发、屏幕、折叠、多屏、电源或截图的改动需提供验证证据
5. **文档和接口同步（如适用）** - 公共 API、错误码、配置项或系统能力修改需同步声明、注释和文档
6. **提交规则（仅在用户要求提交时）** - 先获得用户确认，再使用 `git commit -s`；提交信息采用 `type(scope): subject`，并按协作要求添加 `Co-Authored-By: Agent`

### 如果无法运行验证

明确说明无法运行的原因，列出已执行的静态检查，并给出可从 OpenHarmony 源码根目录执行的构建、测试和板侧验证步骤。不要把“当前仓库缺少完整源码树”描述为测试已通过。

### 完成报告格式

报告应包含：改动摘要（文件列表、双架构影响）、验证结果（构建/测试/板侧证据）、风险评估（API/IPC 兼容性、并发、性能和安全）、未完成事项。

## 知识索引

稳定背景知识位于根目录文档、配置和核心接口中。改动前按场景读取对应资料：

### 场景与路径路由

| 场景 | 修改目录 | 先读资料 |
| --- | --- | --- |
| 子系统职责、Client-Server 分层、双架构 | `wm/`, `dm/`, `wmserver/`, `dmserver/`, `window_scene/` | `README_zh.md`, `scene_board_enable.gni` |
| 编译特性、可选依赖、产品差异 | 任意 `BUILD.gn` 或条件编译代码 | `windowmanager_aafwk.gni`, `bundle.json`, 目标目录 `BUILD.gn` |
| 代码格式、命名、日志、错误处理 | 所有 C++ 改动 | `docs/CodeStyle.md`, `.clang-format`, 相邻实现 |
| 单元测试、测试目标和断言风格 | 各模块 `test/` | `docs/Testing.md`, 对应 `test/**/BUILD.gn` |
| 窗口/显示公共 API 与错误码 | `interfaces/innerkits/`, `interfaces/kits/` | `wm_common.h`, `dm_common.h`, 对应 API 头文件和绑定实现 |
| 合一架构 Session/Screen 协议 | `window_scene/` | `window_scene/interfaces/include/ws_common.h`, 对应 `include/zidl/` 与 `src/zidl/` |
| 权限、调用者身份、多用户 | `wmserver/`, `dmserver/`, `window_scene/` | `utils/include/permission.h`, `window_scene/common/include/session_permission.h`, 相邻 IPC 入口 |
| ABI 导出、SA 和运行配置 | 公共库、服务或配置改动 | `*.map`, `sa_profile/*.json`, `etc/`, `bundle.json` |
| 沉浸式窗口 | `docs/knowledge` | `immersive-avoid-area-layout-timing.md`, `immersive-avoid-area.md`, `immersive-debugging-and-tests.md`, `immersive-system-bars.md`, `immersive-window-overview.md` |
| 画中画 | `docs/knowledge` | `floating_ball.md`, `floating_view.md`, `picture_in_picture.md` |

### 开始编辑前

在修改代码前，按以下顺序确认：

1. 确认任务属于窗口还是显示链路，以及客户端、服务端、绑定层和 IPC 对端
2. 确认是否同时影响分离架构与合一架构，并定位两套实现
3. 根据上表阅读资料和相邻测试，确认实际 GN 条件与产品特性
4. 根据“项目约束”确认不违反 API、IPC、权限、线程或设备边界
5. 声明：“我将修改 X，已检查 Y 架构和 Z 接口/测试，遵循对应兼容性约束”

## 项目约束

### 性能约束

- 窗口信息刷新、焦点/输入分发、拖拽、布局、旋转和 VSync 相关逻辑是高频路径，不要在每帧或每次事件中增加全量容器扫描、同步 IPC、磁盘访问、字符串拼接或 INFO 日志。
- 不要在持有窗口树、session map、screen map 或 listener 容器锁时调用不受控的外部回调或跨进程接口；需要跨线程执行时复用附近的 task scheduler、event handler 或 FFRT 队列。
- 保持现有锁顺序和线程归属。新增异步任务时明确对象生命周期，使用 `sptr`/`wptr` 或安全快照，避免捕获悬空裸指针。
- 图形事务、窗口信息和输入区域更新应保持既有批处理与时序，避免把异步路径改为阻塞路径。

### 架构约束

- `window_manager_use_sceneboard` 由外部构建系统设置，不要在仓内硬编码或用局部宏绕过它。
- 对外语义变更必须评估 `wmserver/` 与 `window_scene/` 两条路径；只适用于单一架构的修改应在代码、测试和报告中明确。
- 保持 Client → Adapter/Proxy → Service/SessionManager 的分层，不要让接口绑定层直接依赖服务内部对象。
- Window、Display、Screen、SceneSession 和 ScreenSession 的 ID/坐标空间有各自语义；不要在未转换、未校验显示归属时混用。
- 产品和特性差异通过 `windowmanager_aafwk.gni`、`bundle.json` 与 GN 条件表达，不要在业务代码中复制一套运行时判断替代构建配置。

### 编码约定

- C++ 代码遵循 `.clang-format`：4 空格缩进、120 列、指针左对齐，并保持 `OHOS::Rosen` 命名空间。
- 优先沿用相邻代码的命名；当前 Native C++ 类和函数通常使用 PascalCase，成员变量使用 lowerCamelCase 加尾随 `_`，文件使用 snake_case。
- 复用所在模块的 `TLOG*`、`WLOG*`、`WLOGF*` 和 `WmsLogTag`，不要在高频路径新增无节流日志。
- 窗口接口返回 `WMError`/`WmErrorCode`，显示接口返回 `DMError`/`DmErrorCode`，Session 接口返回 `WSError`/`WSErrorCode`；不要混用错误域或吞掉原始错误。
- IPC 对象使用 `sptr<T>`，弱引用使用 `wptr<T>`；优先 RAII，避免新增裸 `new`/`delete`。

### 公共 API 约束

**Do not（禁止）：**

- 修改已发布 Native、NAPI、ANI、CJ、NDK API 的签名、参数/返回类型或同步异步语义
- 删除、重命名或改变已有 `WindowType`、`WindowMode`、`WMError`、`DMError`、`WSError` 等枚举值的数值
- 在没有兼容性方案时改变已有错误码映射、默认值、事件名称或回调触发顺序
- 从 `libwm.map`、`libdm.map`、`libscene_session_manager.map` 等导出表移除已有符号

**Ask before（修改前必须确认）：**

- 新增公共 API、系统能力或导出符号
- 修改公共结构体布局、可序列化字段或跨语言绑定行为
- 只在某一种语言绑定或某一种架构中暴露新能力

### 安全与权限边界

**Do not（禁止）：**

- 绕过 `Permission`、`SessionPermission`、AccessToken、系统应用或调用者 PID/UID/用户校验
- 信任客户端传入的窗口/屏幕/session ID、token、Surface、文件描述符或 PixelMap 而不做现有校验
- 在日志中输出截图内容、窗口内容、未脱敏的应用身份、token 或其他敏感信息
- 让普通调用者获得系统窗口、截屏、虚拟屏、多用户或跨应用窗口控制能力

**Ask before（修改前必须确认）：**

- 修改系统窗口、截屏、录屏/虚拟屏、跨设备、多用户或输入重定向相关权限
- 修改 IPC 调用者身份切换、token 传递、用户隔离或窗口归属校验
- 新增可读取窗口内容、窗口列表、焦点应用或屏幕状态的接口

### 协议与数据格式兼容性

**Do not（禁止）：**

- 修改已发布 IPC transaction code、接口 token 或 Parcel 字段读写顺序
- 只修改 proxy、stub 或 interface 的一侧
- 改变跨进程结构体字段顺序、类型、默认值或可选字段语义而不提供兼容处理
- 修改现有 SA ID、`sa_profile` 注册关系或系统参数名称而不评估升级兼容性

**Ask before（修改前必须确认）：**

- 新增或修改 IDL/ZIDL 方法、回调接口和序列化字段
- 修改客户端重连、死亡通知、服务恢复或多用户切换协议
- 修改窗口信息同步给 MMI、RenderService、AbilityManager 或 SceneBoard 的数据语义

### 生成代码边界

- 不要直接修改 `out/.../gen` 中由 IDL/GN 生成的文件；修改 `IDisplayManager.idl`、`IDisplayManagerLite.idl` 等源定义并重新生成。
- 仓库内受版本控制的 `include/zidl/`、`src/zidl/` 并不都属于构建产物。修改这些手写 IPC 文件时，必须同步 interface/proxy/stub、transaction code、Parcel 顺序和测试。
- 不要把 mock、fuzz 或 previewer 生成替代物复制进生产接口实现。

### 设备操作约束

- 涉及屏幕电源、亮度、旋转、折叠状态、分辨率、多屏模式、虚拟屏或截图时，不执行会破坏用户会话或设备可用性的操作。
- 真实设备验证需记录产品、架构开关和关键配置，并提供 `hdc`/HiLog/HiDumper 输出、截图或录屏等证据。
- 截图和窗口快照可能包含隐私信息；仅采集完成验证所需的最小内容，并避免写入仓库或日志。
