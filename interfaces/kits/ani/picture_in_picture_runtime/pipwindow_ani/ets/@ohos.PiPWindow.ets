/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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

import common from '@ohos.app.ability.common';
import { XComponentController } from 'arkui.component.xcomponent';
import { LocalStorage } from '@ohos.arkui.stateManagement';
import { typeNode } from 'arkui.FrameNode';
import { NodeController } from '@ohos.arkui.node';
import { BusinessError } from '@ohos.base';
import hilog from '@ohos.hilog';

const DOMAIN = 0x420E;
const TAG = '[ANI]';

export type Callback<T> = (data: T) => void;
export type CallbackState = (state: int) => void;
export type OnCallback = Callback<typeNode.XComponent> | CallbackState;

export namespace PiPWindow {
  static { loadLibrary('pipwindowani_kit.z') };
  hilog.info(DOMAIN, TAG, 'PiPWindow load pipwindowani_kit.z finish ');
  
  export native function isPiPEnabled(): boolean;

  export interface PiPController {
    startPiP(): Promise<void>;
    stopPiP(): Promise<void>;
    isPiPSupported(): boolean;
    isPiPActive(): Promise<boolean>;
    setAutoStartEnabled(enable: boolean): void;
    getPiPSettingSwitch(): Promise<boolean>;
    updateContentSize(width: int, height: int): void;
    updatePiPControlStatus(controlType: PiPControlType, status: PiPControlStatus): void;
    setPiPControlEnabled(controlType: PiPControlType, enabled: boolean): void;
    onStateChange(callback: (state: PiPState, reason: string) => void): void;
    offStateChange(): void;
    onActiveStatusChange(callback: Callback<boolean>):void;
    offActiveStatusChange(callback?: Callback<boolean>):void;
    onPipWindowSizeChange(callback: Callback<PiPWindowSize>): void;
    offPipWindowSizeChange(callback?: Callback<PiPWindowSize>): void;
    onControlPanelActionEvent(callback: ControlPanelActionEventCallback): void;
    offControlPanelActionEvent(): void;
    onControlEvent(callback: Callback<ControlEventParam>): void;
    offControlEvent(callback?: Callback<ControlEventParam>): void;
    updateContentNode(contentNode: typeNode.XComponent): Promise<void>;
    getPiPWindowInfo(): Promise<PiPWindowInfo>;
  }

  let nativeObj: long;
  export function setNativeObj(nativeObject: long): void {
    nativeObj = nativeObject;
  }

  export type PiPStateChangeCallback = (state: PiPState, reason: string) => void;
  export type ControlPanelActionEventCallback = (event: PiPActionEventType, status?: int) => void;


  export type PiPControlGroup = VideoPlayControlGroup | VideoCallControlGroup | VideoMeetingControlGroup | VideoLiveControlGroup;

  export enum PiPTemplateType {
    VIDEO_PLAY,
    VIDEO_CALL,
    VIDEO_MEETING,
    VIDEO_LIVE,
  }

  export enum VideoPlayControlGroup {
    VIDEO_PREVIOUS_NEXT = 101,
    FAST_FORWARD_BACKWARD = 102,
  }

  export enum VideoCallControlGroup {
    MICROPHONE_SWITCH = 201,
    HANG_UP_BUTTON = 202,
    CAMERA_SWITCH = 203,
    MUTE_SWITCH = 204,
  }

  export enum VideoMeetingControlGroup {
    HANG_UP_BUTTON = 301,
    CAMERA_SWITCH = 302,
    MUTE_SWITCH = 303,
    MICROPHONE_SWITCH = 304,
  }

  export enum VideoLiveControlGroup {
    VIDEO_PLAY_PAUSE = 401,
    MUTE_SWITCH = 402,
  }

  export interface PiPWindowSize {
    width: int;
    height: int;
    scale: double;
  }

  export interface PiPWindowInfo {
    windowId: int;
    size: PiPWindowSize;
  }

  export class PiPWindowSizeInner implements PiPWindowSize {
    width: int = 0;
    height: int = 0;
    scale: double = 0.0;
    constructor(width: int, height: int, scale: double) {
      this.width = width;
      this.height = height;
      this.scale = scale;
    }
  }

