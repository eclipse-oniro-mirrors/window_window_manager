/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import { AsyncCallback, BusinessError, Callback } from '@ohos.base';
import { LocalStorage } from '@ohos.arkui.stateManagement';
import { UIContext } from '@ohos.arkui.UIContext';
import BaseContext from 'application.BaseContext';
import { ColorMetrics } from '@ohos.arkui.node';
import { ImageFit } from '@ohos.arkui.component';
import bundleManager from '@ohos.bundle.bundleManager';
import image from '@ohos.multimedia.image';
import hilog from '@ohos.hilog';
import rpc from '@ohos.rpc';
import dialogRequest from '@ohos.app.ability.dialogRequest';

export type AsyncCallbackVoid = (err: BusinessError) => void;
export type WindowEventCallback = (data: window.WindowEventType) => void;
export type WindowSizeCallback = (data: window.Size) => void;
export type WindowStatusCallback = (data: window.WindowStatusType) => void;
export type WindowVoidCallback = () => void;
export type KeyboardHeightChangeCallback = (data: int) => void;
export type KeyboardDidShowOrHideCallback = (data: window.KeyboardInfo) => void;
export type KeyboardWillShowOrHideCallback = (data: window.KeyboardInfo) => void;
export type RotationChangeCallback<T, U> = (info: T) => U;
export type WindowStageCloseCallback<T, V = void> = (data: T) => V;
export type WindowWillCloseCallback<T, V = void> = (data: T) => V;
const DOMAIN = 0x4200;
const TAG = '[ANI]';
const WM_ERROR_INVALID_PARAM = 401;
const WM_ERROR_ILLEGAL_PARAM = 1300016;
declare type WindowAnimationCurveParam = Array<number>;

namespace window {
  export interface Size {
    /**
     * The width of the window.
     *
     * @type { int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * The width of the window.
     *
     * @type { int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @since 10
     */
    /**
     * The width of the window.
     *
     * @type { int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    width: int;

    /**
     * The height of the window.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * The height of the window.
     *
     * @type { int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @since 10
     */
    /**
     * The height of the window.
     *
     * @type { int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    height: int;
  }
  export enum AvoidAreaType {
    /**
     * Default area of the system
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * Default area of the system
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 11
     */
    /**
     * Default area of the system
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    TYPE_SYSTEM,

    /**
     * Notch
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * Notch
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 11
     */
    /**
     * Notch
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    TYPE_CUTOUT,

    /**
     * Area for system gesture
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 9
     */
    /**
     * Area for system gesture
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 11
     */
    /**
     * Area for system gesture
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    TYPE_SYSTEM_GESTURE,

    /**
     * Area for keyboard
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 9
     */
    /**
     * Area for keyboard
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 11
     */
    /**
     * Area for keyboard
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    TYPE_KEYBOARD,

    /**
     * Area for navigation indicator
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 11
     */
    /**
     * Area for navigation indicator
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    TYPE_NAVIGATION_INDICATOR
  }
  export interface AvoidArea {
    /**
     * Whether avoidArea is visible on screen
     *
     * @type { boolean }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 9
     */
    /**
     * Whether avoidArea is visible on screen
     *
     * @type { boolean }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 11
     */
    visible: boolean;

    /**
     * Rectangle on the left of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * Rectangle on the left of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 11
     */
    /**
     * Rectangle on the left of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    leftRect: Rect;

    /**
     * Rectangle on the top of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * Rectangle on the top of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 11
     */
    /**
     * Rectangle on the top of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    topRect: Rect;

    /**
     * Rectangle on the right of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * Rectangle on the right of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 11
     */
    /**
     * Rectangle on the right of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    rightRect: Rect;

    /**
     * Rectangle on the bottom of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * Rectangle on the bottom of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 11
     */
    /**
     * Rectangle on the bottom of the screen
     *
     * @type { Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 12
     */
    bottomRect: Rect;
  }

  export class AvoidAreaInternal implements AvoidArea {
    visible: boolean;
    leftRect: Rect;
    topRect: Rect;
    rightRect: Rect;
    bottomRect: Rect;
  }

  /**
   * System bar tint of region
   *
   * @interface SystemBarRegionTint
   * @syscap SystemCapability.WindowManager.WindowManager.Core
   * @systemapi Hide this for inner system use.
   * @since 8
   */
  export interface SystemBarRegionTint {
    /**
     * System bar type
     *
     * @type { WindowType }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @systemapi Hide this for inner system use.
     * @since 8
     */
    type: number;

    /**
     * The visibility of system bar
     *
     * @type { ?boolean }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @systemapi Hide this for inner system use.
     * @since 8
     */
    isEnable: boolean;

    /**
     * The region of system bar
     *
     * @type { ?Rect }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @systemapi Hide this for inner system use.
     * @since 8
     */
    region: Rect;

    /**
     * The background color of the system bar.
     *
     * @type { ?string }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @systemapi Hide this for inner system use.
     * @since 8
     */
    backgroundColor: string;

    /**
     * The content color of the system bar.
     *
     * @type { ?string }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @systemapi Hide this for inner system use.
     * @since 8
     */
    contentColor: string;
  }

  /**
   * System bar tint state for systemui
   *
   * @interface SystemBarTintState
   * @syscap SystemCapability.WindowManager.WindowManager.Core
   * @systemapi Hide this for inner system use.
   * @since 8
   */
  export interface SystemBarTintState {
    /**
     * Id of display
     *
     * @type { long }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @systemapi Hide this for inner system use.
     * @since 8
     */
    displayId: long;
    /**
     * Region tint of systembar
     *
     * @type { Array<SystemBarRegionTint> }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @systemapi Hide this for inner system use.
     * @since 8
     */
    regionTint: Array<SystemBarRegionTint>;
  }

  export class SystemBarTintStateInternal implements SystemBarTintState {
    displayId: long;
    regionTint: Array<SystemBarRegionTint>;
  }

  export class SystemBarRegionTintInternal implements SystemBarRegionTint {
    type: number;
    isEnable: boolean;
    region: Rect;
    backgroundColor: string;
    contentColor: string;
  }

  export enum RectChangeReason {
    UNDEFINED = 0,
    MAXIMIZE = 1,
    RECOVER = 2,
    MOVE = 3,
    DRAG = 4,
    DRAG_START = 5,
    DRAG_END = 6,
  }

  export enum WindowAnchor {
    /**
     * The value means window top left corner.
     *
     * @syscap SystemCapability.Window.SessionManager
     * @since 20
     */
    TOP_START = 0,
    /**
     * The value means horizontal midpoint of the border on the window.
     *
     * @syscap SystemCapability.Window.SessionManager
     * @since 20
     */
    TOP = 1,
    /**
     * The value means window top right corner.
     *
     * @syscap SystemCapability.Window.SessionManager
     * @since 20
     */
    TOP_END = 2,
    /**
     * The value means vertical midpoint of the left border of the window.
     *
     * @syscap SystemCapability.Window.SessionManager
     * @since 20
     */
    START = 3,
    /**
     * The value means window horizontal and vertical midpoint.
     *
     * @syscap SystemCapability.Window.SessionManager
     * @since 20
     */
    CENTER = 4,
    /**
     * The value means vertical midpoint of the right border of the window.
     *
     * @syscap SystemCapability.Window.SessionManager
     * @since 20
     */
    END = 5,
    /**
     * The value means window bottom left corner.
     *
     * @syscap SystemCapability.Window.SessionManager
     * @since 20
     */
    BOTTOM_START = 6,
    /**
     * The value means horizontal midpoint of the lower border of the window.
     *
     * @syscap SystemCapability.Window.SessionManager
     * @since 20
     */
    BOTTOM = 7,
    /**
     * The value means window bottom right corner.
     *
     * @syscap SystemCapability.Window.SessionManager
     * @since 20
     */
    BOTTOM_END = 8,
  }

  export interface Rect {
    /**
     * The left of the Rect.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * The left of the Rect.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @since 10
     */
    /**
     * The left of the Rect.
     *
     * @type { int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    left: int;

    /**
     * The top of the Rect.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * The top of the Rect.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @since 10
     */
    /**
     * The top of the Rect.
     *
     * @type { int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    top: int;

    /**
     * The width of the Rect.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * The width of the Rect.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @since 10
     */
    /**
     * The width of the Rect.
     *
     * @type { int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    width: int;

    /**
     * The height of the Rect.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * The height of the Rect.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @since 10
     */
    /**
     * The height of the Rect.
     *
     * @type { int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @crossplatform
     * @atomicservice
     * @since 11
     */
    height: int;
  }

  export class RectInternal implements Rect {
    left: int;
    top: int;
    width: int;
    height: int;
    constructor() {}
    constructor(left: int, top: int, width: int, height: int) {
      this.left = left;
      this.top = top;
      this.width = width;
      this.height = height;
    }
  }

  export interface RectChangeOptions {
    rect: Rect;
    reason: RectChangeReason;
  }

  export class SizeInternal implements Size {
    width: int;
    height: int;
  }

  export interface Position {
    x: int;
    y: int;
  }

  export class PositionInternal implements Position {
    x: int;
    y: int;
  }

  export interface KeyFramePolicy {
    enable: boolean;
    interval?: long;
    distance?: int;
    animationDuration?: long;
    animationDelay?: long;
  }

  export class KeyFramePolicyInternal implements KeyFramePolicy {
    enable: boolean;
    interval?: long;
    distance?: int;
    animationDuration?: long;
    animationDelay?: long;
  }

//enum WMError {
    const WMError_WM_OK = 0
    const WMError_WM_DO_NOTHING = 1
    const WMError_WM_ERROR_NO_MEM = 2
    const WMError_WM_ERROR_DESTROYED_OBJECT = 3
    const WMError_WM_ERROR_INVALID_WINDOW = 4
    const WMError_WM_ERROR_INVALID_WINDOW_MODE_OR_SIZE = 5
    const WMError_WM_ERROR_INVALID_OPERATION = 6
    const WMError_WM_ERROR_INVALID_PERMISSION = 7
    const WMError_WM_ERROR_NOT_SYSTEM_APP = 8
    const WMError_WM_ERROR_NO_REMOTE_ANIMATION = 9
    const WMError_WM_ERROR_INVALID_DISPLAY = 10
    const WMError_WM_ERROR_INVALID_PARENT = 11
    const WMError_WM_ERROR_OPER_FULLSCREEN_FAILED = 12
    const WMError_WM_ERROR_REPEAT_OPERATION = 13
    const WMError_WM_ERROR_INVALID_SESSION = 14
    const WMError_WM_ERROR_INVALID_CALLING = 15
    const WMError_WM_ERROR_SYSTEM_ABNORMALLY = 16

    const WMError_WM_ERROR_DEVICE_NOT_SUPPORT = 801 // the value do not change.It is defined on all system

    const WMError_WM_ERROR_NEED_REPORT_BASE = 1000 // error code > 1000 means need report
    const WMError_WM_ERROR_NULLPTR = 1001
    const WMError_WM_ERROR_INVALID_TYPE = 1002
    const WMError_WM_ERROR_INVALID_PARAM = 1003
    const WMError_WM_ERROR_SAMGR = 1004
    const WMError_WM_ERROR_IPC_FAILED = 1005
    const WMError_WM_ERROR_NEED_REPORT_END = 1006
    const WMError_WM_ERROR_START_ABILITY_FAILED = 1007
    const WMError_WM_ERROR_PIP_DESTROY_FAILED = 1008
    const WMError_WM_ERROR_PIP_STATE_ABNORMALLY = 1009
    const WMError_WM_ERROR_PIP_CREATE_FAILED = 1010
    const WMError_WM_ERROR_PIP_INTERNAL_ERROR = 1011
    const WMError_WM_ERROR_PIP_REPEAT_OPERATION = 1012
//}

//enum WmErrorCode {
    const WmErrorCode_WM_OK = 0
    const WmErrorCode_WM_ERROR_NO_PERMISSION = 201
    const WmErrorCode_WM_ERROR_NOT_SYSTEM_APP = 202
    const WmErrorCode_WM_ERROR_INVALID_PARAM = 401
    const WmErrorCode_WM_ERROR_DEVICE_NOT_SUPPORT = 801
    const WmErrorCode_WM_ERROR_REPEAT_OPERATION = 1300001
    const WmErrorCode_WM_ERROR_STATE_ABNORMALLY = 1300002
    const WmErrorCode_WM_ERROR_SYSTEM_ABNORMALLY = 1300003
    const WmErrorCode_WM_ERROR_INVALID_CALLING = 1300004
    const WmErrorCode_WM_ERROR_STAGE_ABNORMALLY = 1300005
    const WmErrorCode_WM_ERROR_CONTEXT_ABNORMALLY = 1300006
    const WmErrorCode_WM_ERROR_START_ABILITY_FAILED = 1300007
    const WmErrorCode_WM_ERROR_INVALID_DISPLAY = 1300008
    const WmErrorCode_WM_ERROR_INVALID_PARENT = 1300009
    const WmErrorCode_WM_ERROR_OPER_FULLSCREEN_FAILED = 1300010
    const WmErrorCode_WM_ERROR_PIP_DESTROY_FAILED = 1300011
    const WmErrorCode_WM_ERROR_PIP_STATE_ABNORMALLY = 1300012
    const WmErrorCode_WM_ERROR_PIP_CREATE_FAILED = 1300013
    const WmErrorCode_WM_ERROR_PIP_INTERNAL_ERROR = 1300014
    const WmErrorCode_WM_ERROR_PIP_REPEAT_OPERATION = 1300015
//}

export enum ColorSpace {
    DEFAULT = 0,
    WIDE_GAMUT = 1
}

export enum MaximizePresentation {
    FOLLOW_APP_IMMERSIVE_SETTING = 0,
    EXIT_IMMERSIVE = 1,
    ENTER_IMMERSIVE = 2,
    ENTER_IMMERSIVE_DISABLE_TITLE_AND_DOCK_HOVER = 3
}

export enum WindowStatusType {
    UNDEFINED = 0,
    FULL_SCREEN = 1,
    MAXIMIZE = 2,
    MINIMIZE = 3,
    FLOATING = 4,
    SPLIT_SCREEN = 5
}

export enum OcclusionState {
    NO_OCCLUSION = 0,
    PARTIAL_OCCLUSION = 1,
    FULL_OCCLUSION = 2
}

export enum PixelUnit {
    PX = 0,
    VP = 1,
}

export enum WindowStageEventType {
    SHOWN = 1,
    ACTIVE = 2,
    INACTIVE = 3,
    HIDDEN = 4,
    RESUMED = 5,
    PAUSED = 6
}

export enum WindowType {
    TYPE_APP = 0,
    TYPE_SYSTEM_ALERT = 1,
    TYPE_INPUT_METHOD = 2,
    TYPE_STATUS_BAR = 3,
    TYPE_PANEL = 4,
    TYPE_KEYGUARD = 5,
    TYPE_VOLUME_OVERLAY = 6,
    TYPE_NAVIGATION_BAR = 7,
    TYPE_FLOAT = 8,
    TYPE_WALLPAPER = 9,
    TYPE_DESKTOP = 10,
    TYPE_LAUNCHER_RECENT = 11,
    TYPE_LAUNCHER_DOCK = 12,
    TYPE_VOICE_INTERACTION = 13,
    TYPE_POINTER = 14,
    TYPE_FLOAT_CAMERA = 15,
    TYPE_DIALOG = 16,
    TYPE_SCREENSHOT = 17,
    TYPE_SYSTEM_TOAST = 18,
    TYPE_DIVIDER = 19,
    TYPE_GLOBAL_SEARCH = 20,
    TYPE_HANDWRITE = 21,
    TYPE_DYNAMIC = 25,
    TYPE_MUTISCREEN_COLLABORATION = 26,
    TYPE_MAIN = 32,
}

export enum Orientation {
    UNSPECIFIED = 0,
    PORTRAIT = 1,
    LANDSCAPE = 2,
    PORTRAIT_INVERTED = 3,
    LANDSCAPE_INVERTED = 4,
    AUTO_ROTATION = 5,
    AUTO_ROTATION_PORTRAIT = 6,
    AUTO_ROTATION_LANDSCAPE = 7,
    AUTO_ROTATION_RESTRICTED = 8,
    AUTO_ROTATION_PORTRAIT_RESTRICTED = 9,
    AUTO_ROTATION_LANDSCAPE_RESTRICTED = 10,
    LOCKED = 11,
    AUTO_ROTATION_UNSPECIFIED = 12,
    USER_ROTATION_PORTRAIT = 13,
    USER_ROTATION_LANDSCAPE = 14,
    USER_ROTATION_PORTRAIT_INVERTED = 15,
    USER_ROTATION_LANDSCAPE_INVERTED = 16,
    FOLLOW_DESKTOP = 17
}

export enum  RotationInfoType {
    WINDOW_ORIENTATION = 0,
    DISPLAY_ORIENTATION = 1,
    DISPLAY_ROTATION = 2,
}

export enum WindowEventType {
    WINDOW_SHOWN = 1,
    WINDOW_ACTIVE = 2,
    WINDOW_INACTIVE = 3,
    WINDOW_HIDDEN = 4,
    WINDOW_DESTROYED = 7
}

export enum WindowStageLifeCycleEventType {
    FOREGROUND = 1,
    RESUMED = 2,
    PAUSED = 3,
    BACKGROUND = 4,
}

export enum WindowMode {
    UNDEFINED = 1,
    FULLSCREEN = 2,
    PRIMARY = 3,
    SECONDARY = 4,
    FLOATING = 5
}

export enum WindowLayoutMode {
    WINDOW_LAYOUT_MODE_CASCADE = 0,
    WINDOW_LAYOUT_MODE_TILE = 1
}

export enum ModalityType {
    WINDOW_MODALITY = 0,
    APPLICATION_MODALITY = 1
}

export enum RotationChangeType {
  WINDOW_WILL_ROTATE = 0,
  WINDOW_DID_ROTATE = 1
}

export enum RectType {
  RELATIVE_TO_SCREEN = 0,
  RELATIVE_TO_PARENT_WINDOW = 1
}

export interface StatusBarProperty {
  contentColor: string;
}

export class StatusBarPropertyInternal implements StatusBarProperty {
  contentColor: string;
}

export interface WindowLayoutInfo {
  windowRect: Rect;
}

export class WindowLayoutInfoInternal implements WindowLayoutInfo {
  windowRect: Rect;
}

export interface WindowInfo {
  rect: Rect;
  globalDisplayRect?: Rect;
  bundleName: string;
  abilityName: string;
  windowId: int;
  windowStatusType: WindowStatusType;
  isFocused?: boolean;
}

export class WindowInfoInternal implements WindowInfo {
  rect: Rect;
  globalDisplayRect?: Rect;
  bundleName: string;
  abilityName: string;
  windowId: int;
  windowStatusType: WindowStatusType;
  isFocused?: boolean;
  windowStatusTypeInternal: int;
  isFocusedInternal: boolean;
}

export interface ShowWindowOptions {
    /**
     * Indicates whether the window get focus when it is showing
     *
     * @type { ?boolean }
     * @syscap SystemCapability.Window.SessionManager
     * @atomicservice
     * @since 20
     */
    focusOnShow?: boolean;
}

export class ShowWindowOptionsInternal implements ShowWindowOptions {
  focusOnShow?: boolean;
}

export enum BlurStyle {
  OFF = 0,
  THIN = 1,
  REGULAR = 2,
  THICK = 3
}

export interface SystemBarProperties {
    /**
     * The color of the status bar.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 6
     */
    /**
     * The color of the status bar.
     *
     * @type { ?string }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    statusBarColor?: string;

    /**
     * The light icon of the status bar.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * The light icon of the status bar.
     *
     * @type { ?boolean }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    isStatusBarLightIcon?: boolean;

    /**
     * The content color of the status bar
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 8
     */
    /**
     * The content color of the status bar
     *
     * @type { ?string }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    statusBarContentColor?: string;

    /**
     * The color of the navigation bar.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 6
     */
    /**
     * The color of the navigation bar.
     *
     * @type { ?string }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    navigationBarColor?: string;

    /**
     * The light icon of the navigation bar.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 7
     */
    /**
     * The light icon of the navigation bar.
     *
     * @type { ?boolean }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    isNavigationBarLightIcon?: boolean;

    /**
     * The content color of the navigation bar
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 8
     */
    /**
     * The content color of the navigation bar
     *
     * @type { ?string }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    navigationBarContentColor?: string;

    /**
     * Enable the animation of the status bar.
     *
     * @type { ?boolean }
     * @syscap SystemCapability.Window.SessionManager
     * @atomicservice
     * @since 12
     */
    enableStatusBarAnimation?: boolean;

