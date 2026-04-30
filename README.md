# window_manager

- [Introduction](#1-introduction)
- [Architecture Overview](#2-architecture-overview)
- [Separated vs. Unified Architecture](#3-separated-vs-unified-architecture)
- [Sub-module Architecture Details](#4-sub-module-architecture-details)
- [Development Guide](#5-development-guide)
- [Directory Structure](#6-directory-structure)
- [Constraints](#7-constraints)
- [Available APIs](#8-available-apis)
- [Repositories Involved](#9-repositories-involved)

## 1. Introduction

### 1.1 Window Subsystem Overview
The Window Manager subsystem provides core capabilities for window and display management in OpenHarmony. It is the foundational subsystem for UI display, responsible for coordinating and managing the creation, destruction, layout, rendering, and interaction of all windows in the system.

### 1.2 Core Capabilities
#### Screen Management
- **Display-Screen Mapping**: Manages the mapping relationship between logical Displays and physical Screens
- **Display Management**: Multi-display management and information query
- **Screen Control**: Screen on/off control and brightness adjustment
- **Screenshot**: Full-screen screenshot capability

#### Window Management
- **Window Lifecycle Management**: Window creation, show, hide, and destruction
- **Window Relationships and Structure**: Parent-child window relationship management with support for window nesting
- **Window Layout Management**: Window position, size, and Z-order control
- **Window Interaction**: Window drag, resize, and move operations
- **Window Snapshot**: Window content screenshot capability
- **Focus Management**: Window focus switching and input event dispatching
- **Multimodal Input Support**: Provides window layout and focus window information for the multimodal input system

### 1.3 Parts and Relationships
![Parts and Relationships](./docs/figures/window_parts_and_relationships.png)

The window subsystem consists of 3 parts:
1. `window_manager`: The current part, hosting the window management service and the application-layer window framework.
	1. It is the core foundation of the entire window subsystem.
2. `scene_board_core`: Hosts the intermediate-layer framework between desktop-related system applications / system UI and the window management service.
	1. It serves as the middle layer between `window_manager` and `scene_board`.
3. `scene_board`: Hosts the desktop-related system applications and system UI implementations, such as the launcher, wallpaper, lock screen, status bar, navigation bar, and control center.
	1. It is the topmost module of the window subsystem and the entry point for users to interact with the system UI.

The window subsystem is primarily associated with the following subsystems:
- **Applications**: Applications can manage windows and screens through the window or display related APIs
- **Multimodal Input Subsystem**: The multimodal input system relies on the window subsystem for event dispatching
- **Graphics Rendering Subsystem**: The window subsystem works in coordination with the graphics rendering system to accomplish window management and UI rendering

In addition, the window subsystem also serves the following consumers:
- **System Applications**: Provides system window and application window management capabilities for system-level UI applications such as the launcher and wallpaper
- **UI Framework**: The `ArkUI` framework implements UI rendering through windows

## 2. Architecture Overview

### 2.1 Overall Architecture

The window subsystem adopts a `Client-Server` architecture, achieving client-server separation through IPC (Inter-Process Communication).
The overall architecture is shown below:

![Window Subsystem Overall Architecture](./docs/figures/WindowManager.png)

### 2.2 Architecture Design Principles
Layered design:
- **Interface Layer**:
	- Provides Native API and JS/NAPI interfaces for application use
- **Client Layer**:
	- Window Manager Client and Display Manager Client, responsible for interface encapsulation, application framework implementation, and IPC communication
- **Server Layer**:
	- WindowManagerService and DisplayManagerService, acting as system services (ServiceAbility) to provide core business logic for window management and screen management
    - SceneSessionManager and ScreenSessionManager are the core business implementation modules for window management and screen management at the system service layer

Collaborative relationships under the layered design:
- **Application Window Creation Flow**
    ```
    Application → window API → Window Manager Client → IPC → WindowManagerService
    ```

- **Display Information Query Flow**
    ```
    Application → display API → Display Manager Client → IPC → DisplayManagerService
    ```

### 2.3 Dual Architectures
The window subsystem currently supports two base architectures: the Separated Architecture and the Unified Architecture. They can be switched via compile-time feature configuration.
- **Global feature configuration key**: `window_manager_use_sceneboard`
- **Configuration file**: `product/define.gni` or the system feature configuration file
- **Configuration method**:
    ```gni
    # Select separated architecture
    window_manager_use_sceneboard = false

    # Select unified architecture
    window_manager_use_sceneboard = true
    ```
- **Scope of impact**:
	- Separated Architecture: compiles the `window_manager/wmserver` module
	- Unified Architecture: compiles the `window_manager/window_scene` module, `scene_board_core`, and `scene_board`

#### Architecture Differences
![Window Subsystem Dual Architecture Comparison](./docs/figures/WindowManager-Architectures.png)

Both architectures expose exactly the same API interfaces to the outside. There is no perceptible difference at the application layer; the differences are mainly in internal implementation and process model.

## 3. Separated vs. Unified Architecture

### 3.1 Separated Architecture

#### 3.1.1 Architecture Characteristics

The Separated Architecture is the traditional window management implementation approach, with the following characteristics:
- **Independent Process Model**: Desktop, wallpaper, and other system applications run as independent processes
- **Traditional IPC Communication**: Application startup/shutdown involves multiple IPC communications

#### 3.1.2 Process Model
![window_process_model](./docs/figures/window_process_model.png)

#### 3.1.3 Startup Flow

![Separated Architecture Startup Flow](./docs/figures/WindowManager-StartSteps-Separated.png)

**Application Startup Steps**:
1. Tap the icon to launch the application.
2. Via IPC, other system services create the process; the window service creates a splash window and loads the splash screen.
3. Through multiple IPC calls, the application and system services establish connections and schedule different lifecycle callbacks (e.g., `onCreate`, `onForeground`).
4. The application creates an application window during startup and loads the application UI.
5. Throughout the process, the launcher controls the startup animation of the application window via IPC through the window management service.

#### 3.1.4 Pros and Cons

**Advantages**:
- Good process isolation; a system application crash does not affect the window service
- Clear architecture with well-defined separation of responsibilities
- Suitable for traditional desktop systems

**Disadvantages**:
- High IPC communication overhead
- Complex startup flow involving multiple IPC calls
- Cross-process window management is complex

### 3.2 Unified Architecture

#### 3.2.1 Architecture Characteristics

The Unified Architecture is the new window management implementation approach, with the following characteristics:
- **Unified Process**: Desktop, wallpaper, and other system applications are merged into the same process as the window service
- **Component-based Management**: System applications are transformed into system window components
- **Layout-driven**: Window layout management is driven by the ArkUI layout pipeline

#### 3.2.2 Process Model
![window_process_model_unified](./docs/figures/window_process_model_unified.png)

#### 3.2.3 Startup Flow

![Unified Architecture Startup Flow](./docs/figures/WindowManager-StartSteps-Unified.png)

**Startup Steps**:
1. Tap the icon to launch the application.
2. The window management service creates the window first and loads the splash screen.
3. The window management service notifies the Ability Manager Service to start the application and schedule different lifecycle callbacks (e.g., `onCreate`, `onForeground`).
4. After the application starts, it loads the UI and connects to the window management service to replace the splash screen.
5. Throughout the process, the launcher and the window management service are in the same process and directly control the startup animation of the application window.

#### 3.2.4 Core Components

**WindowScene Component**:
- Responsible for component-based window management
- Implements layout management for window components
- Provides window lifecycle control

**Screen Component**:
- Responsible for component-based screen management
- Manages the mapping between physical screens and logical Displays
- Provides screen control capabilities

#### 3.2.5 Pros and Cons

**Advantages**:
- Fewer IPC communications, better performance
- Simplified startup flow, faster launch speed
- Leverages the ArkUI layout pipeline for more flexible layout management
- Suitable for mobile and embedded systems

**Disadvantages**:
- High process coupling
- A system application crash may affect the window service
- Increased debugging complexity

## 4. Sub-module Architecture Details

### 4.1 Window Manager Client (wm)

#### 4.1.2 Module Responsibilities

1. **Window Object Abstraction**: Provides the Window class, encapsulating all window operations
2. **Interface Encapsulation**: Wraps lower-level IPC communication into easy-to-use APIs
3. **Lifecycle Management**: Manages the creation and destruction of window objects
4. **Event Callbacks**: Handles window state change events
5. **IPC Communication**: Communicates with the server side via IPC

#### 4.1.3 Collaborative Relationship

```
Application Code
   ↓
Window API (interfaces/kits)
   ↓
Window Manager Client (wm)
   ↓
IPC Communication
   ↓
Window Manager Server
```

### 4.2 Display Manager Client (dm)

#### 4.2.1 Module Structure

```
dm/
├── include/                # Header files
│   ├── display.h           # Display interface definitions
│   └── display_info.h      # Display information structures
└── src/                    # Source files
    ├── display.cpp         # Display implementation
    └── display_manager.cpp # Display manager
```

#### 4.2.2 Module Responsibilities

1. **Display Information Abstraction**: Provides the Display class, encapsulating Display information queries
2. **Interface Encapsulation**: Provides Display management APIs
3. **IPC Communication**: Communicates with Display Manager Server
4. **Event Listening**: Listens for Display change events

#### 4.2.3 Collaborative Relationship

```
Application Code
   ↓
Display API (interfaces/kits)
   ↓
Display Manager Client (dm)
   ↓
IPC Communication
   ↓
Display Manager Server
```

### 4.3 Window Manager Server (wmserver)

#### 4.3.1 Module Structure

```
wmserver/
├── include/              # Header files
│   ├── window_root.h     # Window root node
│   ├── window_node.h     # Window node
│   ├── window_layout.h   # Window layout manager
│   └── ...
└── src/                 # Source files
    ├── window_root.cpp   # Window root node implementation
    ├── window_node.cpp   # Window node implementation
    ├── window_layout.cpp # Window layout implementation
    └── ...
```

#### 4.3.2 Module Responsibilities

1. **Window Tree Management**: Maintains the window tree structure and manages parent-child window relationships
2. **Window Layout**: Calculates window positions and sizes, handles window layout
3. **Z-order Management**: Manages window Z-order and controls window display sequence
4. **Focus Management**: Manages window focus and handles focus switching
5. **Input Dispatching**: Provides focus window information to the input system
6. **Window Dragging**: Handles window drag logic
7. **Window Snapshot**: Provides window screenshot capability

#### 4.3.3 Core Class Descriptions

- **WindowRoot**: The root node of the window tree, managing all top-level windows
- **WindowNode**: A window node representing a window instance
- **WindowLayout**: The window layout manager, responsible for layout calculation
- **FocusController**: The focus controller, managing window focus

#### 4.3.4 Collaborative Relationship

```
IPC Communication
   ↓
Window Manager Service
   ├── WindowRoot (Window Tree)
   ├── WindowLayout (Layout Management)
   ├── FocusController (Focus Management)
   └── ...
   ↓
Graphics System (RenderService)
```

### 4.4 Display Manager Server (dmserver)

#### 4.4.1 Module Structure

```
dmserver/
├── include/              # Header files
│   ├── abstract_display.h       # Abstract Display
│   ├── abstract_screen.h         # Abstract Screen
│   ├── display_controller.h      # Display controller
│   └── ...
└── src/                 # Source files
    ├── abstract_display.cpp     # Abstract Display implementation
    ├── abstract_screen.cpp       # Abstract Screen implementation
    └── ...
```

#### 4.4.2 Module Responsibilities

1. **Display Management**: Manages logical Displays and provides Display information queries
2. **Screen Management**: Manages physical Screens and provides Screen control
3. **Mapping Management**: Maintains the mapping relationship between Displays and Screens
4. **Screen Control**: Controls screen on/off and brightness
5. **Screenshot**: Provides full-screen screenshot capability

#### 4.4.3 Core Class Descriptions

- **AbstractDisplay**: Abstract Display class, representing a logical display
- **AbstractScreen**: Abstract Screen class, representing a physical screen
- **DisplayController**: Display controller, managing Display lifecycle

#### 4.4.4 Collaborative Relationship

```
IPC Communication
   ↓
Display Manager Service
   ├── AbstractDisplay (Logical Display)
   ├── AbstractScreen (Physical Screen)
   └── DisplayController (Controller)
   ↓
Hardware Abstraction Layer (HDI)
```

### 4.5 WindowScene (window_scene)

#### 4.5.1 Module Structure

```
window_scene/
├── include/              # Header files
│   ├── scene_root.h      # Scene root node
│   ├── scene_board.h     # Scene board
│   └── ...
└── src/                 # Source files
    ├── scene_root.cpp    # Scene root node implementation
    └── scene_board.cpp   # Scene board implementation
```

#### 4.5.2 Module Responsibilities

1. **Scene Management**: Manages window scenes, serving as the container for window components
2. **Component-based Management**: Manages windows as `ArkUI` components
3. **Layout Integration**: Integrates the `ArkUI` layout pipeline to enable layout pipeline reuse
4. **System Component Management**: Manages system window components such as the launcher and wallpaper

#### 4.5.4 Collaborative Relationship

```
IPC Communication
   ↓
WindowScene (ArkUI Component)
   ├── RootScene (Scene Root)
   ├── SystemWindowScene - Launcher Component
   ├── SystemWindowScene - Wallpaper Component
   └── WindowScene - Application Window Component
   ↓
ArkUI Layout Pipeline
   ↓
Graphics Rendering System
```

### 4.6 Extension (extension)

#### 4.6.1 Module Structure

```
extension/
├── extension_connection/  # ExtensionAbility component connection part
│   ├── ability_connection.cpp
│   └── ...
└── window_extension/      # ExtensionAbility component window part
    ├── window_extension.cpp
    └── ...
```

#### 4.6.2 Module Responsibilities

1. **Ability Binding**: Implements the binding relationship between an Ability and a window
2. **Lifecycle Synchronization**: Synchronizes the lifecycle of an Ability and its window
3. **Property Propagation**: Passes properties between an Ability and its window

#### 4.6.3 Collaborative Relationship

```
Ability Framework
   ↓
Extension
   ├── ExtensionConnection (Connection Management)
   └── WindowExtension (Window Extension)
   ↓
Window Manager
```

## 5. Development Guide

### 5.1 Window Properties

**Customizable Window Properties**:
```cpp
// Window type
enum class WindowType {
    TYPE_APP,              // Application window
    TYPE_SYSTEM_ALERT,     // System alert window
    TYPE_INPUT_METHOD,     // Input method window
    TYPE_STATUS_BAR,       // Status bar window
    TYPE_PANEL,            // Panel window
    TYPE_FLOAT,            // Floating window
    // ... extensible as needed
};

// Window mode
enum class WindowMode {
    UNDEFINED,
    FULLSCREEN,            // Full-screen mode
    PRIMARY,               // Split-screen primary window
    SECONDARY,             // Split-screen secondary window
    FLOATING,              // Floating mode
};

// Window layout properties
struct WindowLayoutProperty {
    Rect rect;            // Window position and size
    uint32_t zOrder;      // Window Z-order
    WindowMode mode;      // Window mode
    // ... extensible as needed
};
```

**Development Steps**:
1. Extend the `WindowType` enum to add a custom window type
2. Add the corresponding window type handling logic in the Window Manager Server
3. Modify the window layout algorithm to support the new window type

#### Adding a Custom Window Type

**Step 1**: Extend the window type enum
```cpp
// In interfaces/innerkits/native/include/window/window_type.h
enum class WindowType {
    // ... existing types
    TYPE_CUSTOM_WINDOW = 1000,  // Custom window type
};
```

**Step 2**: Add window type handling logic
```cpp
// In wmserver/src/window_type.cpp
bool IsSystemWindow(WindowType type)
{
    // ... existing logic
    if (type == WindowType::TYPE_CUSTOM_WINDOW) {
        return true;
    }
    return false;
}
```

**Step 3**: Handle the new type in the layout algorithm
```cpp
// In wmserver/src/window_layout.cpp
void WindowLayout::CalculateLayout(WindowNode* node)
{
    if (node->GetType() == WindowType::TYPE_CUSTOM_WINDOW) {
        // Custom layout logic
        CalculateCustomWindowLayout(node);
    } else {
        // Default layout logic
        CalculateDefaultLayout(node);
    }
}
```

### 5.2 Window Layout Algorithm

**Customizable Layout Algorithm**:
```cpp
// In window_layout.h
class WindowLayout {
public:
    // Overridable layout calculation functions
    virtual void CalculateLayout(WindowNode* node);
    virtual void CalculateZOrder(std::vector<WindowNode*>& nodes);

protected:
    // Layout strategy
    LayoutStrategy layoutStrategy_;

    // Customization: add a custom layout strategy
    void ApplyCustomLayout(WindowNode* node);
};
```

**Customization Steps**:
1. Inherit from the `WindowLayout` class
2. Override the `CalculateLayout` method to implement a custom layout algorithm
3. Use the custom layout class in the Window Manager Server

### 5.3 Notes

1. **Compatibility**: Maintain compatibility with existing interfaces
2. **Performance**: Business logic must not impact system performance
3. **Stability**: Code must be thoroughly tested to ensure it does not affect system stability
4. **Maintainability**: Code must have good comments and documentation
5. **Version Upgrades**: Compatibility must be considered when upgrading the system

## 6. Directory Structure

```
foundation/window/window_manager/
├── dm                              # Display Manager Client implementation
├── dmserver                        # Display Manager Service implementation
├── extension                       # Ability Component window-related code
│   ├── extension_connection        # Ability Component embedding part
│   └── window_extension            # Ability Component embedded part
├── interfaces                      # External API directory
│   ├── innerkits                   # Native API directory
│   └── kits                        # JS/NAPI API directory
├── previewer                       # Lightweight IDE simulator window implementation
├── resources                       # Framework resource files
├── sa_profile                      # System service configuration files
├── snapshot                        # Screenshot command-line tool implementation
├── test                            # Fuzz tests and system test cases
├── utils                           # Utility classes
├── window_scene                    # Unified Architecture Window Manager Service implementation
├── wm                              # Window Manager Client implementation
└── wmserver                        # Separated Architecture Window Manager Service implementation
```

## 7. Constraints
- Language version
    - C++11 or later

## 8. Available APIs

- [Window](https://gitcode.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-arkui/arkts-apis-window.md)
- [Display](https://gitcode.com/openharmony/docs/blob/master/zh-cn/application-dev/reference/apis-arkui/js-apis-display.md)

## 9. Repositories Involved

- [graphic_graphic_2d](https://gitcode.com/openharmony/graphic_graphic_2d)
- [arkui_ace_engine](https://gitcode.com/openharmony/arkui_ace_engine)
- [ability_ability_runtime](https://gitcode.com/openharmony/ability_ability_runtime)
- [multimodalinput_input](https://gitcode.com/openharmony/multimodalinput_input)