  export class PiPWindowInfoInner implements PiPWindowInfo {
    windowId: int = 0;
    size: PiPWindowSize;
    constructor(windowId: int, size: PiPWindowSize) {
      this.windowId = windowId;
      this.size = size;
    }
  }

  export enum PiPState {
    ABOUT_TO_START = 1,
    STARTED = 2,
    ABOUT_TO_STOP = 3,
    STOPPED = 4,
    ABOUT_TO_RESTORE = 5,
    ERROR = 6,
  }

  export enum PiPControlStatus {
    PLAY = 1,
    PAUSE = 0,
    OPEN = 1,
    CLOSE = 0,
    DEFAULT = -1,
    ENABLED = -2,
    DISABLED = -3,
  }

  export enum PiPControlType {
    VIDEO_PLAY_PAUSE = 0,
    VIDEO_PREVIOUS = 1,
    VIDEO_NEXT = 2,
    FAST_FORWARD = 3,
    FAST_BACKWARD = 4,
    HANG_UP_BUTTON = 5,
    MICROPHONE_SWITCH = 6,
    CAMERA_SWITCH = 7,
    MUTE_SWITCH = 8,
  }

  type PiPActionEventType = PiPVideoActionEvent | PiPCallActionEvent | PiPMeetingActionEvent | PiPLiveActionEvent;
  type PiPVideoActionEvent = 'playbackStateChanged' | 'nextVideo' | 'previousVideo' | 'fastForward' | 'fastBackward';
  type PiPCallActionEvent = 'hangUp' | 'micStateChanged' | 'videoStateChanged' | 'voiceStateChanged';
  type PiPMeetingActionEvent = 'hangUp' | 'voiceStateChanged' | 'videoStateChanged' | 'micStateChanged';
  type PiPLiveActionEvent = 'playbackStateChanged' | 'voiceStateChanged';

  export interface ControlEventParam {
    controlType: PiPControlType;
    status?: PiPControlStatus;
  }

  export interface PiPConfiguration {
    context: common.BaseContext;
    componentController: XComponentController;
    navigationId?: string;
    templateType?: PiPTemplateType;
    contentWidth?: int;
    contentHeight?: int;
    controlGroups?: Array<PiPControlGroup>;
    customUIController?: NodeController;
    localStorage?: LocalStorage;
    defaultWindowSizeType?: int;
  }
  