    /**
     * Enable the animation of the navigation bar.
     *
     * @type { ?boolean }
     * @syscap SystemCapability.Window.SessionManager
     * @atomicservice
     * @since 12
     */
    enableNavigationBarAnimation?: boolean;
}

export class SystemBarPropertiesInternal implements SystemBarProperties {
    statusBarColor?: string;
    isStatusBarLightIcon?: boolean;
    statusBarContentColor?: string;
    navigationBarColor?: string;
    isNavigationBarLightIcon?: boolean;
    navigationBarContentColor?: string;
    enableStatusBarAnimation?: boolean;
    enableNavigationBarAnimation?: boolean;
}

/**
 * Properties of window, it couldn't update automatically
 *
 * @interface WindowProperties
 * @syscap SystemCapability.WindowManager.WindowManager.Core
 * @since 6
 */
/**
 * Properties of window, it couldn't update automatically
 *
 * @interface WindowProperties
 * @syscap SystemCapability.WindowManager.WindowManager.Core
 * @crossplatform
 * @since 10
 */
/**
 * Properties of window, it couldn't update automatically
 *
 * @interface WindowProperties
 * @syscap SystemCapability.WindowManager.WindowManager.Core
 * @crossplatform
 * @atomicservice
 * @since 11
 */
export interface WindowProperties {
    windowRect: Rect;
    drawableRect: Rect;
    globalDisplayRect?: Rect;
    type: WindowType;
    isFullScreen: boolean;
    isLayoutFullScreen: boolean;
    focusable: boolean;
    touchable: boolean;
    brightness: double;
    dimBehindValue: number;
    isKeepScreenOn: boolean;
    isPrivacyMode: boolean;
    isRoundCorner: boolean;
    isTransparent: boolean;
    id: int;
    displayId?: long;
    name?: string;
}

export class WindowPropertiesInternal implements WindowProperties {
    windowRect: Rect;
    drawableRect: Rect;
    isFullScreen: boolean;
    isLayoutFullScreen: boolean;
    focusable: boolean;
    touchable: boolean;
    brightness: double;
    dimBehindValue: number;
    isKeepScreenOn: boolean;
    isPrivacyMode: boolean;
    isRoundCorner: boolean;
    isTransparent: boolean;
    type: WindowType;
    id: int;
    displayId?: long;
    name?: string;
    globalDisplayRect?: Rect;

    constructor(windowRect: Rect, drawableRect: Rect, isFullScreen: boolean, isLayoutFullScreen: boolean,
      focusable: boolean, touchable: boolean, brightness: double, dimBehindValue: number, isKeepScreenOn: boolean,
      isPrivacyMode: boolean, isRoundCorner: boolean, isTransparent: boolean, typeInternal: int, id: int,
      displayId: long, name: string, globalDisplayRect: Rect)
    {
      this.windowRect = windowRect;
      this.drawableRect = drawableRect;
      this.globalDisplayRect = globalDisplayRect;
      this.isFullScreen = isFullScreen;
      this.isLayoutFullScreen = isLayoutFullScreen;
      this.focusable = focusable;
      this.touchable = touchable;
      this.brightness = brightness;
      this.dimBehindValue = dimBehindValue;
      this.isKeepScreenOn = isKeepScreenOn;
      this.isPrivacyMode = isPrivacyMode;
      this.isRoundCorner = isRoundCorner;
      this.isTransparent = isTransparent;
      this.type = typeInternal as WindowType;
      this.id = id;
      this.displayId = displayId;
      this.name = name;
    }

    constructor() {}
}

/**
  * Frame metrics
  *
  * @interface FrameMetrics
  * @syscap SystemCapability.Window.SessionManager
  * @since 22 dynamic&static
  */
export interface FrameMetrics {
  /**
    * Indicates whether the first frame of the window.
    *
    * @type { boolean }
    * @syscap SystemCapability.Window.SessionManager
    * @since 22 dynamic&static
    */
  firstDrawFrame: boolean;

  /**
    * Indicates the number of nanoseconds elapsed in the input handling stage of a frame.
    *
    * @type { long }
    * @syscap SystemCapability.Window.SessionManager
    * @since 22 dynamic&static
    */
  inputHandlingDuration: long;

  /**
    * Indicates the number of nanoseconds elapsed in the layout measure stage of a frame.
    *
    * @type { long }
    * @syscap SystemCapability.Window.SessionManager
    * @since 22 dynamic&static
    */
  layoutMeasureDuration: long;

  /**
    * Indicates the timestamp of the actual vsync for this frame. The value is expressed in nanoseconds.
    *
    * @type { long }
    * @syscap SystemCapability.Window.SessionManager
    * @since 22 dynamic&static
    */
  vsyncTimestamp: long;
}

export class FrameMetricsInternal implements FrameMetrics {
  firstDrawFrame: boolean;
  inputHandlingDuration: long;
  layoutMeasureDuration: long;
  vsyncTimestamp: long;
}

/**
  * Describes the scale options of window
  *
  * @interface ScaleOptions
  * @syscap SystemCapability.WindowManager.WindowManager.Core
  * @systemapi
  * @since 9
  */
export interface ScaleOptions {
  /**
    * The scale param of x direction. Default is 1.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  x?: double;

  /**
    * The scale param of y direction. Default is 1.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  y?: double;

  /**
    * The scale param of pivot point of x. Default is 0.5f, Interval is 0.f - 1.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  pivotX?: double;

  /**
    * The scale param of pivot point of y. Default is 0.5f, Interval is 0.f - 1.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  pivotY?: double;
}

export class ScaleOptionsInternal implements ScaleOptions {
  x?: double;
  y?: double;
  pivotX?: double;
  pivotY?: double;
}

/**
  * Describes the rotate options of window
  *
  * @interface RotateOptions
  * @syscap SystemCapability.WindowManager.WindowManager.Core
  * @systemapi
  * @since 9
  */
export interface RotateOptions {
  /**
    * The rotate degree of x direction. Default value is 0.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  x?: double;

  /**
    * The rotate degree of y direction. Default value is 0.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  y?: double;

  /**
    * The rotate degree of z direction. Default value is 0.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  z?: double;

  /**
    * The param of pivot point of x. Default is 0.5f, Interval is 0.f - 1.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  pivotX?: double;

  /**
    * The param of pivot point of y. Default is 0.5f, Interval is 0.f - 1.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  pivotY?: double;
}

export class RotateOptionsInternal implements RotateOptions {
  x?: double;
  y?: double;
  z?: double;
  pivotX?: double;
  pivotY?: double;
}

/**
  * Describes the translate options of windows
  *
  * @interface TranslateOptions
  * @syscap SystemCapability.WindowManager.WindowManager.Core
  * @systemapi
  * @since 9
  */
export interface TranslateOptions {
  /**
    * The translate pixel param of x direction. Default is 0.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  x?: double;

  /**
    * The translate pixel param of y direction. Default is 0.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  y?: double;

  /**
    * The translate pixel param of z direction. Default is 0.f
    *
    * @type { ?double }
    * @syscap SystemCapability.WindowManager.WindowManager.Core
    * @systemapi
    * @since 9
    */
  z?: double;
}

export class TranslateOptionsInternal implements TranslateOptions {
  x?: double;
  y?: double;
  z?: double;
}

/**
 * Rotation change info
 *
 * @interface RotationChangeInfo
 * @syscap SystemCapability.Window.SessionManager
 * @atomicservice
 * @since 19
 */
export interface RotationChangeInfo {
  /**
   * Rotation change type
   *
   * @type { RotationChangeType }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 19
   */
  type: RotationChangeType;
  /**
   * window orientation
   *
   * @type { int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 19
   */
  orientation: int;
  /**
   * Display id
   *
   * @type { long }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 19
   */
  displayId: long;
  /**
   * Display rect
   *
   * @type { Rect }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 19
   */
  displayRect: Rect;
}

export class RotationChangeInfoInternal implements RotationChangeInfo {
  type: RotationChangeType;
  orientation: int;
  displayId: long;
  displayRect: Rect;
}

/**
 * Rotation change result
 *
 * @interface RotationChangeResult
 * @syscap SystemCapability.Window.SessionManager
 * @atomicservice
 * @since 19
 */
export interface RotationChangeResult {
  /**
   * Rect type
   *
   * @type { RectType }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 19
   */
  rectType: RectType;
  /**
   * Window Rect
   *
   * @type { Rect }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 19
   */
  windowRect: Rect;
}

export interface MoveConfiguration {
    displayId?: long;
}

export class MoveConfigurationInternal implements MoveConfiguration {
    displayId?: long;
}

export interface WindowDensityInfo {
    systemDensity: double;
    defaultDensity: double;
    customDensity: double;
}

export class WindowDensityInfoInternal implements WindowDensityInfo {
    systemDensity: double;
    defaultDensity: double;
    customDensity: double;
}

export interface GetWindowsByCoordinateParam {
  displayId: long;
  windowNumber: int;
  x: int;
  y: int;
}

export class GetWindowsByCoordinateParamInternal implements GetWindowsByCoordinateParam {
  displayId: long;
  windowNumber: int;
  x: int;
  y: int;
}


export interface SystemBarStyle {
  statusBarContentColor?: string;
}

export interface AvoidAreaOptions {
    type: AvoidAreaType;
    area: AvoidArea;
}

export interface TransitionContext {
    toWindow: Window;
    completeTransition(isCompleted: boolean): void;
}

export interface TransitionController {
    animationForShown: (context: window.TransitionContext) => void;
    animationForHidden: (context: window.TransitionContext) => void;
}

export native function transContextFinalizerCallback(nativeObj: long): void;
let transContextFinalizerRegister = new FinalizationRegistry<long>(transContextFinalizerCallback);
export class TransitionContextInternal implements TransitionContext {
    toWindow: Window;
    private nativeObj: long = 0;

    setNativeObj(nativeObj: long): void {
      if (this.nativeObj) {
        transContextFinalizerRegister.unregister(this);
      }
      transContextFinalizerRegister.register(this, nativeObj, this);
      this.nativeObj = nativeObj;
    }

    private native completeTransitionSync(nativeObj: long, shouldHide: boolean): void;

    public completeTransition(isCompleted: boolean): void {
        this.completeTransitionSync(this.nativeObj, isCompleted);
    }
}

export native function transControllerFinalizerCallback(nativeObj: long): void;
let transControllerFinalizerRegister = new FinalizationRegistry<long>(transControllerFinalizerCallback);
export class TransitionControllerInternal implements TransitionController {
    static { loadLibrary('windowstageani_module.z') }
    private nativeObj: long = 0;

    setNativeObj(nativeObj: long): void {
      if (this.nativeObj) {
        transControllerFinalizerRegister.unregister(this);
      }
      transControllerFinalizerRegister.register(this, nativeObj, this);
      this.nativeObj = nativeObj;
    }

    animationForShown: (context: window.TransitionContext) => void;
    animationForHidden: (context: window.TransitionContext) => void;
}

export enum WindowTransitionType {
    DESTROY = 0,
}

export enum AnimationType {
    FADE_IN_OUT = 0,
    FADE_IN = 1,
    SEE_THE_WORLD = 100,
}

export enum WindowAnimationCurve {
    LINEAR = 0,
    INTERPOLATION_SPRING = 1,
    CUBIC_BEZIER = 2,
}

export interface WindowAnimationConfig {
    curve: WindowAnimationCurve;
    duration?: long;
    param?: WindowAnimationCurveParam;
}

export class WindowAnimationConfigInternal implements WindowAnimationConfig {
    curve: WindowAnimationCurve;
    duration?: long;
    param?: WindowAnimationCurveParam;
}

export interface StartAnimationSystemParams {
    type: AnimationType;
    animationConfig?: WindowAnimationConfig;
}

export class StartAnimationSystemParamsInternal implements StartAnimationSystemParams {
    type: AnimationType;
    animationConfig?: WindowAnimationConfig;
}

export interface StartAnimationParams {
    type: AnimationType;
}

export class StartAnimationParamsInternal implements StartAnimationParams {
    type: AnimationType;
}

export interface TransitionAnimation {
    config: WindowAnimationConfig;
    opacity?: double;
}

export class TransitionAnimationInternal implements TransitionAnimation {
    config: WindowAnimationConfig;
    opacity?: double;
}

export interface WindowCreateParams {
    animationParams?: StartAnimationParams;
    systemAnimationParams?: StartAnimationSystemParams;
}

export class WindowCreateParamsInternal implements WindowCreateParams {
    animationParams?: StartAnimationParams;
    systemAnimationParams?: StartAnimationSystemParams;
}

/**
 * Limits of window.
 *
 * @interface WindowLimits
 * @syscap SystemCapability.Window.SessionManager
 * @since 11
 */
/**
 * Limits of window.
 *
 * @interface WindowLimits
 * @syscap SystemCapability.Window.SessionManager
 * @atomicservice
 * @since 12
 */
export interface WindowLimits {

    /**
     * The maximum width of the window.
     *
     * @type { ?int }
     * @syscap SystemCapability.Window.SessionManager
     * @since 11
     */
    /**
     * The maximum width of the window.
     *
     * @type { ?int }
     * @syscap SystemCapability.Window.SessionManager
     * @atomicservice
     * @since 12
     */
    maxWidth?: int;

    /**
     * The maximum height of the window.
     *
     * @type { ?int }
     * @syscap SystemCapability.Window.SessionManager
     * @since 11
     */
    /**
     * The maximum height of the window.
     *
     * @type { ?int }
     * @syscap SystemCapability.Window.SessionManager
     * @atomicservice
     * @since 12
     */
    maxHeight?: int;

    /**
     * The minimum width of the window.
     *
     * @type { ?int }
     * @syscap SystemCapability.Window.SessionManager
     * @since 11
     */
    /**
     * The minimum width of the window.
     *
     * @type { ?int }
     * @syscap SystemCapability.Window.SessionManager
     * @atomicservice
     * @since 12
     */
    minWidth?: int;

    /**
     * The minimum height of the window.
     *
     * @type { ?int }
     * @syscap SystemCapability.Window.SessionManager
     * @since 11
     */
    /**
     * The minimum height of the window.
     *
     * @type { ?int }
     * @syscap SystemCapability.Window.SessionManager
     * @atomicservice
     * @since 12
     */
    minHeight?: int;

    /**
     * The unit of window limits.
     *
     * @type { ?PixelUnit }
     * @syscap SystemCapability.Window.SessionManager
     * @since 22 dynamic&static
     */
    pixelUnit?:PixelUnit;
}

export class WindowLimitsInternal implements WindowLimits {
    maxWidth?: int;
    maxHeight?: int;
    minWidth?: int;
    minHeight?: int;
    pixelUnit?: PixelUnit;
}

/**
 * Configuration parameters for window creation.
 *
 * @interface Configuration
 * @syscap SystemCapability.WindowManager.WindowManager.Core
 * @since 9
 */
/**
 * Configuration parameters for window creation.
 *
 * @interface Configuration
 * @syscap SystemCapability.WindowManager.WindowManager.Core
 * @atomicservice
 * @since 12
 */
export interface Configuration {
    /**
     * Indicates window id.
     *
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 9
     */
    /**
     * Indicates window id.
     *
     * @type { string }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    name: string;

    /**
     * Indicates window type
     *
     * @type { WindowType }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 9
     */
    /**
     * Indicates window type
     *
     * @type { WindowType }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    windowType: WindowType;

    /**
     * Indicates window context.
     *
     * @type { ?BaseContext }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 9
     */
    /**
     * Indicates window context.
     *
     * @type { ?BaseContext }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    ctx?: BaseContext;

    /**
     * Indicates display id.
     *
     * @type { ?number }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 9
     */
    /**
     * Indicates display id.
     *
     * @type { ?long }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    displayId?: long;

    /**
     * Indicates parent window id
     *
     * @type { ?int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @since 9
     */
    /**
     * Indicates parent window id
     *
     * @type { ?int }
     * @syscap SystemCapability.WindowManager.WindowManager.Core
     * @atomicservice
     * @since 12
     */
    parentId?: int;

    /**
     * Indicates whether enable window decor, only support dialog, the default value is false.
     *
     * @type { ?boolean }
     * @syscap SystemCapability.Window.SessionManager
     * @atomicservice
     * @since 12
     */
    decorEnabled?: boolean;

    /**
     * Indicates dialog window title when decor enabled.
     *
     * @type { ?string }
     * @syscap SystemCapability.Window.SessionManager
     * @atomicservice
     * @since 12
     */
    title?: string;
}

export class ConfigurationInternal implements Configuration {
    name: string;
    windowType: WindowType;
    ctx?: BaseContext;
    displayId?: long;
    parentId?: int;
    decorEnabled?: boolean;
    title?: string;
}

export interface KeyboardInfo {
    beginRect: Rect;
    endRect: Rect;
    animated?: boolean;
    config?: WindowAnimationConfig;
}

export class KeyboardInfoInternal implements KeyboardInfo {
    beginRect: Rect;
    endRect: Rect;
    animated?: boolean;
    config?: WindowAnimationConfig;
}

/**
 * Rectangular area of the title buttons relative to the upper right corner of the window.
 *
 * @interface TitleButtonRect
 * @syscap SystemCapability.Window.SessionManager
 * @since 11
 */
/**
 * Rectangular area of the title buttons relative to the upper right corner of the window.
 *
 * @interface TitleButtonRect
 * @syscap SystemCapability.Window.SessionManager
 * @atomicservice
 * @since 12
 */
export interface TitleButtonRect {
  /**
   * The right of the Rect.
   *
   * @type { int }
   * @syscap SystemCapability.Window.SessionManager
   * @since 11
   */
  /**
   * The right of the Rect.
   *
   * @type { int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 12
   */
  right: int;
  /**
   * The top of the Rect.
   *
   * @type { int }
   * @syscap SystemCapability.Window.SessionManager
   * @since 11
   */
  /**
   * The top of the Rect.
   *
   * @type { int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 12
   */
  top: int;
  /**
   * The width of the Rect.
   *
   * @type { int }
   * @syscap SystemCapability.Window.SessionManager
   * @since 11
   */
  /**
   * The width of the Rect.
   *
   * @type { int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 12
   */
  width: int;
  /**
   * The height of the Rect.
   *
   * @type { int }
   * @syscap SystemCapability.Window.SessionManager
   * @since 11
   */
  /**
   * The height of the Rect.
   *
   * @type { int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 12
   */
  height: int;
}

export class TitleButtonRectInternal implements TitleButtonRect {
  right: int;
  top: int;
  width: int;
  height: int;
}
 
/**
 * The decor button style of the window.
 * 
 * @interface DecorButtonStyle
 * @syscap SystemCapability.Window.SessionManager
 * @atomicservice
 * @since 14
 */
export interface DecorButtonStyle {
  /**
   * color mode.
   * 
   * @type { ?colorMode }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 14
   */
  colorMode?: ConfigurationConstant.ColorMode;
 
  /**
   * button background size when hover.
   * 
   * @type { ?int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 14
   */
  buttonBackgroundSize?: int;
 
  /**
   * button spacing.
   * 
   * @type { ?int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 14
   */
  spacingBetweenButtons?: int;
 
  /**
   * close button right margin.
   * 
   * @type { ?int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 14
   */
  closeButtonRightMargin?: int;
 
  /**
   * button icon size.
   *
   * @type { ?int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 20
   */
  buttonIconSize?: int;
 
