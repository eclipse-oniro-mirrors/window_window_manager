# CodeStyle

## Formatting

Use the project formatter.

Key rules:
- **Column limit**: 120 characters
- **Indent width**: 4 spaces (no tabs)
- **Braces**: Custom — opening brace on same line for classes/structs/control, new line for functions
- **Pointer alignment**: Left (`int* ptr`)
- **Include sorting**: Enabled; grouped by type
- **C++ standard**: C++11

Run formatter tool before committing.

## Code Style Guidelines

### Naming

| Element | Convention | Example |
|---------|-----------|---------|
| Classes / Structs | PascalCase | `WindowManagerService` |
| Methods | camelCase | `GetWindowById()` |
| Member variables | camelCase + trailing underscore | `windowManagerMap_` |
| Constants / Enums | UPPER_SNAKE_CASE | `DEFAULT_SPACING` |
| Namespaces | PascalCase | `OHOS::Rosen` |
| File names | snake_case | `window_manager.cpp` |
| Test classes | PascalCase + `Test` suffix | `WindowManagerServiceTest` |
| Macros | UPPER_SNAKE_CASE | `WMSERVER_LOG_TAG` |

### Include Order

1. Corresponding header (for `.cpp` files)
2. Standard library headers (e.g. `<algorithm>`, `<string>`)
3. OpenHarmony framework headers (e.g. `<hilog.h>`, `<ipc_skeleton.h>`)
4. Project headers from `interfaces/` (e.g. `"wm_common.h"`)
5. Other project-internal headers

Use `#pragma once` or include guards (`#ifndef OHOS_ROSEN_*_H`) in headers.


### Logging

Use `HiLog` wrapper macros:

```cpp
TLOGD("message %d", value);  // Debug
TLOGI("message");             // Info
TLOGW("warning");             // Warn
TLOGE("error: %s", err);     // Error
```

### Error Handling

- Return `WMError` or `WmErrorCode` enum values from public APIs.
- Use early-return pattern for error cases.
- Log errors with `WLOGFE` before returning.
- Use `EXPECT_*` for non-fatal test checks; `ASSERT_*` for fatal.

### Memory & Resource Management

- Use `sptr<T>` for IPC objects and singletons.
- Use `wptr<T>` for weak references where cycles may occur.
- Prefer RAII; avoid raw `new/delete`.

### License Header

Every source file must begin with the Apache 2.0 license header:

```cpp
/*
 * Copyright (c) xxxx-xxxx Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * ...
 */
```

## Key Conventions

- Namespace: always `OHOS::Rosen`
- Feature flags controlled via `declare_args()` in `windowmanager_aafwk.gni`
- ZIDL (IPC proxy/stub) code lives in `src/zidl/` subdirectories
- Use `constexpr` for compile-time constants