  export native function createSync(nativeObj: long, config: PiPConfiguration, contentNode?: typeNode.XComponent): PiPController;
  function create(config: PiPConfiguration): Promise<PiPController> {
    return new Promise<PiPController>((resolve: (value: PiPController) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute((): PiPController => {
            return createSync(nativeObj, config, undefined);
        }).then((ret: Any) => {
            resolve(ret as PiPController);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
  }

  function create(config: PiPConfiguration, contentNode: typeNode.XComponent): Promise<PiPController> {
    return new Promise<PiPController>((resolve: (value: PiPController) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute((): PiPController => {
            return createSync(nativeObj, config, contentNode);
        }).then((ret: Any) => {
            resolve(ret as PiPController);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
  }

  export function runOnStateChangeEvent(cb: Object, state: int, reason: string): void {
    const func = cb as (state: PiPState, reason: string) => void;
    func(state as PiPState, reason as string);
  }

  export function runOnControlPanelActionEvent(cb: Object, action: string, status: int): void {
    (cb as ControlPanelActionEventCallback)(action as PiPActionEventType, status as int);
  }

  export function runOnControlEvent(cb: Object, controlType: int, status: int): void {
    const param: ControlEventParam = {
      controlType: controlType as PiPControlType,
      status: status as PiPControlStatus,
    };
    (cb as Callback<ControlEventParam>)(param);
  }

  export function runOnPipWindowSizeChangeEvent(cb: Object, w: int, h: int, scale: float): void {
    const size: PiPWindowSize = { width: w, height: h, scale: scale };
    (cb as Callback<PiPWindowSize>)(size);
  }

  export function runOnActiveStatusChangeEvent(cb: Object, b: boolean): void {
    (cb as Callback<boolean>)(b);
  }

  export native function pipControllerFinalizerCallback(nativeObj: long): void;
  let pipControllerFinalizerRegister = new FinalizationRegistry<long>(pipControllerFinalizerCallback);

  export class PiPControllerInternal implements PiPController {
    private nativeObj: long = 0;
    public setNativeObj(nativeObject: long): void {
      if (this.nativeObj) {
        pipControllerFinalizerRegister.unregister(this);
      }
      this.nativeObj = nativeObject;
      pipControllerFinalizerRegister.register(this, this.nativeObj, this);
    }

    native isPiPSupportedNative(nativeObj: long): boolean;
    public isPiPSupported(): boolean {
      return this.isPiPSupportedNative(this.nativeObj);
    }

    native startPiPNative(nativeObj: long): void;
    public startPiP(): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.startPiPNative(this.nativeObj);
            }).then((ret: Any) :void => {
                resolve(undefined);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    native stopPiPNative(nativeObj: long): void;
    public stopPiP(): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.stopPiPNative(this.nativeObj);
            }).then((ret: Any) :void => {
                resolve(undefined);
            }).catch((err: Any) :void => {
                reject(err as BusinessError);
            });
        });
    }

    native setAutoStartEnabledNative(nativeObj: long, enable: boolean): void;
    public setAutoStartEnabled(enable: boolean): void {
      this.setAutoStartEnabledNative(this.nativeObj, enable);
    }

    native updateContentSizeNative(nativeObj: long, width: int, height: int): void;
    public updateContentSize(width: int, height: int): void {
      this.updateContentSizeNative(this.nativeObj, width, height);
    }
    
    native updatePiPControlStatusNative(nativeObj: long, controlType: int, status: int): void;
    public updatePiPControlStatus(controlType: PiPControlType, status: PiPControlStatus): void {
      this.updatePiPControlStatusNative(this.nativeObj, controlType as int, status as int);
    }

    native updateContentNodeNative(nativeObj: long, contentNode: typeNode.XComponent): void;
    public updateContentNode(contentNode: typeNode.XComponent): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.updateContentNodeNative(this.nativeObj, contentNode);
          }).then((ret: Any) :void => {
              resolve(undefined);
          }).catch((err: Any) :void => {
              reject(err as BusinessError);
          });
       });
     }

     native setPiPControlEnabledNative(nativeObj: long, controlType: int, enabled: boolean): void;
     public setPiPControlEnabled(controlType: PiPControlType, enabled: boolean): void {
       this.setPiPControlEnabledNative(this.nativeObj, controlType as int, enabled);
     }

     native getPiPWindowInfoNative(nativeObj: long): PiPWindowInfo;
     public getPiPWindowInfo(): Promise<PiPWindowInfo> {
      return new Promise<PiPWindowInfo>((resolve: (value: PiPWindowInfo) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): PiPWindowInfo => {
            let res = this.getPiPWindowInfoNative(this.nativeObj) as PiPWindowInfo;
            return res;
        }).then((ret: Any) :void => {
            resolve(ret as PiPWindowInfo);
        }).catch((err: Any) :void => {
            reject(err as BusinessError);
        });
      });
    }

    native getPiPSettingSwitchNative(nativeObj: long): boolean;
    public getPiPSettingSwitch(): Promise<boolean> {
      return new Promise<boolean>((resolve: (value: boolean) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): boolean => {
            return this.getPiPSettingSwitchNative(this.nativeObj);
        }).then((ret: Any) :void => {
            resolve(ret as boolean);
        }).catch((err: Any) :void => {
            reject(err as BusinessError);
        });
      });
    }

    native isPiPActiveNative(nativeObj: long): boolean;
    public isPiPActive(): Promise<boolean> {
      return new Promise<boolean>((resolve: (value: boolean) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): boolean => {
            return this.isPiPActiveNative(this.nativeObj);
        }).then((ret: Any) :void => {
            resolve(ret as boolean);
        }).catch((err: Any) :void => {
            reject(err as BusinessError);
        });
      });
    }

    native onStateChangeNative(nativeObj: long, callback: Object): void;
    public onStateChange(callback: PiPStateChangeCallback): void {
      if (!this.nativeObj || !callback) {
        return;
      }
      this.onStateChangeNative(this.nativeObj, callback);
    }

    native offStateChangeNative(nativeObj: long, callback?: Object): void;
    public offStateChange(): void {
      if (!this.nativeObj) {
        return;
      }
      this.offStateChangeNative(this.nativeObj);
    }

    native onControlPanelActionEventNative(nativeObj: long, callback: Object): void;
    public onControlPanelActionEvent(callback: ControlPanelActionEventCallback): void {
      if (!this.nativeObj || !callback) {
        return;
      }
      this.onControlPanelActionEventNative(this.nativeObj, callback);
    }

    native offControlPanelActionEventNative(nativeObj: long): void;
    public offControlPanelActionEvent(): void {
      if (!this.nativeObj) {
        return;
      }
      this.offControlPanelActionEventNative(this.nativeObj);
    }

    native onControlEventNative(nativeObj: long, callback: Object): void;
    public onControlEvent(callback: Callback<ControlEventParam>): void {
      if (!this.nativeObj || !callback) {
        return;
      }
      this.onControlEventNative(this.nativeObj, callback);
    }

    native offControlEventNative(nativeObj: long, callback?: Object): void;
    public offControlEvent(callback?: Callback<ControlEventParam>): void {
      if (!this.nativeObj) {
        return;
      }
      this.offControlEventNative(this.nativeObj, callback);
    }

    native onPipWindowSizeChangeNative(nativeObj: long, callback: Object): void;
    public onPipWindowSizeChange(callback: Callback<PiPWindowSize>): void {
      if (!this.nativeObj || !callback) {
        return;
      }
      this.onPipWindowSizeChangeNative(this.nativeObj, callback);
    }

    native offPipWindowSizeChangeNative(nativeObj: long, callback?: Object): void;
    public offPipWindowSizeChange(callback?: Callback<PiPWindowSize>): void {
      if (!this.nativeObj) {
        return;
      }
      this.offPipWindowSizeChangeNative(this.nativeObj, callback);
    }

    native onActiveStatusChangeNative(nativeObj: long, callback: Object): void;
    public onActiveStatusChange(callback: Callback<boolean>): void {
      if (!this.nativeObj || !callback) {
        return;
      }
      this.onActiveStatusChangeNative(this.nativeObj, callback);
    }

    native offActiveStatusChangeNative(nativeObj: long, callback?: Object): void;
    public offActiveStatusChange(callback?: Callback<boolean>): void {
      if (!this.nativeObj) {
        return;
      }
      this.offActiveStatusChangeNative(this.nativeObj, callback);
    }

  }  // class PiPControllerInternal
}  // namespace PiPWindow