  /**
   * corner radius of button background when hover.
   *
   * @type { ?int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 20
   */
  buttonBackgroundCornerRadius?: int;
}
 
export class DecorButtonStyleInternal implements DecorButtonStyle {
  colorMode?: ConfigurationConstant.ColorMode;
  buttonBackgroundSize?: int;
  spacingBetweenButtons?: int;
  closeButtonRightMargin?: int;
  buttonIconSize?: int;
  buttonBackgroundCornerRadius?: int;
}
 
export interface WindowTitleButtonVisibleParam {
  isMaximizeButtonVisible: boolean;
  isMinimizeButtonVisible: boolean;
  isCloseButtonVisible: boolean;
}

/**
 * Options for subwindow creation
 * 
 * @interface SubWindowOptions
 * @syscap SystemCapability.Window.SessionManager
 * @since 11
 */
/**
 * Options for subwindow creation
 * 
 * @interface SubWindowOptions
 * @syscap SystemCapability.Window.SessionManager
 * @atomicservice
 * @since 12
 */
export interface SubWindowOptions {
  /**
   * Indicates subwindow title
   * 
   * @type { string }
   * @syscap SystemCapability.Window.SessionManager
   * @since 11
   */
  /**
   * Indicates subwindow title
   * 
   * @type { string }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 12
   */
  title: string;
  /**
   * Indicates decor of subwindow
   * 
   * @type { boolean }
   * @syscap SystemCapability.Window.SessionManager
   * @since 11
   */
  /**
   * Indicates decor of subwindow
   * 
   * @type { boolean }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 12
   */
  decorEnabled: boolean;
  /**
   * Indicates modality of subwindow
   * 
   * @type { ?boolean }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 12
   */
  isModal?: boolean;
  /**
   * Indicates whether subwindow is topmost
   * 
   * @type { ?boolean }
   * @syscap SystemCapability.Window.SessionManager
   * @systemapi Hide this for inner system use.
   * @since 12
   */
  isTopmost?: boolean;
  /**
   * Indicates modality type of subwindow
   * 
   * @type { ?ModalityType }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 14
   */
  modalityType?: ModalityType;
  /**
   * Indicates position and size of subwindow
   * 
   * @type { ?Rect }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 18
   */
  windowRect?: Rect;
  /**
   * Indicates whether subwindow support fullscreen
   * 
   * @type { ?boolean }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 19
   */
  maximizeSupported?: boolean;
  /**
   * Indicates zlevel of subwindow
   * 
   * @type { ?int }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 18
   */
  zLevel?: int;
  /**
   * Indicates whether subwindow show outline
   * 
   * @type { ?boolean }
   * @syscap SystemCapability.Window.SessionManager
   * @atomicservice
   * @since 20
   */
  outlineEnabled?: boolean;
}

/**
 * Attribute of the window for UI service extension ability.
 *
 * @enum { number }
 * @syscap SystemCapability.Window.SessionManager
 * @systemapi Hide this for inner system use.
 * @stagemodelonly
 * @since 14
 */
export enum ExtensionWindowAttribute {
  /**
   * System window.
   *
   * @syscap SystemCapability.Window.SessionManager
   * @systemapi Hide this for inner system use.
   * @stagemodelonly
   * @since 14
   */
  SYSTEM_WINDOW = 0,

  /**
   * Sub window.
   *
   * @syscap SystemCapability.Window.SessionManager
   * @systemapi Hide this for inner system use.
   * @stagemodelonly
   * @since 14
   */
  SUB_WINDOW = 1
}

/**
 * Options for system window creation.
 *
 * @interface SystemWindowOptions
 * @syscap SystemCapability.Window.SessionManager
 * @systemapi Hide this for inner system use.
 * @stagemodelonly
 * @since 14
 */
export interface SystemWindowOptions {
  /**
   * Indicates window type.
   *
   * @type { WindowType }
   * @syscap SystemCapability.Window.SessionManager
   * @systemapi Hide this for inner system use.
   * @stagemodelonly
   * @since 14
   */
  windowType: WindowType;
}

/**
 * Configuration of the window for UI service extension ability.
 *
 * @interface ExtensionWindowConfig
 * @syscap SystemCapability.Window.SessionManager
 * @systemapi Hide this for inner system use.
 * @stagemodelonly
 * @since 14
 */
export interface ExtensionWindowConfig {
  /**
   * Window name.
   *
   * @type { string }
   * @syscap SystemCapability.Window.SessionManager
   * @systemapi Hide this for inner system use.
   * @stagemodelonly
   * @since 14
   */
  windowName: string;

  /**
   * Defines whether the window is a subwindow or a system window.
   *
   * @type { ExtensionWindowAttribute }
   * @syscap SystemCapability.Window.SessionManager
   * @systemapi Hide this for inner system use.
   * @stagemodelonly
   * @since 14
   */
  windowAttribute: ExtensionWindowAttribute;

  /**
   * The position and size of window.
   *
   * @type { Rect }
   * @syscap SystemCapability.Window.SessionManager
   * @systemapi Hide this for inner system use.
   * @stagemodelonly
   * @since 14
   */
  windowRect: Rect;

  /**
   * The options of sub window creation.
   *
   * @type { ?SubWindowOptions }
   * @syscap SystemCapability.Window.SessionManager
   * @systemapi Hide this for inner system use.
   * @stagemodelonly
   * @since 14
   */
  subWindowOptions?: SubWindowOptions;

  /**
   * The options of system window creation.
   *
   * @type { ?SystemWindowOptions }
   * @syscap SystemCapability.Window.SessionManager
   * @systemapi Hide this for inner system use.
   * @stagemodelonly
   * @since 14
   */
  systemWindowOptions?: SystemWindowOptions;
}

export native function destroyAniExtConfig(nativeObj: long): void;
let extConfigRegister = new FinalizationRegistry<long>(destroyAniExtConfig);

export class ExtensionWindowConfigInternal implements ExtensionWindowConfig {
  static { loadLibrary('windowstageani_module.z') }

  private nativeObj: long = 0;
  setNativeObj(nativeObj: long): void {
      this.nativeObj = nativeObj;
      extConfigRegister.register(this, nativeObj);
  }

  set windowName(value: string) {
    this.setWindowName(this.nativeObj, value);
  }

  get windowName(): string {
    return this.getWindowName(this.nativeObj);
  }

  set windowAttribute(value: ExtensionWindowAttribute) {
    this.setWindowAttribute(this.nativeObj, value);
  }

  get windowAttribute(): ExtensionWindowAttribute {
    return this.getWindowAttribute(this.nativeObj);
  }

  set windowRect(value: Rect) {
    this.setWindowRect(this.nativeObj, value);
  }

  get windowRect(): Rect {
    return this.getWindowRect(this.nativeObj);
  }

  set subWindowOptions(value: SubWindowOptions | undefined) {
    this.setSubWindowOptions(this.nativeObj, value);
  }

  get subWindowOptions(): SubWindowOptions | undefined {
    return this.getSubWindowOptions(this.nativeObj);
  }

  set systemWindowOptions(value: SystemWindowOptions | undefined) {
    this.setSystemWindowOptions(this.nativeObj, value);
  }

  get systemWindowOptions(): SystemWindowOptions | undefined {
    return this.getSystemWindowOptions(this.nativeObj);
  }

  public native setWindowName(nativeObj: long, value: string): void;
  public native getWindowName(nativeObj: long): string;
  public native setWindowAttribute(nativeObj: long, value: ExtensionWindowAttribute): void;
  public native getWindowAttribute(nativeObj: long): ExtensionWindowAttribute;
  public native setWindowRect(nativeObj: long, value: Rect): void;
  public native getWindowRect(nativeObj: long): Rect;
  public native setSubWindowOptions(nativeObj: long, value?: SubWindowOptions): void;
  public native getSubWindowOptions(nativeObj: long): SubWindowOptions;
  public native setSystemWindowOptions(nativeObj: long, value?: SystemWindowOptions): void;
  public native getSystemWindowOptions(nativeObj: long): SystemWindowOptions;
}

export class ExtConfigRect implements Rect {
  static { loadLibrary('windowstageani_module.z') }

  private nativeObj: long = 0;
  setNativeObj(nativeObj: long): void {
      this.nativeObj = nativeObj;
      extConfigRegister.register(this, nativeObj);
  }
  
  set left(value: int) {
    this.setLeft(this.nativeObj, value);
  }

  get left(): int {
    return this.getLeft(this.nativeObj);
  }

  set top(value: int) {
    this.setTop(this.nativeObj, value);
  }

  get top(): int {
    return this.getTop(this.nativeObj);
  }

  set width(value: int) {
    this.setWidth(this.nativeObj, value);
  }

  get width(): int {
    return this.getWidth(this.nativeObj);
  }

  set height(value: int) {
    this.setHeight(this.nativeObj, value);
  }

  get height(): int {
    return this.getHeight(this.nativeObj);
  }

  public native setLeft(nativeObj: long, value: int): void;
  public native getLeft(nativeObj: long): int;
  public native setTop(nativeObj: long, value: int): void;
  public native getTop(nativeObj: long): int;
  public native setWidth(nativeObj: long, value: int): void;
  public native getWidth(nativeObj: long): int;
  public native setHeight(nativeObj: long, value: int): void;
  public native getHeight(nativeObj: long): int;
}

export class ExtConfigSubWindowOptions implements SubWindowOptions {
  static { loadLibrary('windowstageani_module.z') }

  private nativeObj: long = 0;
  setNativeObj(nativeObj: long): void {
      this.nativeObj = nativeObj;
      extConfigRegister.register(this, nativeObj);
  }

  modalityType?: ModalityType;
  windowRect?: Rect;
  maximizeSupported?: boolean;
  zLevel?: int;
  outlineEnabled?: boolean;

  set title(value: string) {
    this.setTitle(this.nativeObj, value);
  }

  get title(): string {
    return this.getTitle(this.nativeObj);
  }

  set decorEnabled(value: boolean) {
    this.setDecorEnabled(this.nativeObj, value);
  }

  get decorEnabled(): boolean {
    return this.getDecorEnabled(this.nativeObj);
  }

  set isModal(value: boolean | undefined) {
    this.setIsModal(this.nativeObj, value ?? false);
  }

  get isModal(): boolean | undefined {
    return this.getIsModal(this.nativeObj);
  }

  set isTopmost(value: boolean | undefined) {
    this.setIsTopmost(this.nativeObj, value ?? false);
  }

  get isTopmost(): boolean | undefined {
    return this.getIsTopmost(this.nativeObj);
  }

  public native setTitle(nativeObj: long, value: string): void;
  public native getTitle(nativeObj: long): string;
  public native setDecorEnabled(nativeObj: long, value: boolean): void;
  public native getDecorEnabled(nativeObj: long): boolean;
  public native setIsModal(nativeObj: long, value: boolean): void;
  public native getIsModal(nativeObj: long): boolean;
  public native setIsTopmost(nativeObj: long, value: boolean): void;
  public native getIsTopmost(nativeObj: long): boolean; 
}

export class ExtConfigSystemWindowOptions implements SystemWindowOptions {
  static { loadLibrary('windowstageani_module.z') }

  private nativeObj: long = 0;
  setNativeObj(nativeObj: long): void {
      this.nativeObj = nativeObj;
      extConfigRegister.register(this, nativeObj);
  }

  set windowType(value: WindowType) {
    this.setWindowType(this.nativeObj, value);
  }

  get windowType(): WindowType {
    return this.getWindowType(this.nativeObj);
  }

  public native setWindowType(nativeObj: long, value: WindowType): void;
  public native getWindowType(nativeObj: long): WindowType;
}

export class WindowStageInternal implements WindowStage {
    static { loadLibrary('windowstageani_module.z') }

    private nativeObj: long = 0;
    setNativeObj(nativeObj: long):void {
        this.nativeObj = nativeObj;
    }

    private static native nativeTransferStatic(input: ESValue): Object;
    private static native nativeTransferDynamic(nativeObj: long): ESValue;
    public native loadContentSync(nativeObj: long, path: string, storage?: LocalStorage): void;
    public native loadContentByNameSync(nativeObj: long, path: string, storage?: LocalStorage): void;
    public native disableWindowDecorSync(nativeObj:long): void;
    public native setShowOnLockScreenSync(nativeObj:long, showOnLockScreen: boolean): void;
    public native setWindowModal(nativeObj: long, isModal: boolean): void;
    public native getMainWindowSync(nativeObj: long): Window;
    public native createSubWindowSync(nativeObj: long, name: String): Window;
    public native setWindowRectAutoSave(nativeObj: long, enabled: boolean): void;
    public native setWindowRectAutoSave(nativeObj: long, enabled: boolean, isSaveBySpecifiedFlag: boolean): void;
    public native isWindowRectAutoSave(nativeObj: long): boolean;
    public native setImageForRecentSync(nativeObj: long, imageResource: long | image.PixelMap, value: int): void;
    public native removeImageForRecentSync(nativeObj: long): void;
    public native setCustomDensitySync(nativeObj: long, density: double, applyToSubWindow: boolean): void;
    public native setDefaultDensityEnabledSync(nativeObj: long, enabled: boolean): void;
    public native removeStartingWindowSync(nativeObj: long): void;
    public native setSupportedWindowModes(
      nativeObj: long, supportedWindowModes: Array<bundleManager.SupportWindowMode>): void;
    public native setSupportedWindowModes(
      nativeObj: long, supportedWindowModes: Array<bundleManager.SupportWindowMode>,
      grayOutMaximizeButton: boolean): void;

    public native getSubWindowSync(nativeObj: long): Array<Window>;
    public native createSubWindowWithOptionsSync(nativeObj: long, name: string, options: SubWindowOptions): Window;
    native onSync(nativeObj: long, eventType: string, callback: Object): void;
    native offSync(nativeObj: long, eventType: string, callback?: Object): void;

    public loadContent(path: string, storage: LocalStorage, callback: AsyncCallback<void>): void {
      try {
        this.loadContentSync(this.nativeObj, path, storage);
        callback(new BusinessError(), undefined);
      } catch (err: Error) {
        callback(err as BusinessError, undefined);
      }
    }

    public loadContent(path: string, storage?: LocalStorage): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        try {
          this.loadContentSync(this.nativeObj, path, storage);
          resolve(undefined);
        } catch (err: Error) {
          reject(err as BusinessError);
        }
      });
    }

    public loadContent(path: string, callback: AsyncCallback<void>): void {
      try {
        this.loadContentSync(this.nativeObj, path, undefined);
        callback(new BusinessError(), undefined);
      } catch (err: Error) {
        callback(err as BusinessError, undefined);
      }
    }

    public loadContentByName(name: string, storage: LocalStorage, callback: AsyncCallback<void>): void {
      try {
        this.loadContentByNameSync(this.nativeObj, name, storage);
        callback(new BusinessError(), undefined);
      } catch (err: Error) {
        callback(err as BusinessError, undefined);
      }
    }

