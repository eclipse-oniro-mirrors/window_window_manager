# CLAUDE.md — Window Manager (OpenHarmony)

This file provides guidance for AI coding agents working in this repository.

## What is this repo

`window_manager` is the window management subsystem of OpenHarmony. It provides core capabilities for window and display management, serving as the foundational subsystem for UI display. See [README.md](./README.md) for full architecture details.

## Architecture

Two compile-time architectures are supported, selected via `window_manager_use_sceneboard` in `product/define.gni`:

| Value | Architecture | Key module compiled |
|-------|-------------|---------------------|
| `false` | Separated | `wmserver/` |
| `true` | Unified | `window_scene/` |

Key modules:
- `wm/` — Window Manager Client
- `dm/` — Display Manager Client
- `wmserver/` — Window Manager Server (separated arch)
- `dmserver/` — Display Manager Server
- `window_scene/` — Window Manager Server (unified arch)
- `interfaces/innerkits/` — Native APIs
- `interfaces/kits/` — JS/NAPI APIs
- `extension/` — Ability Component window integration
- `utils/` — Shared utilities

## Build

This project uses the OpenHarmony `gn` + `ninja` build system.

Feature flags are declared via `declare_args()` in `windowmanager_aafwk.gni`. The global architecture switch is:
```gni
window_manager_use_sceneboard = true   # unified arch
window_manager_use_sceneboard = false  # separated arch
```

## Code Style

Full rules: [docs/CodeStyle.md](./docs/CodeStyle.md)

- **C++ standard**: C++11
- **Column limit**: 120 characters
- **Indent**: 4 spaces, no tabs
- **Braces**: Same line for classes/structs/control flow; new line for functions
- **Pointer alignment**: Left — `int* ptr`
- **Namespace**: always `OHOS::Rosen`

### Naming conventions

| Element | Convention | Example |
|---------|-----------|---------|
| Classes / Structs | PascalCase | `WindowManagerService` |
| Methods | camelCase | `GetWindowById()` |
| Member variables | camelCase + trailing `_` | `windowManagerMap_` |
| Constants / Enums | UPPER_SNAKE_CASE | `DEFAULT_SPACING` |
| Namespaces | PascalCase | `OHOS::Rosen` |
| File names | snake_case | `window_manager.cpp` |
| Test classes | PascalCase + `Test` suffix | `WindowManagerServiceTest` |
| Macros | UPPER_SNAKE_CASE | `WMSERVER_LOG_TAG` |

### Include order (per `.cpp`)

1. Corresponding header
2. Standard library headers (`<algorithm>`, `<string>`, …)
3. OpenHarmony framework headers (`<hilog.h>`, `<ipc_skeleton.h>`, …)
4. Project headers from `interfaces/`
5. Other project-internal headers

Use `#pragma once` or `#ifndef OHOS_ROSEN_*_H` include guards in headers.

### Logging

```cpp
TLOGD("message %d", value);  // Debug
TLOGI("message");             // Info
TLOGW("warning");             // Warn
TLOGE("error: %s", err);     // Error
```

### Error handling

- Return `WMError` / `WmErrorCode` enum values from public APIs.
- Use early-return for error cases; log with `WLOGFE` before returning.

### Memory management

- Use `sptr<T>` for IPC objects and singletons; `wptr<T>` for weak refs.
- Prefer RAII; avoid raw `new`/`delete`.

### License header

Every source file must begin with:
```cpp
/*
 * Copyright (c) xxxx-xxxx Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * ...
 */
```

## Testing

Full rules: [docs/Testing.md](./docs/Testing.md)

- Test files use `*_test.cpp` suffix, named after the class under test.
- Use `HWTEST_F` macro; namespace must be `OHOS::Rosen`.
- `EXPECT_*` for non-fatal checks; `ASSERT_*` for fatal.

### Test structure template

```cpp
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class MyFeatureTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

HWTEST_F(MyFeatureTest, TestSomething, TestSize.Level1)
{
    EXPECT_EQ(result, expected);
    ASSERT_NE(ptr, nullptr);
}
} // namespace Rosen
} // namespace OHOS
```

### Build targets per module

| Module | Build target |
|--------|-------------|
| wm | `wm:test` |
| wmserver | `wmserver:test` |
| dm | `dm:test` |
| dmserver | `dmserver:test` |
| dm_lite | `dm_lite:test` |
| window_scene | `window_scene:test` |
| snapshot | `snapshot:test` |
| extension/window_extension | `extension/window_extension:test` |

## IPC / ZIDL

IPC proxy/stub code lives in `src/zidl/` subdirectories within each module. IDL definitions are at the module root (e.g. `dmserver/IDisplayManager.idl`).