// use for PiPContent.ets only
export namespace PiPManager {
  static { loadLibrary('pipwindowani_kit.z') };
  hilog.info(DOMAIN, TAG, 'PiPManager load pipwindowani_kit.z finish');

  let nativeObj: long;
  export function setNativeObj(nativeObject: long): void {
    nativeObj = nativeObject;
  }

  export native function InitXComponentController(nativeObj: long, xComponentController: XComponentController): void;
  export native function GetCustomUIController(nativeObj: long): NodeController;
  export native function GetTypeNode(nativeObj: long): typeNode.XComponent;
  export native function RegisterCallback(nativeObj: long, type: string, callback: OnCallback): void;
  export native function UnregisterCallback(nativeObj: long, type: string): void;
  export native function SetTypeNodeEnabled(nativeObj: long): void;
  export native function SetPipNodeType(nativeObj: long, typeNode: typeNode.XComponent, markPip: boolean): void;

  export function initXComponentController(xComponentController: XComponentController): void {
    return InitXComponentController(nativeObj, xComponentController);
  }

  export function getCustomUIController(): NodeController {
    return GetCustomUIController(nativeObj);
  }

  export function getTypeNode(): typeNode.XComponent {
    return GetTypeNode(nativeObj);
  }

  export function setTypeNodeEnabled(): void {
    return SetTypeNodeEnabled(nativeObj);
  }

  export function setPipNodeType(typeNode: typeNode.XComponent, markPip: boolean): void {
    return SetPipNodeType(nativeObj, typeNode, markPip);
  }

  export function on(type: string, callback: OnCallback): void {
    return RegisterCallback(nativeObj, type, callback);
  }

  export function off(type: string): void {
    return UnregisterCallback(nativeObj, type);
  }

  export function runUpdateNodeCallback(fn: object, fnArg: object): void {
    let f = fn as (d: object) => void;
    f(fnArg);
  }

  export function runStateChangeCallback(fn: object, state: int): void {
    let f = fn as (state: int) => void;
    f(state);
  }
}  // namespace PiPManager