    public loadContentByName(name: string, storage?: LocalStorage): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        try {
          this.loadContentByNameSync(this.nativeObj, name, storage);
          resolve(undefined);
        } catch (err: Error) {
          reject(err as BusinessError);
        }
      });
    }

    public loadContentByName(name: string, callback: AsyncCallback<void>): void {
      try {
        this.loadContentByNameSync(this.nativeObj, name, undefined);
        callback(new BusinessError(), undefined);
      } catch (err: Error) {
        callback(err as BusinessError, undefined);
      }
    }

    public disableWindowDecor(): void {
        this.disableWindowDecorSync(this.nativeObj);
    }

    public setShowOnLockScreen(showOnLockScreen: boolean): void {
        this.setShowOnLockScreenSync(this.nativeObj, showOnLockScreen);
    }

    public setWindowModal(isModal: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.setWindowModal(this.nativeObj, isModal);
            }).then((ret: Any): void => {
                resolve(undefined);
            }).catch((err: Any): void => {
                reject(err as BusinessError);
            });
        });
    }
    public getMainWindowSync(): Window {
        let ret = this.getMainWindowSync(this.nativeObj);
        return ret;
    }

    public getMainWindow(): Promise<Window> {
        return new Promise<Window>((resolve: (value: Window) => void, reject: (error: BusinessError) => void ) => {
            taskpool.execute((): Window => {
                let window = this.getMainWindowSync(this.nativeObj);
                return window;
            }).then((ret: Any) => {
                resolve(ret as Window);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    public getMainWindow(callback: AsyncCallback<Window>): void {
        taskpool.execute((): Window => {
            let res = this.getMainWindowSync(this.nativeObj);
            return res;
        }).then((ret: Any) => {
            callback(new BusinessError(), ret as Window);
        }).catch((err: Any) => {
        });
    }

    public createSubWindow(name: string): Promise<Window> {
        return new Promise<Window>((resolve: (value: Window) => void, reject: (error: BusinessError) => void) => {
            taskpool.execute((): Window => {
                let window = this.createSubWindowSync(this.nativeObj, name);
                return window;
            }).then((ret: Any) => {
                resolve(ret as Window);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    public createSubWindow(name: string, callback: AsyncCallback<Window>): void {
        taskpool.execute((): Window => {
            let res = this.createSubWindowSync(this.nativeObj, name);
            return res;
        }).then((ret: Any) => {
            callback(new BusinessError(), ret as Window);
        }).catch((err: Any) => {
        });
    }

    public setWindowRectAutoSave(enabled: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
            taskpool.execute((): void => {
                this.setWindowRectAutoSave(this.nativeObj, enabled, false);
            }).then((ret: Any) => {
                resolve(undefined);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    public setWindowRectAutoSave(enabled: boolean, isSaveBySpecifiedFlag: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
            taskpool.execute((): void => {
                this.setWindowRectAutoSave(this.nativeObj, enabled, isSaveBySpecifiedFlag);
            }).then((ret: Any) => {
                resolve(undefined);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    public isWindowRectAutoSave(): Promise<boolean> {
        return new Promise<boolean>((resolve: (value: boolean) => void, reject: (error: BusinessError) => void) => {
            taskpool.execute((): boolean => {
                return this.isWindowRectAutoSave(this.nativeObj);
            }).then((ret: Any) => {
                resolve(ret as boolean);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    public onWindowStageClose(callback: WindowStageCloseCallback<void, boolean>): void {
        this.onSync(this.nativeObj, 'windowStageClose', callback);
    }

    public offWindowStageClose(callback?: WindowStageCloseCallback<void, boolean>): void {
        this.offSync(this.nativeObj, 'windowStageClose', callback);
    }

    public getSubWindow(): Promise<Array<Window>> {
      return new Promise<Array<Window>>((resolve: (value: Array<Window>) => void,
        reject: (error: BusinessError) => void) => {
          taskpool.execute((): Array<Window> => {
              return this.getSubWindowSync(this.nativeObj);
          }).then((ret: Any) => {
              resolve(ret as Array<Window>);
          }).catch((err: Any) => {
              reject(err as BusinessError);
          });
      });
    }

    public getSubWindow(callback: AsyncCallback<Array<Window>>): void {
      taskpool.execute((): Array<Window> => {
          return this.getSubWindowSync(this.nativeObj);
      }).then((ret: Any) => {
          callback(new BusinessError(), ret as Array<Window>);
      }).catch((err: Any) => {
          callback(err as BusinessError, undefined);
      });
    }

    public createSubWindowWithOptions(name: string, options: SubWindowOptions): Promise<Window> {
      return new Promise<Window>((resolve: (value: Window) => void, reject: (error: BusinessError) => void) => {
          taskpool.execute((): Window => {
              return this.createSubWindowWithOptionsSync(this.nativeObj, name, options);
          }).then((ret: Any) => {
              resolve(ret as Window);
          }).catch((err: Any) => {
              reject(err as BusinessError);
          });
      });
    }
    
    public on<T>(eventType: string, callback: Callback<T>): void {
      this.onSync(this.nativeObj, eventType, callback);
    }

    public off<T>(eventType: string, callback?: Callback<T>): void {
      this.offSync(this.nativeObj, eventType, callback);
    }

    public static transferStatic(input: Any): Object {
      if (!input) {
        throw new BusinessError(WM_ERROR_INVALID_PARAM, new Error('TransferStatic invalid input!'));
      }
      hilog.info(DOMAIN, TAG, 'transfer static, input:' + input);
      let windowStage: ESValue = ESValue.wrap(input).getPropertySafe('windowStage_');
      if (windowStage !== ESValue.Undefined) {
        return WindowStageInternal.nativeTransferStatic(windowStage);
      } else {
        return WindowStageInternal.nativeTransferStatic(ESValue.wrap(input));
      }
    }

    public static transferDynamic(input: Object): Any {
      if (!input) {
        throw new BusinessError(WM_ERROR_INVALID_PARAM, new Error('TransferDynamic invalid input!'));
      }
      hilog.info(DOMAIN, TAG, 'transfer dynamic, input:' + input);
      if (input instanceof WindowStageInternal) {
        return WindowStageInternal.nativeTransferDynamic((input as WindowStageInternal).nativeObj).unwrap();
      } else {
        throw new BusinessError(WM_ERROR_INVALID_PARAM, new Error('Invalid instance input!'));
      }
    }

    public setImageForRecent(imageResource: long | image.PixelMap, value: ImageFit): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setImageForRecentSync(this.nativeObj, imageResource, value as int);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }
 
    public removeImageForRecent(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        try {
          this.removeImageForRecentSync(this.nativeObj);
          resolve(undefined);
        } catch (err: Error) {
          reject(err as BusinessError);
        }
      });
    }

    public setCustomDensity(density: double, applyToSubWindow?: boolean): void {
      let isApplyToSubWindow = applyToSubWindow ? applyToSubWindow : false;
      this.setCustomDensitySync(this.nativeObj, density, isApplyToSubWindow);
    }

    public setDefaultDensityEnabled(enabled: boolean): void {
      this.setDefaultDensityEnabledSync(this.nativeObj, enabled);
    }

    public removeStartingWindow(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.removeStartingWindowSync(this.nativeObj);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }

    public setSupportedWindowModes(supportedWindowModes: Array<bundleManager.SupportWindowMode>): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute(() => {
          this.setSupportedWindowModes(this.nativeObj, supportedWindowModes);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setSupportedWindowModes(
      supportedWindowModes: Array<bundleManager.SupportWindowMode>, grayOutMaximizeButton: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute(() => {
          this.setSupportedWindowModes(this.nativeObj, supportedWindowModes, grayOutMaximizeButton);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public onWindowStageEvent(callback: Callback<WindowStageEventType>): void {
      this.onSync(this.nativeObj, 'windowStageEvent', callback);
    }

    public offWindowStageEvent(callback?: Callback<WindowStageEventType>): void {
      this.offSync(this.nativeObj, 'windowStageEvent', callback);
    }
}

export interface WindowStage {
    loadContent(path: string, storage: LocalStorage, callback: AsyncCallback<void>): void;
    loadContent(path: string, storage?: LocalStorage): Promise<void>;
    loadContent(path: string, callback: AsyncCallback<void>): void;
    loadContentByName(name: string, storage: LocalStorage, callback: AsyncCallback<void>): void;
    loadContentByName(name: string, storage?: LocalStorage): Promise<void>;
    loadContentByName(name: string, callback: AsyncCallback<void>): void;
    getMainWindowSync(): Window;
    getMainWindow(): Promise<Window>;
    getMainWindow(callback: AsyncCallback<Window>): void;
    disableWindowDecor(): void;
    setShowOnLockScreen(showOnLockScreen: boolean): void;

    setImageForRecent(imageResource: long | image.PixelMap, value: ImageFit): Promise<void>;
    removeImageForRecent(): Promise<void>;
    setWindowModal(isModal: boolean): Promise<void>;
    setCustomDensity(density: double, applyToSubWindow?: boolean): void;
    setDefaultDensityEnabled(enabled: boolean): void;
    removeStartingWindow(): Promise<void>;
    createSubWindow(name: string): Promise<Window>;
    createSubWindow(name: string, callback: AsyncCallback<Window>): void;
    setWindowRectAutoSave(enabled: boolean): Promise<void>;
    setWindowRectAutoSave(enabled: boolean, isSaveBySpecifiedFlag: boolean): Promise<void>;
    isWindowRectAutoSave(): Promise<boolean>;
    onWindowStageClose(callback: WindowStageCloseCallback<void, boolean>): void;
    offWindowStageClose(callback?: WindowStageCloseCallback<void, boolean>): void;
    setSupportedWindowModes(supportedWindowModes: Array<bundleManager.SupportWindowMode>): Promise<void>;
    setSupportedWindowModes(
      supportedWindowModes: Array<bundleManager.SupportWindowMode>, grayOutMaximizeButton: boolean): Promise<void>;

    getSubWindow(): Promise<Array<Window>>;
    getSubWindow(callback: AsyncCallback<Array<Window>>): void;
    createSubWindowWithOptions(name: string, options: SubWindowOptions): Promise<Window>;
    on<T>(eventType: string, callback: Callback<T>): void;
    off<T>(eventType: string, callback?: Callback<T>): void;
    onWindowStageEvent(callback: Callback<WindowStageEventType>): void;
    offWindowStageEvent(callback?: Callback<WindowStageEventType>): void;
}

export class WindowInternal implements Window {
    static readonly DEFAULT_RET_VAL = 1;
    static { loadLibrary('windowstageani_module.z') }
    private nativeObj: long;

    public setNativeObj(nativeObj: long): void {
        this.nativeObj = nativeObj;
    }

    private native hideSync(nativeObj: long): void;
    private native createSubWindowWithOptionsSync(nativeObj: long, name: string, options: SubWindowOptions): Window;
    private native restore(nativeObj: long): void;
    private native restoreMainWindow(nativeObj: long, wantParameters?: Record<string, object>): void;

    private static native nativeTransferStatic(input: ESValue): Object;
    private static native nativeTransferDynamic(nativeObj: long): ESValue;

    private native throwIfWindowInvalid(nativeObj: long): void;
    private native setWindowTopmost(nativeObj: long, isWindowTopmost: boolean): void;
    private native resize(nativeObj: long, width: int, height: int): void;
    private native moveWindowTo(nativeObj: long, x: int, y: int): void;
    private native getGlobalRect(nativeObj:long): Rect;
    private native setDecorButtonStyle(nativeObj: long, decorStyle: DecorButtonStyle): void;
    private native setWindowTitle(nativeObj:long, titleName: string): void;
    private native setTitleButtonVisible(nativeObj: long, isMaximizeVisible: boolean,
      isMinimizeVisible: boolean, isSplitVisible: boolean, isCloseVisible: boolean): void;
    private native setWindowTitleButtonVisible(nativeObj: long, visibleParam: WindowTitleButtonVisibleParam): void;
    private native setWindowTitleMoveEnabled(nativeObj: long, enabled: boolean): void;
    private native getTitleButtonRect(nativeObj: long): TitleButtonRect;
    private native getDecorButtonStyle(nativeObj: long): DecorButtonStyle;
    private native setTitleAndDockHoverShown(nativeObj: long, isTitleHoverShown?: boolean,
      isDockHoverShown?: boolean): void;
    private native setHandwritingFlag(nativeObj: long, enable: boolean): void;
    private native getWindowDecorVisible(nativeObj: long): boolean;
    private native getWindowDecorHeight(nativeObj:long): int;
    private native setWindowBackgroundColor(nativeObj: long, color: string): int;
    private native setImmersiveModeEnabledState(nativeObj: long, enable: boolean): int;
    private native setWindowDecorVisible(nativeObj: long, isVisible: boolean): int;
    private native setWindowDecorHeight(nativeObj: long, height: int): int;
    private native getWindowProperties(nativeObj: long): WindowProperties;
    private native getProperties(nativeObj: long): WindowProperties;
    private native isWindowSupportWideGamut(nativeObj: long): boolean;
    private native setWindowLayoutFullScreen(nativeObj: long, isLayoutFullScreen: boolean): int;
    private native setRaiseByClickEnabled(nativeObj: long, enable: boolean): int;
    private native setWindowSystemBarProperties(nativeObj: long, systemBarProperties: SystemBarProperties): int;
    private native setExclusivelyHighlighted(nativeObj: long, exclusivelyHighlighted: boolean): int;
    private native setSpecificSystemBarEnabled(
      nativeObj: long, name: String, enable: boolean, enableAnimation?: boolean): int;
    private native setDragKeyFramePolicy(nativeObj: long, keyFramePolicy: KeyFramePolicy): KeyFramePolicy;
    private native snapshot(nativeObj: long): image.PixelMap;
    private native snapshotSync(nativeObj: long): image.PixelMap;
    private native hideNonSystemFloatingWindows(nativeObj: long, shouldHide: boolean): void;
    private native opacity(nativeObj: long, opacity: double): void;
    private native scale(nativeObj: long, scaleOptions: ScaleOptions): void;
    private native translate(nativeObj: long, translateOptions: TranslateOptions): void;
    private native rotate(nativeObj: long, rotateOptions: RotateOptions): void;
    private native setShadow(nativeObj: long, radius: double, color?: string,
      offsetX?: double, offsetY?: double): void;
    private native setTouchableAreas(nativeObj: long, rects: Array<Rect>): void;
    private native setCornerRadiusSync(nativeObj: long, cornerRadius: double): void;
    private native getTransitionControllerSync(nativeObj: long): TransitionController;
    private native setBlur(nativeObj: long, radius: double): void;
    private native setBackdropBlurStyle(nativeObj: long, blurStyle: int): void;
    private native setBackdropBlur(nativeObj: long, radius: double): void;
    private native getWindowCornerRadius(nativeObj: long): double;
    private native setWindowCornerRadiusSync(nativeObj: long, cornerRadius: double): void;
    private native setWindowShadowRadius(nativeObj: long, radius: double): void;
    private native setWindowColorSpaceSync(nativeObj: long, colorSpace: int): void;
    private native setPreferredOrientationSync(nativeObj: long, orientation: int): void;
    private native getPreferredOrientation(nativeObj: long): int;
    private native convertOrientationAndRotation(nativeObj: long, from: int, to: int, value: int): int;
    private native setWindowPrivacyModeSync(nativeObj: long, isPrivacyMode: boolean): void;
    private native recoverSync(nativeObj: long): void;
    private native setUIContentSync(nativeObj: long, path: string): void;
    private native loadContentSync(nativeObj: long, path: string, storage?: LocalStorage): void;
    private native loadContentByNameSync(nativeObj: long, path: string, storage?: LocalStorage): void;
    private native setWindowKeepScreenOnSync(nativeObj: long, isKeepScreenOn: boolean): void;
    private native setWindowSystemBarEnableSync(nativeObj: long, names: Array<'status' | 'navigation'>): void;
    private native setWindowTouchableSync(nativeObj: long, isTouchable: boolean): void;
    private native setDialogBackGestureEnabledSync(nativeObj: long, enabled: boolean): void;
    private native setWindowMaskSync(nativeObj: long, windowMask: Array<Array<long>>): void;
    private native getUIContextSync(nativeObj: long): UIContext;
    private native getWindowAvoidAreaSync(nativeObj: long, type: int): AvoidArea;
    private native setWindowBrightness(nativeObj: long, brightness: double): void;
    private native getWindowColorSpace(nativeObj: long): int;
    private native setWakeUpScreen(nativeObj: long, wakeUp: boolean): void;
    private native setSnapshotSkip(nativeObj: long, isSkip: boolean): void;
    private native snapshotIgnorePrivacy(nativeObj: long): image.PixelMap;
    private native getStatusBarProperty(nativeObj: long): StatusBarProperty;
    private native setStatusBarColor(nativeObj: long, color: long): void;
    private native isSystemAvoidAreaEnabled(nativeObj: long): boolean;
    private native setSystemAvoidAreaEnabled(nativeObj: long, enabled: boolean): void;
    private native getWindowDensityInfo(nativeObj: long): WindowDensityInfo;
    private native getImmersiveModeEnabledState(nativeObj: long): boolean;
    private native setWindowGrayScale(nativeObj: long, grayScale: double): void;
    private native getWindowSystemBarProperties(nativeObj: long): SystemBarProperties;
    private native isGestureBackEnabled(nativeObj: long): boolean;
    private native setGestureBackEnabled(nativeObj: long, enabled: boolean): void;
    private native setSingleFrameComposerEnabled(nativeObj: long, enabled: boolean): void;
    private native getWindowAvoidAreaIgnoringVisibilitySync(nativeObj: long, type: int): AvoidArea;
    private native setWaterMarkFlagSync(nativeObj: long, enable: boolean): void;
    private native raiseMainWindowAboveTargetSync(nativeObj: long, windowId: int): void;
    private native setMainWindowRaiseByClickEnabledSync(nativeObj: long, enable: boolean): void;
    private native setWindowFocusableSync(nativeObj: long, isFocusable: boolean): void;
    private native getSubWindowZLevelSync(nativeObj: long): int;
    private native raiseAboveTargetSync(nativeObj: long, windowId: int): void;
    private native raiseToAppTopSync(nativeObj: long): void;
    private native setTopmostSync(nativeObj: long, isTopmost: boolean): void;
    private native setReceiveDragEventEnabled(nativeObj: long, enabled: boolean): void;
    private native isReceiveDragEventEnabled(nativeObj: long): boolean;
    private native setSeparationTouchEnabled(nativeObj: long, enabled: boolean): void;
    private native isSeparationTouchEnabled(nativeObj: long): boolean;
    private native requestFocusSync(nativeObj: long, isFocused: boolean): void;
    private native setSubWindowModal(nativeObj: long, isModal: boolean): void;
    private native setSubWindowModalType(nativeObj: long, isModal: boolean, modalityType: int): void;
    private native isFocusedSync(nativeObj: long): boolean;
    private native setSubWindowZLevelSync(nativeObj: long, zLevel: int): void;
    private native showWindowSync(nativeObj: long): void;
    private native showWindowWithOptions(nativeObj: long, options: ShowWindowOptions): void;
    private native destroyWindowSync(nativeObj: long): void;
    private native isWindowShowingSync(nativeObj: long): boolean;
    private native isWindowHighlightedSync(nativeObj: long): boolean;
    private native hideWithAnimationSync(nativeObj: long): void;
    private native showWithAnimationSync(nativeObj: long): void;
    private native getParentWindow(nativeObj: long): Window;
    private native setParentWindow(nativeObj: long, windowId: int): void;
    private native setContentAspectRatio(
      nativeObj: long, ratio: double, isPersistent: boolean, needUpdateRect: boolean): void;
    private native onSync(nativeObj: long, type: string, callback: object): void;
    private native onNoInteractionDetected(nativeObj: long, type: string, timeout: long, callback: Object): void;
    private native offSync(nativeObj: long, type: string, callback?: object): void;
    private native bindDialogTargetWithRemoteObjectSync(nativeObj: long, token: rpc.RemoteObject,
      deathCallback: object): void;
    private native bindDialogTargetWithRequestInfoSync(nativeObj: long, requestInfo: dialogRequest.RequestInfo,
      deathCallback: object): void;
    private native resizeAsync(nativeObj: long, width: int, height: int): void;
    private native setWindowLimits(nativeObj: long, windowLimits: WindowLimits, isForcible?: boolean): WindowLimits;
    private native getWindowLimits(nativeObj: long): WindowLimits;
    private native getWindowLimitsVP(nativeObj: long): WindowLimits;
    private native setAspectRatio(nativeObj: long, ratio: double): void;
    private native resetAspectRatio(nativeObj: long): void;
    private native getWindowStatus(nativeObj: long): int;
    private native minimize(nativeObj: long): void;
    private native maximize(nativeObj: long, presentation?: MaximizePresentation, acrossDisplay?: boolean): void;
    private native startMoving(nativeObj: long): void;
    private native startMoveWindowWithCoordinate(nativeObj: long, offsetX: int, offsetY: int): void;
    private native stopMoving(nativeObj: long): void;
    private native setResizeByDragEnabled(nativeObj: long, enable: boolean): void;
    private native enableDrag(nativeObj: long, enable: boolean): void;
    private native moveWindowToGlobal(nativeObj: long, x: int, y: int, moveConfiguration?: MoveConfiguration): void;
    private native moveWindowToAsync(nativeObj: long, x: int, y: int, moveConfiguration?: MoveConfiguration): void;
    private native setWindowMode(nativeObj: long, mode: WindowMode): void;
    private native setForbidSplitMove(nativeObj: long, isForbidSplitMove: boolean): void;
    private native setFollowParentWindowLayoutEnabled(nativeObj: long, enabled: boolean): void;
    private native setFollowParentMultiScreenPolicy(nativeObj: long, enabled: boolean): void;
    private native moveWindowToGlobalDisplay(nativeObj: long, x: int, y: int): void;
    private native clientToGlobalDisplay(nativeObj: long, winX: int, winY: int): Position;
    private native globalDisplayToClient(nativeObj: long, globalDisplayX: int, globalDisplayY: int): Position;
    private native setRotationLocked(nativeObj: long, locked: boolean): void;
    private native getRotationLocked(nativeObj: long): boolean;
    private native isInFreeWindowMode(nativeObj: long): boolean;
    private native setRelativePositionToParentWindowEnabled(nativeObj: long, enabled: boolean, anchor?: WindowAnchor,
      offsetX?: int, offsetY?: int): void;
    private native setWindowDelayRaiseOnDrag(nativeObj: long, isEnabled: boolean): void;
    private native setDefaultDensityEnabled(nativeObj: long, enabled: boolean): void;
    private native setWindowContainerColor(nativeObj: long, activeColor: string, inactiveColor: string): void;
    private native setWindowContainerModalColor(nativeObj: long, activeColor: string, inactiveColor: string): void;
    private native getWindowStateSnapshotSync(nativeObj: long): string;
    private native isMainWindowFullScreenAcrossDisplaysSync(nativeObj: long): boolean;
    private native setWindowShadowEnabledSync(nativeObj: long, enable: boolean): void;
    private native isImmersiveLayoutSync(nativeObj: long): boolean;
    private native keepKeyboardOnFocusSync(nativeObj: long, enable: boolean): void;

    private native setWindowTransitionAnimation(nativeObj: long, transitionType: WindowTransitionType,
        animation: TransitionAnimation): void;
    private native getWindowTransitionAnimationSync(nativeObj: long,
      transitionType: WindowTransitionType): TransitionAnimation | undefined;

    public hide(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void,
        reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.hideSync(this.nativeObj);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public hide(callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.hideSync(this.nativeObj);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public createSubWindowWithOptions(name: string, options: SubWindowOptions): Promise<Window> {
      return new Promise<Window>((resolve: (value: Window) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): Window => {
          return this.createSubWindowWithOptionsSync(this.nativeObj, name, options); 
        }).then((ret: Any) => {
          resolve(ret as Window);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public restore(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute((): void => {
          this.restore(this.nativeObj);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }  

    public restoreMainWindow(wantParameters?: Record<string, object>): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute((): void => {
          this.restoreMainWindow(this.nativeObj, wantParameters);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }  

    public static transferStatic(input: Any): Object {
      return WindowInternal.nativeTransferStatic(ESValue.wrap(input));
    }

    public static transferDynamic(input: Object): Any {
      return WindowInternal.nativeTransferDynamic((input as WindowInternal).nativeObj).unwrap();
    }

    public setWindowTopmost(isWindowTopmost: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowTopmost(this.nativeObj, isWindowTopmost);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }

    public resize(width: int, height: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ) :void => {
        taskpool.execute((): void => {
          this.resize(this.nativeObj, width, height);
        }).then((ret: Any) :void => {
            resolve(undefined);
        }).catch((err: Any) :void => {
            reject(err as BusinessError);
        });
      });
    }

    public moveWindowTo(x: int, y: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ) :void => {
        taskpool.execute((): void => {
          this.moveWindowTo(this.nativeObj, x, y);
        }).then((ret: Any) :void => {
            resolve(undefined);
        }).catch((err: Any) :void => {
            reject(err as BusinessError);
        });
      });
    }

    public resize(width: int, height: int, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.resize(this.nativeObj, width, height);
        }).then((ret: Any) => {
            callback(new BusinessError(), undefined);
        }).catch((err: Any) => {
            callback(err as BusinessError, undefined);
        });
    }

    public moveWindowTo(x: int, y: int, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.moveWindowTo(this.nativeObj, x, y);
        }).then((ret: Any) => {
            callback(new BusinessError(), undefined);
        }).catch((err: Any) => {
            callback(err as BusinessError, undefined);
        });
    }

    public getGlobalRect(): Rect {
      return this.getGlobalRect(this.nativeObj);
    }

    public setDecorButtonStyle(decorStyle: DecorButtonStyle): void {
      this.setDecorButtonStyle(this.nativeObj, decorStyle);
    }

    public setWindowTitleButtonVisible(isMaximizeButtonVisible: boolean, isMinimizeButtonVisible: boolean,
                                   isCloseButtonVisible?: boolean): void {
      let visibleParam: window.WindowTitleButtonVisibleParam = {
        isMaximizeButtonVisible: isMaximizeButtonVisible,
        isMinimizeButtonVisible: isMinimizeButtonVisible,
        isCloseButtonVisible: true
      };
      if (isCloseButtonVisible != undefined) {
        visibleParam.isCloseButtonVisible = isCloseButtonVisible;
      }
      this.setWindowTitleButtonVisible(this.nativeObj, visibleParam);
    }

    public setWindowTitle(titleName: string): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            try {
                this.setWindowTitle(this.nativeObj, titleName);
                resolve(undefined);
            } catch (err: Error) {
                reject(err as BusinessError);
            }
        });
    }

    public setTitleButtonVisible(isMaximizeVisible: boolean,
        isMinimizeVisible: boolean, isSplitVisible: boolean): void {
        this.setTitleButtonVisible(this.nativeObj, isMaximizeVisible, isMinimizeVisible, isSplitVisible, true);
    }

    public setTitleButtonVisible(isMaximizeVisible: boolean,
        isMinimizeVisible: boolean, isSplitVisible: boolean, isCloseVisible: boolean): void {
        this.setTitleButtonVisible(this.nativeObj, isMaximizeVisible, isMinimizeVisible, isSplitVisible, isCloseVisible);
    }

    public setWindowTitleMoveEnabled(enabled: boolean): void {
        this.setWindowTitleMoveEnabled(this.nativeObj, enabled);
    }

    public getTitleButtonRect(): TitleButtonRect {
        return this.getTitleButtonRect(this.nativeObj);
    }

    public getDecorButtonStyle(): DecorButtonStyle {
        return this.getDecorButtonStyle(this.nativeObj);
    }

    public setTitleAndDockHoverShown(isTitleHoverShown?: boolean, isDockHoverShown?: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.setTitleAndDockHoverShown(this.nativeObj, isTitleHoverShown, isDockHoverShown);
            }).then((ret: Any): void => {
                resolve(undefined);
            }).catch((err: Any): void => {
                reject(err as BusinessError);
            });
        });
    }

    public setHandwritingFlag(enable: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.setHandwritingFlag(this.nativeObj, enable);
            }).then((ret: Any): void => {
                resolve(undefined);
            }).catch((err: Any): void => {
                reject(err as BusinessError);
            });
        });
    }

    public getWindowDecorVisible(): boolean {
        return this.getWindowDecorVisible(this.nativeObj);
    }

    public onWindowWillClose(callback: WindowWillCloseCallback<void, Promise<boolean>>): void {
      this.onSync(this.nativeObj, 'windowWillClose', callback);
    }

    public offWindowWillClose(callback?: WindowWillCloseCallback<void, Promise<boolean>>): void {
      this.offSync(this.nativeObj, 'windowWillClose', callback);
    }

    public onWindowTitleButtonRectChange(callback: Callback<TitleButtonRect>): void {
      this.onSync(this.nativeObj, 'windowTitleButtonRectChange', callback);
    }

    public offWindowTitleButtonRectChange(callback?: Callback<TitleButtonRect>): void {
      this.offSync(this.nativeObj, 'windowTitleButtonRectChange', callback);
    }

    public getWindowDecorHeight(): int {
       let ret = this.getWindowDecorHeight(this.nativeObj);
       return ret;
    }

    public setWindowBackgroundColor(color: string | ColorMetrics): void {
      if (color instanceof string) {
        this.setWindowBackgroundColor(this.nativeObj, color as string);
      } else if (color instanceof ColorMetrics) {
        const alpha = color.alpha;
        const red = color.red;
        const green = color.green;
        const blue = color.blue;

        const toHex = (value: number): string => {
          return value.toString(16).padStart(2, '0').slice(-2);
        };

        let hexStr = '#';
        if (alpha !== 255) {
          hexStr += toHex(alpha);
        }
        hexStr += toHex(red) + toHex(green) + toHex(blue);
        this.setWindowBackgroundColor(this.nativeObj, hexStr);
      }
    }

    public setBackgroundColor(color: string): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ) :void => {
            taskpool.execute((): void => {
                this.setWindowBackgroundColor(this.nativeObj, color);
            }).then((ret: Any) :void => {
                resolve(undefined);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    public setBackgroundColor(color: string, callback: AsyncCallback<void>): void {
        taskpool.execute((): void => {
            this.setWindowBackgroundColor(this.nativeObj, color);
        }).then((ret: Any) => {
            callback(new BusinessError(), undefined);
        }).catch((err: Any) => {
            callback(err as BusinessError, undefined);
        });
    }

    public setImmersiveModeEnabledState(enable: boolean): void {
        this.setImmersiveModeEnabledState(this.nativeObj, enable);
    }

    public setWindowDecorVisible(isVisible: boolean): void {
        this.setWindowDecorVisible(this.nativeObj, isVisible);
    }

    public setWindowDecorHeight(height: int): void {
        this.setWindowDecorHeight(this.nativeObj, height);
    }

    public getWindowProperties(): WindowProperties {
        let windowProperties = this.getWindowProperties(this.nativeObj) as WindowPropertiesInternal;
        return windowProperties;
    }

    public getProperties(): Promise<WindowProperties> {
        return new Promise<WindowProperties>((resolve: (value: WindowProperties) => void,
            reject: (error: BusinessError) => void) => {
            taskpool.execute((): WindowProperties => {
                let res = this.getProperties(this.nativeObj) as WindowPropertiesInternal;
                return res;
            }).then((ret: Any) => {
                resolve(ret as WindowProperties);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    public getProperties(callback: AsyncCallback<WindowProperties>): void {
        taskpool.execute((): WindowProperties => {
            let res = this.getProperties(this.nativeObj) as WindowPropertiesInternal;
            return res;
        }).then((ret: Any) => {
            callback(new BusinessError(), ret as WindowProperties);
        }).catch((err: Any) => {
            callback(err as BusinessError, new WindowPropertiesInternal());
        });
    }

    public isWindowSupportWideGamut(): Promise<boolean> {
        return new Promise<boolean>((resolve: (value: boolean) => void, reject: (error: BusinessError) => void ) => {
            taskpool.execute((): boolean => {
                let res = this.isWindowSupportWideGamut(this.nativeObj);
                return res;
            }).then((ret: Any) => {
                resolve(ret as boolean);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    public isWindowSupportWideGamut(callback: AsyncCallback<boolean>): void {
        taskpool.execute((): boolean => {
            let res = this.isWindowSupportWideGamut(this.nativeObj);
            return res;
        }).then((ret: Any) => {
            callback(new BusinessError(), ret as boolean);
        }).catch((err: Any) => {
            callback(err as BusinessError, false);
        });
    }

    public setWindowLayoutFullScreen(isLayoutFullScreen: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) :void => {
            taskpool.execute((): void => {
                this.setWindowLayoutFullScreen(this.nativeObj, isLayoutFullScreen);
            }).then((ret: Any) :void => {
                resolve(undefined);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    public setWindowLayoutFullScreen(isLayoutFullScreen: boolean, callback: AsyncCallback<void>): void {
        taskpool.execute((): void => {
            this.setWindowLayoutFullScreen(this.nativeObj, isLayoutFullScreen);
        }).then((ret: Any) => {
            callback(new BusinessError(), undefined);
        }).catch((err: Any) => {
            callback(err as BusinessError, undefined);
        });
    }

    public setRaiseByClickEnabled(enable: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.setRaiseByClickEnabled(this.nativeObj, enable);
            }).then((ret: Any): void => {
                resolve(undefined);
            }).catch((err: Any): void => {
                reject(err as BusinessError);
            });
        });
    }

    public setRaiseByClickEnabled(enable: boolean, callback: AsyncCallback<void>): void {
        taskpool.execute((): void => {
            this.setRaiseByClickEnabled(this.nativeObj, enable);
        }).then((ret: Any) => {
            callback(new BusinessError(), undefined);
        }).catch((err: Any) => {
            callback(err as BusinessError, undefined);
        });
    }

    public setExclusivelyHighlighted(exclusivelyHighlighted: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.setExclusivelyHighlighted(this.nativeObj, exclusivelyHighlighted);
            }).then((ret: Any): void => {
                resolve(undefined);
            }).catch((err: Any): void => {
                reject(err as BusinessError);
            });
        });
    }

    public setWindowSystemBarProperties(systemBarProperties: SystemBarProperties): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) :void => {
            taskpool.execute((): void => {
                this.setWindowSystemBarProperties(this.nativeObj, systemBarProperties);
            }).then((ret: Any) :void => {
                resolve(undefined);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    public setWindowSystemBarProperties(systemBarProperties: SystemBarProperties, callback: AsyncCallback<void>): void {
        taskpool.execute((): void => {
            this.setWindowSystemBarProperties(this.nativeObj, systemBarProperties);
        }).then((ret: Any) => {
            callback(new BusinessError(), undefined);
        }).catch((err: Any) => {
            callback(err as BusinessError, undefined);
        });
    }

    public setSpecificSystemBarEnabled(name: String, enable: boolean, enableAnimation?: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) :void => {
            taskpool.execute((): void => {
              this.setSpecificSystemBarEnabled(this.nativeObj, name, enable, enableAnimation);
            }).then((ret: Any) :void => {
                resolve(undefined);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    public setDragKeyFramePolicy(keyFramePolicy: KeyFramePolicy): Promise<KeyFramePolicy> {
      if ((keyFramePolicy.interval ?? 1) <= 0 || (keyFramePolicy.distance ?? 1) < 0 ||
          (keyFramePolicy.animationDelay ?? 1) < 0 || (keyFramePolicy.animationDuration ?? 1) < 0) {
        throw new BusinessError(WM_ERROR_ILLEGAL_PARAM, new Error('Static invalid param!'));
      }
      return new Promise<KeyFramePolicy>((resolve: (value: KeyFramePolicy) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute(() => {
          return this.setDragKeyFramePolicy(this.nativeObj, keyFramePolicy);
        }).then((ret: Any) => {
          resolve(ret as KeyFramePolicy);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public snapshot(): Promise<image.PixelMap> {
      return new Promise<image.PixelMap>((resolve: (value: image.PixelMap) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute(() => {
          return this.snapshot(this.nativeObj);
        }).then((ret: Any) => {
          resolve(ret as image.PixelMap);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public snapshot(callback: AsyncCallback<image.PixelMap | undefined>): void {
      taskpool.execute(() => {
        return this.snapshot(this.nativeObj);
      }).then((ret: Any) => {
        callback(new BusinessError(), ret as image.PixelMap);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public snapshotSync(): image.PixelMap {
      return this.snapshotSync(this.nativeObj);
    }

    public hideNonSystemFloatingWindows(shouldHide: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute(() => {
          this.hideNonSystemFloatingWindows(this.nativeObj, shouldHide);
        }).then(() => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public hideNonSystemFloatingWindows(shouldHide: boolean, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.hideNonSystemFloatingWindows(this.nativeObj, shouldHide);
      }).then(() => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public opacity(opacity: double): void {
      this.opacity(this.nativeObj, opacity);
    }

    public scale(scaleOptions: ScaleOptions): void {
      this.scale(this.nativeObj, scaleOptions);
    }

    public translate(translateOptions: TranslateOptions): void {
      this.translate(this.nativeObj, translateOptions);
    }

    public rotate(rotateOptions: RotateOptions): void {
      this.rotate(this.nativeObj, rotateOptions);
    }

    public setShadow(radius: double, color?: string, offsetX?: double, offsetY?: double): void {
      this.setShadow(this.nativeObj, radius, color, offsetX, offsetY);
    }

    public setTouchableAreas(rects: Array<Rect>): void {
      this.setTouchableAreas(this.nativeObj, rects);
    }

    public setBlur(radius: double): void {
      this.setBlur(this.nativeObj, radius);
    }

    public setBackdropBlurStyle(blurStyle: BlurStyle): void {
      this.setBackdropBlurStyle(this.nativeObj, blurStyle);
    }

    public setBackdropBlur(radius: double): void {
      this.setBackdropBlur(this.nativeObj, radius);
    }

    public getWindowCornerRadius(): double {
      return this.getWindowCornerRadius(this.nativeObj);
    }

    public setWindowCornerRadius(cornerRadius: double): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowCornerRadiusSync(this.nativeObj, cornerRadius);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWindowShadowRadius(radius: double): void {
      this.setWindowShadowRadius(this.nativeObj, radius);
    }

    public setWindowColorSpace(colorSpace: ColorSpace): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowColorSpaceSync(this.nativeObj, colorSpace as int);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWindowColorSpace(colorSpace: ColorSpace, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.setWindowColorSpaceSync(this.nativeObj, colorSpace as int);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public setPreferredOrientation(orientation: Orientation): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setPreferredOrientationSync(this.nativeObj, orientation as int);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setPreferredOrientation(orientation: Orientation, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.setPreferredOrientationSync(this.nativeObj, orientation as int);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public getPreferredOrientation(): Orientation {
      return this.getPreferredOrientation(this.nativeObj) as Orientation;
    }

    public convertOrientationAndRotation(from: RotationInfoType, to: RotationInfoType, value: int): int {
      return this.convertOrientationAndRotation(this.nativeObj, from as int, to as int, value);
    }

    public setWindowPrivacyMode(isPrivacyMode: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowPrivacyModeSync(this.nativeObj, isPrivacyMode);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWindowPrivacyMode(isPrivacyMode: boolean, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.setWindowPrivacyModeSync(this.nativeObj, isPrivacyMode);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public recover(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.recoverSync(this.nativeObj);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setUIContent(path: string): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        try {
          this.setUIContentSync(this.nativeObj, path);
          resolve(undefined);
        } catch (err: Error) {
          reject(err as BusinessError);
        }
      });
    }

    public setUIContent(path: string, callback: AsyncCallback<void>): void {
      try {
        this.setUIContentSync(this.nativeObj, path);
        callback(new BusinessError(), undefined);
      } catch (err: Error) {
        callback(err as BusinessError, undefined);
      }
    }

    public loadContent(path: string, storage: LocalStorage): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        try {
          this.loadContentSync(this.nativeObj, path, storage);
          resolve(undefined);
        } catch (err: Error) {
          reject(err as BusinessError);
        }
      });
    }

    public loadContent(path: string, storage: LocalStorage, callback: AsyncCallback<void>): void {
      try {
        this.loadContentSync(this.nativeObj, path, storage);
        callback(new BusinessError(), undefined);
      } catch (err: Error) {
        callback(err as BusinessError, undefined);
      }
    }

    public loadContent(path: string): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        try {
          this.loadContentSync(this.nativeObj, path, undefined);
          resolve(undefined);
        } catch (err: Error) {
          reject(err as BusinessError);
        }
      });
    }

    public loadContent(path: string, callback: AsyncCallback<void>): void {
      try {
        this.loadContentSync(this.nativeObj, path, undefined);
        callback(new BusinessError(), undefined);
      } catch (err: Error) {
        callback(err as BusinessError, undefined);
      }
    }

    public loadContentByName(name: string, storage: LocalStorage, callback: AsyncCallback<void>): void {
      try {
        this.loadContentByNameSync(this.nativeObj, name, storage);
        callback(new BusinessError(), undefined);
      } catch (err: Error) {
        callback(err as BusinessError, undefined);
      }
    }

    public loadContentByName(name: string, storage?: LocalStorage): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        try {
          this.loadContentByNameSync(this.nativeObj, name, storage);
          resolve(undefined);
        } catch (err: Error) {
          reject(err as BusinessError);
        }
      });
    }

    public loadContentByName(name: string, callback: AsyncCallback<void>): void {
      try {
        this.loadContentByNameSync(this.nativeObj, name, undefined);
        callback(new BusinessError(), undefined);
      } catch (err: Error) {
        callback(err as BusinessError, undefined);
      }
    }

    public onAvoidAreaChange(callback: Callback<AvoidAreaOptions>): void {
      this.onSync(this.nativeObj, 'avoidAreaChange', callback);
    }

    public offAvoidAreaChange(callback?: Callback<AvoidAreaOptions>): void {
      this.offSync(this.nativeObj, 'avoidAreaChange', callback);
    }

    public onDisplayIdChange(callback: Callback<long>): void {
      this.onSync(this.nativeObj, 'displayIdChange', callback);
    }

    public offDisplayIdChange(callback?: Callback<long>): void {
      this.offSync(this.nativeObj, 'displayIdChange', callback);
    }

    public onWindowVisibilityChange(callback: Callback<boolean>) {
      this.onSync(this.nativeObj, 'windowVisibilityChange', callback);
    }

    public offWindowVisibilityChange(callback?: Callback<boolean>): void {
      this.offSync(this.nativeObj, 'windowVisibilityChange', callback);
    }

    public onSystemDensityChange(callback: Callback<double>): void {
      this.onSync(this.nativeObj, 'systemDensityChange', callback);
    }

    public offSystemDensityChange(callback?: Callback<double>): void {
      this.offSync(this.nativeObj, 'systemDensityChange', callback);
    }

    public onScreenshot(callback: Callback<void>): void {
      this.onSync(this.nativeObj, 'screenshot', callback);
    }

    public offScreenshot(callback?: Callback<void>): void {
      this.offSync(this.nativeObj, 'screenshot', callback);
    }

    public setWindowKeepScreenOn(isKeepScreenOn: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) :void => {
            taskpool.execute((): void => {
                this.setWindowKeepScreenOnSync(this.nativeObj, isKeepScreenOn);
            }).then((ret: Any): void => {
                resolve(ret as undefined);
            }).catch((err: Any): void => {
                reject(err as BusinessError);
            });
        });
    }

    public setWindowKeepScreenOn(isKeepScreenOn: boolean, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.setWindowKeepScreenOnSync(this.nativeObj, isKeepScreenOn);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public setWindowSystemBarEnable(names: Array<'status' | 'navigation'>): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ) :void => {
            taskpool.execute((): void => {
                this.setWindowSystemBarEnableSync(this.nativeObj, names);
            }).then((ret: Any) => {
                resolve(undefined);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    public setWindowSystemBarEnable(names: Array<'status' | 'navigation'>, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.setWindowSystemBarEnableSync(this.nativeObj, names);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public setWindowTouchable(isTouchable: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowTouchableSync(this.nativeObj, isTouchable);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWindowTouchable(isTouchable: boolean, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.setWindowTouchableSync(this.nativeObj, isTouchable);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public setDialogBackGestureEnabled(enabled: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
         taskpool.execute((): void => {
           this.setDialogBackGestureEnabledSync(this.nativeObj, enabled);
         }).then((ret: Any) => {
           resolve(undefined);
         }).catch((err: Any) => {
           reject(err as BusinessError);
         });
       });
    }

    public setWindowMask(windowMask: Array<Array<long>>): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowMaskSync(this.nativeObj, windowMask);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getUIContext(): UIContext {
      return this.getUIContextSync(this.nativeObj);
    }

    public getWindowAvoidArea(type: AvoidAreaType): AvoidArea {
      return this.getWindowAvoidAreaSync(this.nativeObj, type as AvoidAreaType);
    }

    public setWindowBrightness(brightness: double): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowBrightness(this.nativeObj, brightness);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWindowBrightness(brightness: double, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.setWindowBrightness(this.nativeObj, brightness);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public getWindowColorSpace(): ColorSpace {
      return this.getWindowColorSpace(this.nativeObj) as ColorSpace;
    }

    public setWakeUpScreen(wakeUp: boolean): void {
      return this.setWakeUpScreen(this.nativeObj, wakeUp);
    }

    public setSnapshotSkip(isSkip: boolean): void {
      return this.setSnapshotSkip(this.nativeObj, isSkip);
    }

    public snapshotIgnorePrivacy(): Promise<image.PixelMap> {
      return new Promise<image.PixelMap>((resolve: (value: image.PixelMap) => void,
        reject: (error: BusinessError) => void): void => {
        taskpool.execute(() => {
          return this.snapshotIgnorePrivacy(this.nativeObj);
        }).then((ret: Any) => {
          resolve(ret as image.PixelMap);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getStatusBarProperty(): StatusBarProperty {
      return this.getStatusBarProperty(this.nativeObj);
    }

    public setStatusBarColor(color: ColorMetrics): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setStatusBarColor(
            this.nativeObj, (color.alpha << 24) | (color.red << 16) | (color.green << 8) | color.blue);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public isSystemAvoidAreaEnabled(): boolean {
      return this.isSystemAvoidAreaEnabled(this.nativeObj);
    }

    public setSystemAvoidAreaEnabled(enabled: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setSystemAvoidAreaEnabled(this.nativeObj, enabled);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getWindowDensityInfo(): WindowDensityInfo {
      return this.getWindowDensityInfo(this.nativeObj) as WindowDensityInfoInternal;
    }

    public getImmersiveModeEnabledState(): boolean {
      return this.getImmersiveModeEnabledState(this.nativeObj);
    }

    public setWindowGrayScale(grayScale: double): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowGrayScale(this.nativeObj, grayScale);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getWindowSystemBarProperties(): SystemBarProperties {
      return this.getWindowSystemBarProperties(this.nativeObj);
    }

    public isGestureBackEnabled(): boolean {
      return this.isGestureBackEnabled(this.nativeObj);
    }

    public setGestureBackEnabled(enabled: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setGestureBackEnabled(this.nativeObj, enabled);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setSingleFrameComposerEnabled(enabled: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setSingleFrameComposerEnabled(this.nativeObj, enabled);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getWindowAvoidAreaIgnoringVisibility(type: AvoidAreaType): AvoidArea {
      return this.getWindowAvoidAreaIgnoringVisibilitySync(this.nativeObj, type as AvoidAreaType);
    }

    public setWaterMarkFlag(enable: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWaterMarkFlagSync(this.nativeObj, enable);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWaterMarkFlag(enable: boolean, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.setWaterMarkFlagSync(this.nativeObj, enable);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public raiseMainWindowAboveTarget(windowId: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.raiseMainWindowAboveTargetSync(this.nativeObj, windowId);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setMainWindowRaiseByClickEnabled(enable: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setMainWindowRaiseByClickEnabledSync(this.nativeObj, enable);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWindowFocusable(isFocusable: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowFocusableSync(this.nativeObj, isFocusable);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWindowFocusable(isFocusable: boolean, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.setWindowFocusableSync(this.nativeObj, isFocusable);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public getSubWindowZLevel(): int {
      let ret = this.getSubWindowZLevelSync(this.nativeObj);
      return ret;
    };

    public isFocused(): boolean {
      return this.isFocusedSync(this.nativeObj);
    };

    public setSubWindowZLevel(zLevel: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setSubWindowZLevelSync(this.nativeObj, zLevel);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public raiseAboveTarget(windowId: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.raiseAboveTargetSync(this.nativeObj, windowId);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }

    public raiseAboveTarget(windowId: int, callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.raiseAboveTargetSync(this.nativeObj, windowId);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public raiseToAppTop(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.raiseToAppTopSync(this.nativeObj);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }

    public raiseToAppTop(callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.raiseToAppTopSync(this.nativeObj);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public setTopmost(isTopmost: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setTopmostSync(this.nativeObj, isTopmost);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }

    public setReceiveDragEventEnabled(enabled: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setReceiveDragEventEnabled(this.nativeObj, enabled);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }

    public isReceiveDragEventEnabled(): boolean{
      return this.isReceiveDragEventEnabled(this.nativeObj);
    }

    public setSeparationTouchEnabled(enabled: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setSeparationTouchEnabled(this.nativeObj, enabled);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }

    public isSeparationTouchEnabled(): boolean{
      return this.isSeparationTouchEnabled(this.nativeObj);
    }

    public requestFocus(isFocused: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.requestFocusSync(this.nativeObj, isFocused);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }

    public setSubWindowModal(isModal: boolean): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.setSubWindowModal(this.nativeObj, isModal);
            }).then((ret: Any): void => {
                resolve(undefined);
            }).catch((err: Any): void => {
                reject(err as BusinessError);
            });
        });
    }

    public setSubWindowModal(isModal: boolean, modalityType: ModalityType): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.setSubWindowModalType(this.nativeObj, isModal, modalityType);
            }).then((ret: Any): void => {
                resolve(undefined);
            }).catch((err: Any): void => {
                reject(err as BusinessError);
            });
        });
    }

    public setContentAspectRatio(ratio: double, isPersistent?: boolean, needUpdateRect?: boolean): Promise<void> {
      return new Promise<void>(
        (resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
          taskpool.execute((): void => {
              this.setContentAspectRatio(this.nativeObj, ratio, isPersistent ?? true, needUpdateRect ?? true);
          }).then((ret: Any) => {
              resolve(undefined);
          }).catch((err: Any) => {
              reject(err as BusinessError);
          });
      });
    }

    public setCornerRadius(cornerRadius: double): void {
      this.setCornerRadiusSync(this.nativeObj, cornerRadius);
    }

    public getTransitionController(): TransitionController {
      return this.getTransitionControllerSync(this.nativeObj);
    }

    public on<T>(type: string, callback: Callback<T>): void {
      this.onSync(this.nativeObj, type, callback);
    }

    public on<T>(type: 'noInteractionDetected', timeout: long, callback: Callback<T>): void {
      this.onNoInteractionDetected(this.nativeObj, type, timeout, callback as WindowVoidCallback);
    }

    public onTouchOutside(callback: Callback<void>): void {
      this.onSync(this.nativeObj, 'touchOutside', callback);
    }

    public onDialogTargetTouch(callback: Callback<void>): void {
      this.onSync(this.nativeObj, 'dialogTargetTouch', callback);
    }

    public onNoInteractionDetected(timeout: long, callback: Callback<void>): void {
      this.onNoInteractionDetected(this.nativeObj, 'noInteractionDetected', timeout, callback as WindowVoidCallback);
    }
    
    public onKeyboardHeightChange(callback: Callback<int>): void {
      this.onSync(this.nativeObj, "keyboardHeightChange", callback);
    }

    public onKeyboardWillShow(callback: Callback<KeyboardInfo>): void {
      this.onSync(this.nativeObj, 'keyboardWillShow', callback);
    }

    public onKeyboardWillHide(callback: Callback<KeyboardInfo>): void {
      this.onSync(this.nativeObj, 'keyboardWillHide', callback);
    }

    public onKeyboardDidShow(callback: Callback<KeyboardInfo>): void {
      this.onSync(this.nativeObj, "keyboardDidShow", callback);
    }

    public onKeyboardDidHide(callback: Callback<KeyboardInfo>): void {
      this.onSync(this.nativeObj, "keyboardDidHide", callback);
    }

    public onUiExtensionSecureLimitChange(callback: Callback<boolean>): void {
      this.onSync(this.nativeObj, 'uiExtensionSecureLimitChange', callback);
    }

    public onRotationChange(callback: RotationChangeCallback<RotationChangeInfo,
      RotationChangeResult | undefined>): void {
      this.onSync(this.nativeObj, "rotationChange", callback);
    }

    public off<T>(type: string, callback?: Callback<T>): void {
      this.offSync(this.nativeObj, type, callback);
    }

    public offTouchOutside(callback?: Callback<void>): void {
      this.offSync(this.nativeObj, 'touchOutside', callback);
    }

    public offDialogTargetTouch(callback?: Callback<void>): void {
      this.offSync(this.nativeObj, 'dialogTargetTouch', callback);
    }

    public offNoInteractionDetected(callback?: Callback<void>): void {
      this.offSync(this.nativeObj, 'noInteractionDetected', callback);
    }
    
    public offKeyboardHeightChange(callback?: Callback<int>): void {
      this.offSync(this.nativeObj, "keyboardHeightChange", callback);
    }

    public offKeyboardWillShow(callback?: Callback<KeyboardInfo>): void {
      this.offSync(this.nativeObj, 'keyboardWillShow', callback);
    }

    public offKeyboardWillHide(callback?: Callback<KeyboardInfo>): void {
      this.offSync(this.nativeObj, 'keyboardWillHide', callback);
    }

    public offKeyboardDidShow(callback?: Callback<KeyboardInfo>): void {
      this.offSync(this.nativeObj, "keyboardDidShow", callback);
    }

    public offKeyboardDidHide(callback?: Callback<KeyboardInfo>): void {
      this.offSync(this.nativeObj, "keyboardDidHide", callback);
    }

    public offUiExtensionSecureLimitChange(callback?: Callback<boolean>): void {
      this.offSync(this.nativeObj, 'uiExtensionSecureLimitChange', callback);
    }

    public onOcclusionStateChanged(callback: Callback<OcclusionState>): void {
      this.onSync(this.nativeObj, 'occlusionStateChanged', callback);
    }

    public offOcclusionStateChanged(callback?: Callback<OcclusionState>): void {
      this.offSync(this.nativeObj, 'occlusionStateChanged', callback);
    }

    public onFrameMetricsMeasured(callback: Callback<FrameMetrics>): void {
      this.onSync(this.nativeObj, 'frameMetricsMeasured', callback);
    }

    public offFrameMetricsMeasured(callback: Callback<FrameMetrics>): void {
      this.offSync(this.nativeObj, 'frameMetricsMeasured', callback);
    }

    public offRotationChange(callback?: RotationChangeCallback<RotationChangeInfo,
      RotationChangeResult | undefined>): void {
      this.offSync(this.nativeObj, "rotationChange", callback);
    }

    public setDefaultDensityEnabled(enabled: boolean): void {
      this.setDefaultDensityEnabled(this.nativeObj, enabled);
    }

    public setWindowContainerColor(activeColor: string, inactiveColor: string): void {
      this.setWindowContainerColor(this.nativeObj, activeColor, inactiveColor);
    }

    public setWindowContainerModalColor(activeColor: string, inactiveColor: string): void {
      this.setWindowContainerModalColor(this.nativeObj, activeColor, inactiveColor);
    }

    public onMainWindowFullScreenAcrossDisplaysChanged(callback: Callback<boolean>): void {
      this.onSync(this.nativeObj, 'mainWindowFullScreenAcrossDisplaysChanged', callback);
    }

    public offMainWindowFullScreenAcrossDisplaysChanged(callback?: Callback<boolean>): void {
      this.offSync(this.nativeObj, 'mainWindowFullScreenAcrossDisplaysChanged', callback);
    }

    public onScreenshotAppEvent(callback: Callback<ScreenshotEventType>): void {
      this.onSync(this.nativeObj, 'screenshotAppEvent', callback);
    }

    public offScreenshotAppEvent(callback?: Callback<ScreenshotEventType>): void {
      this.offSync(this.nativeObj, 'screenshotAppEvent', callback);
    }

    public onWindowHighlightChange(callback: Callback<boolean>): void {
      this.onSync(this.nativeObj, "windowHighlightChange", callback);
    }

    public offWindowHighlightChange(callback?: Callback<boolean>): void {
      this.offSync(this.nativeObj, "windowHighlightChange", callback);
    }

    public setWindowShadowEnabled(enable: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowShadowEnabledSync(this.nativeObj, enable);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public isMainWindowFullScreenAcrossDisplays(): Promise<boolean> {
      return new Promise<boolean>((resolve: (value: boolean) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): boolean => {
          return this.isMainWindowFullScreenAcrossDisplaysSync(this.nativeObj);
        }).then((ret: Any) => {
          resolve(ret as boolean);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getWindowStateSnapshot(): Promise<string> {
      return new Promise<string>((resolve: (value: string) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): string => {
          return this.getWindowStateSnapshotSync(this.nativeObj);
        }).then((ret: Any) => {
          resolve(ret as string);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public isImmersiveLayout(): boolean {
      return this.isImmersiveLayoutSync(this.nativeObj);
    }

    public onWindowSizeChange(callback: Callback<Size>): void {
      this.onSync(this.nativeObj, 'windowSizeChange', callback);
    }

    public offWindowSizeChange(callback?: Callback<Size>): void {
      this.offSync(this.nativeObj, 'windowSizeChange', callback);
    }

    public onWindowStatusChange(callback: Callback<WindowStatusType>): void {
      this.onSync(this.nativeObj, 'windowStatusChange', callback);
    }

    public offWindowStatusChange(callback?: Callback<WindowStatusType>): void {
      this.offSync(this.nativeObj, 'windowStatusChange', callback);
    }

    public onWindowRectChange(callback: Callback<RectChangeOptions>): void {
      this.onSync(this.nativeObj, 'windowRectChange', callback);
    }

    public offWindowRectChange(callback?: Callback<RectChangeOptions>): void {
      this.offSync(this.nativeObj, 'windowRectChange', callback);
    }

    public onWindowStatusDidChange(callback: Callback<WindowStatusType>): void {
      this.onSync(this.nativeObj, 'windowStatusDidChange', callback);
    }

    public offWindowStatusDidChange(callback?: Callback<WindowStatusType>): void {
      this.offSync(this.nativeObj, 'windowStatusDidChange', callback);
    }

    public onRectChangeInGlobalDisplay(callback: Callback<RectChangeOptions>): void {
      this.onSync(this.nativeObj, 'rectChangeInGlobalDisplay', callback);
    }

    public offRectChangeInGlobalDisplay(callback?: Callback<RectChangeOptions>): void {
      this.offSync(this.nativeObj, 'rectChangeInGlobalDisplay', callback);
    }

    public showWindow(callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.showWindowSync(this.nativeObj);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public showWindow(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.showWindowSync(this.nativeObj);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public showWindow(options: ShowWindowOptions): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.showWindowWithOptions(this.nativeObj, options);
        }).then((ret: Any): void => {
          resolve(undefined);
        }).catch((err: Any): void => {
          reject(err as BusinessError);
        });
      });
    }

    public destroyWindow(callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.destroyWindowSync(this.nativeObj);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public destroyWindow(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.destroyWindowSync(this.nativeObj);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public isWindowShowing(): boolean {
      return this.isWindowShowingSync(this.nativeObj);
    }

    public isWindowHighlighted(): boolean {
      return this.isWindowHighlightedSync(this.nativeObj);
    }

    public hideWithAnimation(callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.hideWithAnimationSync(this.nativeObj);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public hideWithAnimation(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.hideWithAnimationSync(this.nativeObj);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public showWithAnimation(callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.showWithAnimationSync(this.nativeObj);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public showWithAnimation(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.showWithAnimationSync(this.nativeObj);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getParentWindow(): Window {
      return this.getParentWindow(this.nativeObj);
    }

    public setParentWindow(windowId: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setParentWindow(this.nativeObj, windowId);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public bindDialogTarget(token: rpc.RemoteObject, deathCallback: Callback<void>): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.bindDialogTargetWithRemoteObjectSync(this.nativeObj, token, deathCallback);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public bindDialogTarget(token: rpc.RemoteObject, deathCallback: Callback<void>, 
      callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.bindDialogTargetWithRemoteObjectSync(this.nativeObj, token, deathCallback);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public bindDialogTarget(requestInfo: dialogRequest.RequestInfo, deathCallback: Callback<void>): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.bindDialogTargetWithRequestInfoSync(this.nativeObj, requestInfo, deathCallback);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public bindDialogTarget(requestInfo: dialogRequest.RequestInfo, deathCallback: Callback<void>, 
      callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
        this.bindDialogTargetWithRequestInfoSync(this.nativeObj, requestInfo, deathCallback);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public resizeAsync(width: int, height: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.resizeAsync(this.nativeObj, width, height);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWindowLimits(windowLimits: WindowLimits): Promise<WindowLimits> {
      return new Promise<WindowLimits>((resolve: (value: WindowLimits) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute(() => {
          return this.setWindowLimits(this.nativeObj, windowLimits);
        }).then((ret: Any) => {
          resolve(ret as WindowLimits);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWindowLimits(windowLimits: WindowLimits, isForcible: boolean): Promise<WindowLimits> {
      return new Promise<WindowLimits>((resolve: (value: WindowLimits) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute(() => {
          return this.setWindowLimits(this.nativeObj, windowLimits, isForcible);
        }).then((ret: Any) => {
          resolve(ret as WindowLimits);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getWindowLimits(): WindowLimits {
      return this.getWindowLimits(this.nativeObj);
    }

    public getWindowLimitsVP(): WindowLimits {
      return this.getWindowLimitsVP(this.nativeObj);
    }

    public setAspectRatio(ratio: double, callback: AsyncCallback<void>) {
      taskpool.execute(() => {
        this.setAspectRatio(this.nativeObj, ratio);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public setAspectRatio(ratio: double): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.setAspectRatio(this.nativeObj, ratio);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public resetAspectRatio(callback: AsyncCallback<void>) {
      taskpool.execute(() => {
        this.resetAspectRatio(this.nativeObj);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public resetAspectRatio(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.resetAspectRatio(this.nativeObj);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getWindowStatus(): WindowStatusType {
      return this.getWindowStatus(this.nativeObj) as WindowStatusType;
    }

    public minimize(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute((): void => {
          this.minimize(this.nativeObj);
        }).then((ret: Any): void => {
            resolve(undefined);
        }).catch((err: Any): void => {
            reject(err as BusinessError);
        });
      });
    }

    public minimize(callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
          this.minimize(this.nativeObj);
      }).then((ret: Any) => {
          callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
          callback(err as BusinessError, undefined);
      });
    }

    public maximize(presentation?: MaximizePresentation): Promise<void> {
      return this.maximize(presentation, undefined);
    }

    public maximize(presentation?: MaximizePresentation, acrossDisplay?: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.maximize(this.nativeObj, presentation, acrossDisplay);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public startMoving(): Promise<void> {
      this.throwIfWindowInvalid(this.nativeObj);
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute((): void => {
          this.startMoving(this.nativeObj);
        }).then((ret: Any): void => {
            resolve(undefined);
        }).catch((err: Any): void => {
            reject(err as BusinessError);
        });
      });
    }

    public startMoving(offsetX: int, offsetY: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute((): void => {
          this.startMoveWindowWithCoordinate(this.nativeObj, offsetX, offsetY);
        }).then((ret: Any): void => {
            resolve(undefined);
        }).catch((err: Any): void => {
            reject(err as BusinessError);
        });
      });
    }

    public stopMoving(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute(() => {
          this.stopMoving(this.nativeObj);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setResizeByDragEnabled(enable: boolean, callback: AsyncCallback<void>) {
      taskpool.execute(() => {
        this.setResizeByDragEnabled(this.nativeObj, enable);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public setResizeByDragEnabled(enable: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.setResizeByDragEnabled(this.nativeObj, enable);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public enableDrag(enable: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.enableDrag(this.nativeObj, enable);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public moveWindowToGlobal(x: int, y: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.moveWindowToGlobal(this.nativeObj, x, y);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public moveWindowToGlobal(x: int, y: int, moveConfiguration?: MoveConfiguration): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.moveWindowToGlobal(this.nativeObj, x, y, moveConfiguration);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public moveWindowToAsync(x: int, y: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.moveWindowToAsync(this.nativeObj, x, y);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public moveWindowToAsync(x: int, y: int, moveConfiguration?: MoveConfiguration): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.moveWindowToAsync(this.nativeObj, x, y, moveConfiguration);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setWindowMode(mode: WindowMode, callback: AsyncCallback<void>) {
      taskpool.execute(() => {
        this.setWindowMode(this.nativeObj, mode);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public setWindowMode(mode: WindowMode): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.setWindowMode(this.nativeObj, mode);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setForbidSplitMove(isForbidSplitMove: boolean, callback: AsyncCallback<void>) {
      taskpool.execute(() => {
        this.setForbidSplitMove(this.nativeObj, isForbidSplitMove);
      }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
      });
    }

    public setForbidSplitMove(isForbidSplitMove: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.setForbidSplitMove(this.nativeObj, isForbidSplitMove);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setFollowParentWindowLayoutEnabled(enabled: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.setFollowParentWindowLayoutEnabled(this.nativeObj, enabled);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public setFollowParentMultiScreenPolicy(enabled: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.setFollowParentMultiScreenPolicy(this.nativeObj, enabled);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public moveWindowToGlobalDisplay(x: int, y: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
        taskpool.execute(() => {
          this.moveWindowToGlobalDisplay(this.nativeObj, x, y);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public clientToGlobalDisplay(winX: int, winY: int): Position {
      return this.clientToGlobalDisplay(this.nativeObj, winX, winY);
    }

    public globalDisplayToClient(globalDisplayX: int, globalDisplayY: int): Position {
      return this.globalDisplayToClient(this.nativeObj, globalDisplayX, globalDisplayY);
    }

    public setRotationLocked(locked: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setRotationLocked(this.nativeObj, locked);
          }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getRotationLocked(): boolean {
      return this.getRotationLocked(this.nativeObj);
    }

    public isInFreeWindowMode(): boolean {
      return this.isInFreeWindowMode(this.nativeObj);
    }

    public onFreeWindowModeChange(callback: Callback<boolean>): void {
      let type = "freeWindowModeChange";
      this.onSync(this.nativeObj, type, callback);
    }

    public offFreeWindowModeChange(callback?: Callback<boolean>): void {
      let type = "freeWindowModeChange";
      this.offSync(this.nativeObj, type, callback);
    }

    public setRelativePositionToParentWindowEnabled(enabled: boolean, anchor?: WindowAnchor,
      offsetX?: int, offsetY?: int): Promise<void> {
      return new Promise<void>(
        (resolve: (value: undefined) => void, reject: (error: BusinessError) => void ): void => {
          taskpool.execute((): void => {
            this.setRelativePositionToParentWindowEnabled(this.nativeObj, enabled, anchor, offsetX, offsetY);
          }).then((ret: Any) => {
            resolve(undefined);
          }).catch((err: Any) => {
            reject(err as BusinessError);
          });
      });
    }

    public setWindowDelayRaiseOnDrag(isEnabled: boolean): void {
      this.setWindowDelayRaiseOnDrag(this.nativeObj, isEnabled);
    }

    public setWindowTransitionAnimation(transitionType: WindowTransitionType,
      animation: TransitionAnimation): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute(() => {
          this.setWindowTransitionAnimation(this.nativeObj, transitionType, animation);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public getWindowTransitionAnimation(transitionType: WindowTransitionType): TransitionAnimation | undefined {
      return this.getWindowTransitionAnimationSync(this.nativeObj, transitionType);
    }

    public keepKeyboardOnFocus(keepKeyboardFlag: boolean): void {
      this.keepKeyboardOnFocusSync(this.nativeObj, keepKeyboardFlag);
    }

    public onWindowEvent(callback: Callback<WindowEventType>): void {
      this.onSync(this.nativeObj, 'windowEvent', callback);
    }

    public offWindowEvent(callback?: Callback<WindowEventType>): void {
      this.offSync(this.nativeObj, 'windowEvent', callback);
    }

    public onSubWindowClose(callback: Callback<void>): void {
      this.onSync(this.nativeObj, 'subWindowClose', callback);
    }

    public offSubWindowClose(callback?: Callback<void>): void {
      this.offSync(this.nativeObj, 'subWindowClose', callback);
    }
}

export enum GlobalWindowMode {
  FULLSCREEN = 1,
  SPLIT = 1 << 1,
  FLOAT = 1 << 2,
  PIP = 1 << 3
}

export enum ScreenshotEventType {
  SYSTEM_SCREENSHOT = 0,
  SYSTEM_SCREENSHOT_ABORT = 1,
  SCROLL_SHOT_START = 2,
  SCROLL_SHOT_END = 3,
  SCROLL_SHOT_ABORT = 4,
}

export interface Window {
    hide(): Promise<void>;
    hide(callback: AsyncCallback<void>): void;
    createSubWindowWithOptions(name: string, options: SubWindowOptions): Promise<Window>;
    restore(): Promise<void>;
    restoreMainWindow(wantParameters?: Record<string, object>): Promise<void>;
    resize(width: int, height: int): Promise<void>;
    moveWindowTo(x: int, y: int): Promise<void>;
    resize(width: int, height: int, callback: AsyncCallback<void>): void;
    moveWindowTo(x: int, y: int, callback: AsyncCallback<void>): void;
    getGlobalRect(): Rect;
    setDecorButtonStyle(decorStyle: DecorButtonStyle): void;
    setWindowTitle(titleName: string): Promise<void>;
    setWindowTitleButtonVisible(isMaximizeButtonVisible: boolean, isMinimizeButtonVisible: boolean,
      isCloseButtonVisible?: boolean): void;
    setTitleButtonVisible(isMaximizeVisible: boolean, isMinimizeVisible: boolean,
      isSplitVisible: boolean): void;
    setTitleButtonVisible(isMaximizeVisible: boolean, isMinimizeVisible: boolean,
      isSplitVisible: boolean, isCloseVisible: boolean): void;
    setWindowTitleMoveEnabled(enabled: boolean): void;
    getTitleButtonRect(): TitleButtonRect;
    getDecorButtonStyle(): DecorButtonStyle;
    setTitleAndDockHoverShown(isTitleHoverShown?: boolean, isDockHoverShown?: boolean): Promise<void>;
    setHandwritingFlag(enable: boolean): Promise<void>;
    getWindowDecorVisible(): boolean;
    onWindowWillClose(callback: WindowWillCloseCallback<void, Promise<boolean>>): void;
    offWindowWillClose(callback?: WindowWillCloseCallback<void, Promise<boolean>>): void;
    onWindowTitleButtonRectChange(callback: Callback<TitleButtonRect>): void;
    offWindowTitleButtonRectChange(callback?: Callback<TitleButtonRect>): void;
    getWindowDecorHeight(): int;
    setWindowBackgroundColor(color: string | ColorMetrics): void;
    setBackgroundColor(color: string): Promise<void>;
    setBackgroundColor(color: string, callback: AsyncCallback<void>): void;
    setImmersiveModeEnabledState(enable: boolean): void;
    setWindowDecorVisible(isVisible: boolean): void;
    setWindowDecorHeight(height: int): void;
    getWindowProperties(): WindowProperties;
    getProperties(): Promise<WindowProperties>;
    getProperties(callback: AsyncCallback<WindowProperties>): void;
    isWindowSupportWideGamut(): Promise<boolean>;
    isWindowSupportWideGamut(callback: AsyncCallback<boolean>): void;
    setWindowLayoutFullScreen(isLayoutFullScreen: boolean): Promise<void>;
    setWindowLayoutFullScreen(isLayoutFullScreen: boolean, callback: AsyncCallback<void>): void;
    setExclusivelyHighlighted(exclusivelyHighlighted: boolean): Promise<void>;
    setRaiseByClickEnabled(enable: boolean): Promise<void>;
    setRaiseByClickEnabled(enable: boolean, callback: AsyncCallback<void>): void;
    setWindowSystemBarProperties(systemBarProperties: SystemBarProperties): Promise<void>;
    setWindowSystemBarProperties(systemBarProperties: SystemBarProperties, callback: AsyncCallback<void>): void;
    setSpecificSystemBarEnabled(name: String, enable: boolean, enableAnimation?: boolean): Promise<void>;
    setDragKeyFramePolicy(keyFramePolicy: KeyFramePolicy): Promise<KeyFramePolicy>;
    snapshot(): Promise<image.PixelMap>;
    snapshot(callback: AsyncCallback<image.PixelMap | undefined>): void;
    snapshotSync(): image.PixelMap;
    hideNonSystemFloatingWindows(shouldHide: boolean): Promise<void>;
    hideNonSystemFloatingWindows(shouldHide: boolean, callback: AsyncCallback<void>): void;
    setWindowColorSpace(colorSpace: ColorSpace): Promise<void>;
    setWindowColorSpace(colorSpace: ColorSpace, callback: AsyncCallback<void>): void;
    setPreferredOrientation(orientation: Orientation): Promise<void>;
    setPreferredOrientation(orientation: Orientation, callback: AsyncCallback<void>): void;
    getPreferredOrientation(): Orientation;
    convertOrientationAndRotation(from: RotationInfoType, to: RotationInfoType, value: int): int;
    setWindowPrivacyMode(isPrivacyMode: boolean): Promise<void>;
    setWindowPrivacyMode(isPrivacyMode: boolean, callback: AsyncCallback<void>): void;
    recover(): Promise<void>;
    setUIContent(path: string): Promise<void>;
    setUIContent(path: string, callback: AsyncCallback<void>): void;
    loadContent(path: string, storage: LocalStorage): Promise<void>;
    loadContent(path: string, storage: LocalStorage, callback: AsyncCallback<void>): void;
    loadContent(path: string): Promise<void>;
    loadContent(path: string, callback: AsyncCallback<void>): void;
    loadContentByName(name: string, storage: LocalStorage, callback: AsyncCallback<void>): void;
    loadContentByName(name: string, storage?: LocalStorage): Promise<void>;
    loadContentByName(name: string, callback: AsyncCallback<void>): void;
    onAvoidAreaChange(callback: Callback<AvoidAreaOptions>): void;
    offAvoidAreaChange(callback?: Callback<AvoidAreaOptions>): void;
    onDisplayIdChange(callback: Callback<long>): void;
    offDisplayIdChange(callback?: Callback<long>): void;
    onWindowVisibilityChange(callback: Callback<boolean>): void;
    offWindowVisibilityChange(callback?: Callback<boolean>): void;
    onSystemDensityChange(callback: Callback<double>): void;
    offSystemDensityChange(callback?: Callback<double>): void;
    onScreenshot(callback: Callback<void>): void;
    offScreenshot(callback?: Callback<void>): void;
    setWindowKeepScreenOn(isKeepScreenOn: boolean): Promise<void>;
    setWindowKeepScreenOn(isKeepScreenOn: boolean, callback: AsyncCallback<void>): void;
    setWindowSystemBarEnable(names: Array<'status' | 'navigation'>): Promise<void>;
    setWindowSystemBarEnable(names: Array<'status' | 'navigation'>, callback: AsyncCallback<void>): void;
    setWindowTouchable(isTouchable: boolean): Promise<void>;
    setWindowTouchable(isTouchable: boolean, callback: AsyncCallback<void>): void;
    setDialogBackGestureEnabled(enabled: boolean): Promise<void>;
    setWindowMask(windowMask: Array<Array<long>>): Promise<void>;
    setTouchableAreas(rects: Array<Rect>): void;
    getUIContext(): UIContext;
    getWindowAvoidArea(type: AvoidAreaType): AvoidArea;
    getWindowAvoidAreaIgnoringVisibility(type: AvoidAreaType): AvoidArea;
    setWindowBrightness(brightness: double): Promise<void>;
    setWindowBrightness(brightness: double, callback: AsyncCallback<void>): void;
    getWindowColorSpace(): ColorSpace;
    setWakeUpScreen(wakeUp: boolean): void;
    setSnapshotSkip(isSkip: boolean): void;
    snapshotIgnorePrivacy(): Promise<image.PixelMap>;
    getStatusBarProperty(): StatusBarProperty;
    setStatusBarColor(color: ColorMetrics): Promise<void>;
    isSystemAvoidAreaEnabled(): boolean;
    setSystemAvoidAreaEnabled(enabled: boolean): Promise<void>;
    getWindowDensityInfo(): WindowDensityInfo;
    getImmersiveModeEnabledState(): boolean;
    setWindowGrayScale(grayScale: double): Promise<void>;
    getWindowSystemBarProperties(): SystemBarProperties;
    isGestureBackEnabled(): boolean;
    setGestureBackEnabled(enabled: boolean): Promise<void>;
    setSingleFrameComposerEnabled(enabled: boolean): Promise<void>;
    setWaterMarkFlag(enable: boolean): Promise<void>;
    setWaterMarkFlag(enable: boolean, callback: AsyncCallback<void>): void;
    raiseMainWindowAboveTarget(windowId: int): Promise<void>;
    setWindowTopmost(isWindowTopmost: boolean): Promise<void>;
    setMainWindowRaiseByClickEnabled(enable: boolean): Promise<void>;
    setWindowFocusable(isFocusable: boolean): Promise<void>;
    setWindowFocusable(isFocusable: boolean, callback: AsyncCallback<void>): void;
    getSubWindowZLevel(): int;
    isFocused(): boolean;
    setSubWindowZLevel(zLevel: int): Promise<void>;
    raiseAboveTarget(windowId: int): Promise<void>;
    raiseAboveTarget(windowId: int, callback: AsyncCallback<void>): void;
    raiseToAppTop(callback: AsyncCallback<void>): void;
    raiseToAppTop(): Promise<void>;
    setTopmost(isTopmost: boolean): Promise<void>;
    setReceiveDragEventEnabled(enabled: boolean): Promise<void>;
    isReceiveDragEventEnabled(): boolean;
    setSeparationTouchEnabled(enabled: boolean): Promise<void>;
    isSeparationTouchEnabled(): boolean;
    requestFocus(isFocused: boolean): Promise<void>;
    setSubWindowModal(isModal: boolean): Promise<void>;
    setSubWindowModal(isModal: boolean, modalityType: ModalityType): Promise<void>;
    showWindow(callback: AsyncCallback<void>): void;
    showWindow(): Promise<void>;
    showWindow(options: ShowWindowOptions): Promise<void>;
    destroyWindow(callback: AsyncCallback<void>): void;
    destroyWindow(): Promise<void>;
    isWindowShowing(): boolean;
    isWindowHighlighted(): boolean;
    hideWithAnimation(callback: AsyncCallback<void>): void;
    hideWithAnimation(): Promise<void>;
    showWithAnimation(callback: AsyncCallback<void>): void;
    showWithAnimation(): Promise<void>;
    getParentWindow(): Window;
    setParentWindow(windowId: int): Promise<void>;
    bindDialogTarget(token: rpc.RemoteObject, deathCallback: Callback<void>): Promise<void>;
    bindDialogTarget(token: rpc.RemoteObject, deathCallback: Callback<void>,
      callback: AsyncCallback<void>): void;
    bindDialogTarget(requestInfo: dialogRequest.RequestInfo, deathCallback: Callback<void>): Promise<void>;
    bindDialogTarget(requestInfo: dialogRequest.RequestInfo, deathCallback: Callback<void>,
      callback: AsyncCallback<void>): void;
    setContentAspectRatio(ratio: double, isPersistent?: boolean, needUpdateRect?: boolean): Promise<void>;
    opacity(opacity: double): void;
    scale(scaleOptions: ScaleOptions): void;
    translate(translateOptions: TranslateOptions): void;
    rotate(rotateOptions: RotateOptions): void;
    setShadow(radius: double, color?: string, offsetX?: double, offsetY?: double): void;
    onWindowHighlightChange(callback: Callback<boolean>): void;
    offWindowHighlightChange(callback?: Callback<boolean>): void;
    setCornerRadius(cornerRadius: double): void;
    getTransitionController(): TransitionController;
    setBlur(radius: double): void;
    setBackdropBlurStyle(blurStyle: BlurStyle): void;
    setBackdropBlur(radius: double): void;
    getWindowCornerRadius(): double;
    setWindowCornerRadius(cornerRadius: double): Promise<void>;
    setWindowShadowRadius(radius: double): void;
    on<T>(type: string, callback: Callback<T>): void;
    on<T>(type: 'noInteractionDetected', timeout: long, callback: Callback<T>): void;
    onTouchOutside(callback: Callback<void>): void;
    onDialogTargetTouch(callback: Callback<void>): void;
    onNoInteractionDetected(timeout: long, callback: Callback<void>): void;
    onKeyboardHeightChange(callback: Callback<int>): void;
    onKeyboardWillShow(callback: Callback<KeyboardInfo>): void;
    onKeyboardWillHide(callback: Callback<KeyboardInfo>): void;
    onKeyboardDidShow(callback: Callback<KeyboardInfo>): void;
    onKeyboardDidHide(callback: Callback<KeyboardInfo>): void;
    onUiExtensionSecureLimitChange(callback: Callback<boolean>): void;
    onRotationChange(callback: RotationChangeCallback<RotationChangeInfo, RotationChangeResult |
      undefined>): void;
    off<T>(type: string, callback?: Callback<T>): void;
    offTouchOutside(callback?: Callback<void>): void;
    offDialogTargetTouch(callback?: Callback<void>): void;
    offNoInteractionDetected(callback?: Callback<void>): void;
    offKeyboardHeightChange(callback?: Callback<int>): void;
    offKeyboardWillShow(callback?: Callback<KeyboardInfo>): void;
    offKeyboardWillHide(callback?: Callback<KeyboardInfo>): void;
    offKeyboardDidShow(callback?: Callback<KeyboardInfo>): void;
    offKeyboardDidHide(callback?: Callback<KeyboardInfo>): void;
    offUiExtensionSecureLimitChange(callback?: Callback<boolean>): void;
    offRotationChange(callback?: RotationChangeCallback<RotationChangeInfo, RotationChangeResult |
      undefined>): void;
    onOcclusionStateChanged(callback: Callback<OcclusionState>): void;
    offOcclusionStateChanged(callback?: Callback<OcclusionState>): void;
    onFrameMetricsMeasured(callback: Callback<FrameMetrics>): void;
    offFrameMetricsMeasured(callback: Callback<FrameMetrics>): void;
    onWindowSizeChange(callback: Callback<Size>): void;
    offWindowSizeChange(callback?: Callback<Size>): void;
    onWindowStatusChange(callback: Callback<WindowStatusType>): void;
    offWindowStatusChange(callback?: Callback<WindowStatusType>): void;
    onWindowRectChange(callback: Callback<RectChangeOptions>): void;
    offWindowRectChange(callback?: Callback<RectChangeOptions>): void;
    onWindowStatusDidChange(callback: Callback<WindowStatusType>): void;
    offWindowStatusDidChange(callback?: Callback<WindowStatusType>): void;
    onRectChangeInGlobalDisplay(callback: Callback<RectChangeOptions>): void;
    offRectChangeInGlobalDisplay(callback?: Callback<RectChangeOptions>): void;
    resizeAsync(width: int, height: int): Promise<void>;
    setWindowLimits(windowLimits: WindowLimits): Promise<WindowLimits>;
    setWindowLimits(windowLimits: WindowLimits, isForcible: boolean): Promise<WindowLimits>;
    getWindowLimits(): WindowLimits;
    getWindowLimitsVP(): WindowLimits;
    setAspectRatio(ratio: double, callback: AsyncCallback<void>): void;
    setAspectRatio(ratio: double): Promise<void>;
    resetAspectRatio(callback: AsyncCallback<void>): void;
    resetAspectRatio(): Promise<void>;
    getWindowStatus(): WindowStatusType;
    minimize(callback: AsyncCallback<void>): void;
    minimize(): Promise<void>;
    maximize(presentation?: MaximizePresentation): Promise<void>;
    maximize(presentation?: MaximizePresentation, acrossDisplay?: boolean): Promise<void>;
    startMoving(): Promise<void>;
    startMoving(offsetX: int, offsetY: int): Promise<void>;
    stopMoving(): Promise<void>;
    setResizeByDragEnabled(enable: boolean, callback: AsyncCallback<void>): void;
    setResizeByDragEnabled(enable: boolean): Promise<void>;
    enableDrag(enable: boolean): Promise<void>;
    moveWindowToGlobal(x: int, y: int): Promise<void>;
    moveWindowToGlobal(x: int, y: int, moveConfiguration?: MoveConfiguration): Promise<void>;
    moveWindowToAsync(x: int, y: int): Promise<void>;
    moveWindowToAsync(x: int, y: int, moveConfiguration?: MoveConfiguration): Promise<void>;
    setWindowMode(mode: WindowMode): Promise<void>;
    setWindowMode(mode: WindowMode, callback: AsyncCallback<void>): void;
    setForbidSplitMove(isForbidSplitMove: boolean, callback: AsyncCallback<void>): void;
    setForbidSplitMove(isForbidSplitMove: boolean): Promise<void>;
    setFollowParentWindowLayoutEnabled(enabled: boolean): Promise<void>;
    setFollowParentMultiScreenPolicy(enabled: boolean): Promise<void>;
    moveWindowToGlobalDisplay(x: int, y: int): Promise<void>;
    clientToGlobalDisplay(winX: int, winY: int): Position;
    globalDisplayToClient(globalDisplayX: int, globalDisplayY: int): Position;
    setRotationLocked(locked: boolean): Promise<void>;
    getRotationLocked(): boolean;
    isInFreeWindowMode(): boolean;
    onFreeWindowModeChange(callback: Callback<boolean>): void;
    offFreeWindowModeChange(callback?: Callback<boolean>): void;
    setRelativePositionToParentWindowEnabled(enabled: boolean, anchor?: WindowAnchor,
      offsetX?: int, offsetY?: int): Promise<void>;
    setWindowDelayRaiseOnDrag(isEnabled: boolean): void;
    setDefaultDensityEnabled(enabled: boolean): void;
    setWindowContainerColor(activeColor: string, inactiveColor: string): void;
    setWindowContainerModalColor(activeColor: string, inactiveColor: string): void;
    getWindowStateSnapshot(): Promise<string>;
    isMainWindowFullScreenAcrossDisplays(): Promise<boolean>;
    setWindowShadowEnabled(enable: boolean): Promise<void>;
    isImmersiveLayout(): boolean;
    onScreenshotAppEvent(callback: Callback<ScreenshotEventType>): void;
    offScreenshotAppEvent(callback: Callback<ScreenshotEventType>): void;
    onMainWindowFullScreenAcrossDisplaysChanged(callback: Callback<boolean>): void;
    offMainWindowFullScreenAcrossDisplaysChanged(callback?: Callback<boolean>): void;
    setWindowTransitionAnimation(transitionType: WindowTransitionType, animation: TransitionAnimation): Promise<void>;
    getWindowTransitionAnimation(transitionType: WindowTransitionType): TransitionAnimation | undefined;
    keepKeyboardOnFocus(enable: boolean): void;
    onWindowEvent(callback: Callback<WindowEventType>): void;
    offWindowEvent(callback?: Callback<WindowEventType>): void;
    onSubWindowClose(callback: Callback<void>): void;
    offSubWindowClose(callback?: Callback<void>): void;
}

export native function CreateWindowStage(scene: long): WindowStageInternal;
export function CreateWindowStageApi(scene: long): WindowStage { return CreateWindowStage(scene); }
export native function CreateWindow(window: long): WindowInternal;

let nativeObj: long;
export function setNativeObj(nativeObject: long):void {
  nativeObj = nativeObject;
}

native function getLastWindowSync(nativeObj: long, ctx: BaseContext): Window;
native function minimizeAllSync(nativeObj: long, id: long): void;
native function minimizeAllWithExclusionSync(nativeObj: long, displayId: long, excludeWindowId: int): void;
native function findWindowSync(nativeObj: long, name: string): Window;
native function onSync(nativeObj: long, type: string, callback: object): void;
native function offSync(nativeObj: long, type: string, callback?: object): void;
native function createWindowSync(nativeObj: long, config: Configuration): Window;
native function getAllWindowLayoutInfo(nativeObj: long, displayId: long): Array<WindowLayoutInfo>;
native function getSnapshot(nativeObj: long, windowId: int): image.PixelMap;
native function getVisibleWindowInfo(nativeObj: long): Array<WindowInfoInternal>;
native function setGestureNavigationEnabled(nativeObj: long, enabled: boolean): void;
native function setWaterMarkImage(nativeObj: long, pixelMap: image.PixelMap, enabled: boolean): void;
native function getWindowsByCoordinate(nativeObj: long, param: GetWindowsByCoordinateParam): Array<Window>;
native function shiftAppWindowFocusSync(nativeObj: long, sourceWindowId: int, targetWindowId: int): void;
native function shiftAppWindowPointerEventSync(nativeObj: long, sourceWindowId: int, targetWindowId: int): void;
native function shiftAppWindowTouchEventSync(nativeObj: long, sourceWindowId: int, targetWindowId: int,
  fingerId: int): void;
native function setWindowLayoutMode(nativeObj: long, mode: WindowLayoutMode): void;
native function setWatermarkImageForAppWindowsSync(nativeObj: long, pixelMap: image.PixelMap | undefined): void;
native function getTopNavDestinationNameSync(nativeObj: long, windowId: int): string;
native function getGlobalWindowModeSync(nativeObj: long, displayId?: long): int;
native function setStartWindowBackgroundColorSync(nativeObj: long, moduleName: string, abilityName: string,
  color: long): void;
native function notifyScreenshotEventSync(nativeObj: long, eventType: ScreenshotEventType): void;
native function toggleShownStateForAllAppWindowsSync(nativeObj: long): void;
native function setSpecificSystemWindowZIndexSync(nativeObj: long, windowType: WindowType, zIndex: int): void;

export function toggleShownStateForAllAppWindows(callback: AsyncCallback<void>): void {
      taskpool.execute((): void => {
          toggleShownStateForAllAppWindowsSync(nativeObj);
      }).then((ret: Any) => {
          callback(new BusinessError(), undefined);
      }).catch((err: Any) => {
          callback(err as BusinessError, undefined);
      });
}

export function toggleShownStateForAllAppWindows(): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
    taskpool.execute((): void => {
        return toggleShownStateForAllAppWindowsSync(nativeObj);
    }).then((ret: Any) => {
        resolve(undefined);
    }).catch((err: Any) => {
        reject(err as BusinessError);
    });
  });
}

export function setWatermarkImageForAppWindows(pixelMap: image.PixelMap | undefined): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
    taskpool.execute((): void => {
      return setWatermarkImageForAppWindowsSync(nativeObj, pixelMap);
    }).then((ret: Any) => {
      resolve(undefined);
    }).catch((err: Any) => {
      reject(err as BusinessError);
    });
  });
}

export function getTopNavDestinationName(windowId: int): Promise<string> {
  return new Promise<string>((resolve: (value: string) => void, reject: (error: BusinessError) => void) => {
    taskpool.execute((): string => {
      return getTopNavDestinationNameSync(nativeObj, windowId);
    }).then((ret: Any) => {
      resolve(ret as string);
    }).catch((err: Any) => {
      reject(err as BusinessError);
    });
  });
}

export function getGlobalWindowMode(displayId?: long): Promise<int> {
  return new Promise<int>((resolve: (value: int) => void, reject: (error: BusinessError) => void): void => {
    taskpool.execute((): int => {
      return getGlobalWindowModeSync(nativeObj, displayId);
    }).then((ret: Any) => {
      resolve(ret as int);
    }).catch((err: Any) => {
      reject(err as BusinessError);
    });
  });
}

export function setStartWindowBackgroundColor(moduleName: string, abilityName: string,
  color: ColorMetrics): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
    taskpool.execute((): void => {
      return setStartWindowBackgroundColorSync(nativeObj, moduleName, abilityName,
        (color.alpha << 24) | (color.red << 16) | (color.green << 8) | color.blue);
    }).then((ret: Any) => {
      resolve(undefined);
    }).catch((err: Any) => {
      reject(err as BusinessError);
    });
  });
}

export function notifyScreenshotEvent(eventType: ScreenshotEventType): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
    taskpool.execute((): void => {
      return notifyScreenshotEventSync(nativeObj, eventType);
    }).then((ret: Any) => {
      resolve(undefined);
    }).catch((err: Any) => {
      reject(err as BusinessError);
    });
  });
}

export function getLastWindow(ctx: BaseContext): Promise<Window> {
  return new Promise<Window>((resolve: (value: Window) => void, reject: (error: BusinessError) => void ) => {
    taskpool.execute((): Window => {
      return getLastWindowSync(nativeObj, ctx);
    }).then((ret: Any) => {
      resolve(ret as Window);
    }).catch((err: Any) => {
      reject(err as BusinessError);
    });
  }); 
}

export function getLastWindow(ctx: BaseContext, callback: AsyncCallback<Window>): void {
    taskpool.execute((): Window => {
        return getLastWindowSync(nativeObj, ctx);
    }).then((ret: Any) => {
        callback(new BusinessError(), ret as Window);
    }).catch((err: Any) => {
        callback(err as BusinessError, new WindowInternal());
    });
}

export function createWindow(config: Configuration): Promise<Window> {
  return new Promise<Window>((resolve: (value: Window) => void, reject: (error: BusinessError) => void ) => {
      taskpool.execute((): Window => {
          return createWindowSync(nativeObj, config);
      }).then((ret: Any) => {
          resolve(ret as Window);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}

export function createWindow(config: Configuration, callback: AsyncCallback<Window>): void {
    taskpool.execute((): Window => {
        return createWindowSync(nativeObj, config);
    }).then((ret: Any) => {
        callback(new BusinessError(), ret as Window);
    }).catch((err: Any) => {
        callback(err as BusinessError, new WindowInternal());
    });
}

export function minimizeAll(id: long): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute((): void => {
          minimizeAllSync(nativeObj, id);
      }).then((ret: Any) => {
          resolve(undefined);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}


export function minimizeAll(id: long, callback: AsyncCallback<void>): void {
    taskpool.execute((): void => {
        minimizeAllSync(nativeObj, id);
    }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function minimizeAllWithExclusion(displayId: long, excludeWindowId: int): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute((): void => {
          minimizeAllWithExclusionSync(nativeObj, displayId, excludeWindowId);
      }).then((ret: Any) => {
          resolve(undefined);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}

export function getAllWindowLayoutInfo(displayId: long): Promise<Array<WindowLayoutInfo>> {
  return new Promise<Array<WindowLayoutInfo>>((
    resolve: (value: Array<WindowLayoutInfo>) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute(() => {
          return getAllWindowLayoutInfo(nativeObj, displayId);
      }).then((ret: Any) => {
          resolve(ret as Array<WindowLayoutInfo>);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}

export function getSnapshot(windowId: int): Promise<image.PixelMap> {
  return new Promise<image.PixelMap>((
    resolve: (value: image.PixelMap) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute(() => {
          return getSnapshot(nativeObj, windowId);
      }).then((ret: Any) => {
          resolve(ret as image.PixelMap);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}

export function getVisibleWindowInfo(): Promise<Array<WindowInfo>> {
  return new Promise<Array<WindowInfo>>((
    resolve: (value: Array<WindowInfo>) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute(() => {
          let visibleWindowInfos = getVisibleWindowInfo(nativeObj);
          for (let i = 0; i < visibleWindowInfos.length; i++) {
            visibleWindowInfos[i].windowStatusType =
              visibleWindowInfos[i].windowStatusTypeInternal as WindowStatusType;
            visibleWindowInfos[i].isFocused = visibleWindowInfos[i].isFocusedInternal;
          }
          return visibleWindowInfos;
      }).then((ret: Any) => {
          resolve(ret as Array<WindowInfo>);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}

export function setGestureNavigationEnabled(enabled: boolean): Promise<void> {
  return new Promise<void>((
    resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute((): void => {
          return setGestureNavigationEnabled(nativeObj, enabled);
      }).then((ret: Any) => {
          resolve(undefined);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}

export function setGestureNavigationEnabled(enabled: boolean, callback: AsyncCallback<void>): void {
    taskpool.execute((): void => {
        setGestureNavigationEnabled(nativeObj, enabled);
    }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function setWaterMarkImage(pixelMap: image.PixelMap, enabled: boolean): Promise<void> {
  return new Promise<void>((
    resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute((): void => {
          return setWaterMarkImage(nativeObj, pixelMap, enabled);
      }).then((ret: Any) => {
          resolve(undefined);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}

export function setWaterMarkImage(pixelMap: image.PixelMap, enabled: boolean, callback: AsyncCallback<void>): void {
    taskpool.execute((): void => {
        setWaterMarkImage(nativeObj, pixelMap, enabled);
    }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function getWindowsByCoordinate(displayId: long, windowNumber?: int,
  x?: int, y?: int): Promise<Array<Window>> {
  return new Promise<Array<Window>>((resolve: (value: Array<Window>) => void,
                                     reject: (error: BusinessError) => void): void => {
    taskpool.execute((): Array<Window> => {
      let param: window.GetWindowsByCoordinateParam = {
        displayId: displayId,
        windowNumber: 0,
        x: -1,
        y: -1
      } as GetWindowsByCoordinateParamInternal;
      if (windowNumber != undefined) {
        param.windowNumber = windowNumber;
      }
      if (x != undefined) {
        param.x = x;
      }
      if (y != undefined) {
        param.y = y;
      }
      return getWindowsByCoordinate(nativeObj, param);
    }).then((ret: Any) => {
        resolve(ret as Array<Window>);
    }).catch((err: Any) => {
        reject(err as BusinessError);
    });
  });
}

export function findWindow(name: string): Window {
  return findWindowSync(nativeObj, name);
}

export function shiftAppWindowFocus(sourceWindowId: int, targetWindowId: int): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute((): void => {
          return shiftAppWindowFocusSync(nativeObj, sourceWindowId, targetWindowId);
      }).then((ret: Any) => {
          resolve(undefined);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}

export function shiftAppWindowPointerEvent(sourceWindowId: int, targetWindowId: int): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
    taskpool.execute((): void => {
      return shiftAppWindowPointerEventSync(nativeObj, sourceWindowId, targetWindowId);
    }).then((ret: Any) => {
      resolve(undefined);
    }).catch((err: Any) => {
      reject(err as BusinessError);
    });
  });
}

export function setSpecificSystemWindowZIndex(windowType: WindowType, zIndex: int): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute((): void => {
          return setSpecificSystemWindowZIndexSync(nativeObj, windowType, zIndex);
      }).then((ret: Any) => {
          resolve(undefined);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}

export function shiftAppWindowTouchEvent(sourceWindowId: int, targetWindowId: int, fingerId: int): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
    taskpool.execute((): void => {
      return shiftAppWindowTouchEventSync(nativeObj, sourceWindowId, targetWindowId, fingerId);
    }).then((ret: Any) => {
      resolve(undefined);
    }).catch((err: Any) => {
      reject(err as BusinessError);
    });
  });
}

export function setWindowLayoutMode(mode: WindowLayoutMode, callback: AsyncCallback<void>): void {
    taskpool.execute((): void => {
        setWindowLayoutMode(nativeObj, mode);
    }).then((ret: Any) => {
        callback(new BusinessError(), undefined);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function setWindowLayoutMode(mode: WindowLayoutMode): Promise<void> {
  return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute((): void => {
          setWindowLayoutMode(nativeObj, mode);
      }).then((ret: Any) => {
          resolve(undefined);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  }); 
}

export interface MainWindowInfo {
  displayId: number;
  windowId: number;
  showing: boolean;
  label: string;
}

export class MainWindowInfoInternal implements MainWindowInfo {
  displayId: number;
  windowId: number;
  showing: boolean;
  label: string;
}

export interface WindowSnapshotConfiguration {
  useCache?: boolean;
}

export class WindowSnapshotConfigurationInternal implements WindowSnapshotConfiguration {
  useCache?: boolean;
}

native function getAllMainWindowInfo(nativeObj: long): Array<MainWindowInfo>;
native function getMainWindowSnapshot(
  nativeObj: long, windowId: Array<number>, config: WindowSnapshotConfiguration): Array<image.PixelMap>;

export function getAllMainWindowInfo(): Promise<Array<MainWindowInfo>> {
  return new Promise<Array<MainWindowInfo>>(
    (resolve: (value: Array<MainWindowInfo>) => void, reject: (error: BusinessError) => void ) => {
      taskpool.execute((): Array<MainWindowInfo> => {
          return getAllMainWindowInfo(nativeObj);
      }).then((ret: Any) => {
          resolve(ret as Array<MainWindowInfo>);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
  });
}

export function getMainWindowSnapshot(windowId: Array<number>, config: WindowSnapshotConfiguration):
  Promise<Array<image.PixelMap>> {
    return new Promise<Array<image.PixelMap>>(
      (resolve: (value: Array<image.PixelMap>) => void, reject: (error: BusinessError) => void ) => {
      taskpool.execute((): Array<image.PixelMap> => {
        return getMainWindowSnapshot(nativeObj, windowId, config);
    }).then((ret: Any) => {
        resolve(ret as Array<image.PixelMap>);
    }).catch((err: Any) => {
        reject(err as BusinessError);
    });
  });
}

export function on<T>(type: string, callback: Callback<T>): void {
  onSync(nativeObj, type, callback);
}

export function off<T>(type: string, callback?: Callback<T>): void {
  offSync(nativeObj, type, callback);
}

export function onSystemBarTintChange(callback: Callback<SystemBarTintState>): void {
  onSync(nativeObj, 'systemBarTintChange', callback);
}

export function offSystemBarTintChange(callback?: Callback<SystemBarTintState>): void {
  offSync(nativeObj, 'systemBarTintChange', callback);
}

export function onGestureNavigationEnabledChange(callback: Callback<boolean>): void {
  onSync(nativeObj, 'gestureNavigationEnabledChange', callback);
}

export function offGestureNavigationEnabledChange(callback?: Callback<boolean>): void {
  offSync(nativeObj, 'gestureNavigationEnabledChange', callback);
}

export function onWaterMarkFlagChange(callback: Callback<boolean>): void {
  onSync(nativeObj, 'waterMarkFlagChange', callback);
}

export function offWaterMarkFlagChange(callback?: Callback<boolean>): void {
  offSync(nativeObj, 'waterMarkFlagChange', callback);
}

function runWindowListenerVoidArgCallback(cb: object): void {
  const func = cb as () => void;
  func();
}

function runWindowListenerBooleanArgCallback(cb: object, cbArg: boolean): void {
  const func = cb as (cbArg: boolean) => void;
  func(cbArg);
}

function runSystemDensityChangeCallback(cb: object, density: double): void {
  const func = cb as (density: double) => void;
  func(density as double);
}

function runDisplayIdChangeCallback(cb: object, displayId: long): void {
  const func = cb as (displayId: long) => void;
  func(displayId as long);
}

function runSystemBarTintChangeCallback(cb: object, tint: object): void {
  const func = cb as (tint: SystemBarTintState) => void;
  func(tint as SystemBarTintState);
}

function runAvoidAreaChangeCallback(cb: object, area: object, type: int): void {
  const func = cb as (data: AvoidAreaOptions) => void;
  func({type: type as AvoidAreaType, area: area as AvoidArea});
}

function runWindowStageLifecycleEventCallback(cb: object, cbArg: int): void {
    const func = cb as (cbArg: WindowStageLifeCycleEventType) => void;
    func(cbArg as WindowStageLifeCycleEventType);
}

function runWindowEventCallback(cb: object, cbArg: int): void {
    const func = cb as (cbArg: WindowEventType) => void;
    func(cbArg as WindowEventType);
}

function runWindowSizeCallback(cb: object, cbArg: object): void {
    const func = cb as (cbArg: Size) => void;
    func(cbArg as Size);
}

function runWindowRectChangeCallback(cb: object, rect: object, reason: int): void {
    const func = cb as (data: RectChangeOptions) => void;
    func({rect: rect as Rect, reason: reason as RectChangeReason});
}

function runKeyboardHeightChangeCallback(cb: object, cbArg: int): void {
    const func = cb as (cbArg: int) => void;
    func(cbArg as int);
}

function runKeyboardDidShowCallback(cb: object, cbArg: object): void {
    const func = cb as (cbArg: KeyboardInfo) => void;
    func(cbArg as KeyboardInfo);
}

function runKeyboardDidHideCallback(cb: object, cbArg: object): void {
    const func = cb as (cbArg: KeyboardInfo) => void;
    func(cbArg as KeyboardInfo);
}

function runKeyboardWillShowCallback(cb: object, cbArg: object): void {
    const func = cb as (cbArg: KeyboardInfo) => void;
    func(cbArg as KeyboardInfo);
}

function runKeyboardWillHideCallback(cb: object, cbArg: object): void {
    const func = cb as (cbArg: KeyboardInfo) => void;
    func(cbArg as KeyboardInfo);
}

function runWindowStatusCallback(cb: object, cbArg: int): void {
    const func = cb as (cbArg: WindowStatusType) => void;
    func(cbArg as WindowStatusType);
}

function runOcclusionStateChangeCallback(cb: object, cbArg: int): void {
    const func = cb as (cbArg: OcclusionState) => void;
    func(cbArg as OcclusionState);
}

function runFrameMetricsChangeCallback(cb: object, cbArg: object): void {
    const func = cb as (cbArg: FrameMetrics) => void;
    func(cbArg as FrameMetrics);
}

function runWindowTouchOutCallback(cb: object): void {
  const func = cb as () => void;
  func();
}

function runWindowDialogTargetCallback(cb: object): void {
  const func = cb as () => void;
  func();
}

function runWindowNoInteractionCallback(cb: object): void {
  const func = cb as () => void;
  func();
}

function runWindowStageEventCallback(cb: object, cbArg: int): void {
    const func = cb as (cbArg: WindowStageEventType) => void;
    func(cbArg as WindowStageEventType);
}

function runMainWindowFullScreenAcrossDisplaysChangedCallback(cb: object, cbArg: boolean): void {
  const func = cb as (cbArg: boolean) => void;
  func(cbArg as boolean);
}

function runScreenshotAppEvent(cb: object, cbArg: ScreenshotEventType): void {
  const func = cb as (cbArg: ScreenshotEventType) => void;
  func(cbArg as ScreenshotEventType);
}

function runWindowTitleButtonRectChangedCallback(cb: object, cbArg: object): void {
  const func = cb as (cbArg: TitleButtonRect) => void;
  func(cbArg as TitleButtonRect);
}

function runRectChangeInGlobalDisplayCallback(cb: object, rect: object, reason: int): void {
  const func = cb as (data: RectChangeOptions) => void;
  func({rect: rect as Rect, reason: reason as RectChangeReason});
}

function runUiExtensionSecureLimitChangeCallback(cb: object, cbArg: boolean): void {
  const func = cb as (cbArg: boolean) => void;
  func(cbArg);
}

function runWindowStatusDidChangeCallback(cb: object, windowStatus: int): void {
  const func = cb as (windowStatus: WindowStatusType) => void;
  func(windowStatus as WindowStatusType);
}
}

export default window;