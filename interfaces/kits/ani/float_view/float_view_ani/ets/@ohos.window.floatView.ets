/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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
import type {Callback} from '@ohos.base';
import {BusinessError} from '@ohos.base';
import type floatingBall from '@ohos.window.floatingBall';
import window from '@ohos.window'
import { LocalStorage } from '@ohos.arkui.stateManagement';

export type CallbackVoid<T> = (data: T) => void;
export type CallbackState = (state: int) => void;

export default namespace floatView {
  static { loadLibrary('fvwindowani_kit.z'); }
  
  export interface FloatViewController {
    start(): Promise<void>;
    stop(): Promise<void>;
    setUIContext(path: string, contentStorage?: LocalStorage): Promise<void>;
    setFloatViewVisibilityInApp(visibleInApp: boolean): Promise<void>;
    setWindowSize(size: window.Size): Promise<void>;
    getWindowProperties(): FloatViewProperties;
    restoreMainWindow(wantParameters?: Record<string, object>): Promise<void>;
    onStateChange(callback: Callback<FloatViewStateChangeInfo>): void;
    offStateChange(callback?: Callback<FloatViewStateChangeInfo>): void;
    onRectChange(callback: Callback<FloatViewRectChangeInfo>): void;
    offRectChange(callback?: Callback<FloatViewRectChangeInfo>): void;
    onLimitsChange(callback: Callback<FloatViewLimits>): void;
    offLimitsChange(callback?: Callback<FloatViewLimits>): void;
  }

  export interface FloatViewConfiguration {
      context: common.BaseContext;
      templateType: FloatViewTemplateType;
  }

  export interface RatioLimit {
    minRatio: double;
    maxRatio: double;
  }

  export class RatioLimitInner implements RatioLimit {
    minRatio: double = 0;
    maxRatio: double = 0;
    constructor(minRatio: double, maxRatio: double) {
      this.minRatio = minRatio;
      this.maxRatio = maxRatio;
    }
  }

  export interface FloatViewProperties {
    templateType: FloatViewTemplateType;
    windowId: int;
    displayId: int;
    windowRect: window.Rect;
    windowScale: double;
    avoidArea: window.AvoidArea;
    inSidebar: boolean;
  }

  export class FloatViewPropertiesInner implements FloatViewProperties {
    templateType: FloatViewTemplateType;
    windowId: int;
    displayId: int;
    windowRect: window.Rect;
    windowScale: double;
    avoidArea: window.AvoidArea;
    inSidebar: boolean;
    constructor(templateType: int, windowId: int, displayId: int,
      windowRect: window.Rect, scale: double, avoidArea: window.AvoidArea, sidebar: boolean) {
      this.templateType = FloatViewTemplateType.fromValue(templateType);
      this.windowId = windowId;
      this.displayId = displayId;
      this.windowRect = windowRect;
      this.windowScale = scale;
      this.avoidArea = avoidArea;
      this.inSidebar = sidebar;
    }
  }

  export interface FloatViewStateChangeInfo {
    state: FloatViewState;
    stopReason: string;
  }

  export class FloatViewStateChangeInfoInner implements FloatViewStateChangeInfo {
    state: FloatViewState;
    stopReason: string;
    constructor(state: int, reason: string) {
      this.state = FloatViewState.fromValue(state);
      this.stopReason = reason;
    }
  }

  export interface FloatViewRectChangeInfo {
    windowRect: window.Rect;
    windowScale: double;
    reason: string;
  }

  export class FloatViewRectChangeInfoInner implements FloatViewRectChangeInfo {
    windowRect: window.Rect;
    windowScale: double = 0;
    reason: string = "";
    constructor(rect: window.Rect, scale: double, reason: string) {
      this.windowRect = rect;
      this.windowScale = scale;
      this.reason = reason;
    }
  }

  export interface FloatViewLimits {
    minSize: window.Size;
    maxSize: window.Size;
    ratioLimits: Array<RatioLimit>;
  }

  export class FloatViewLimitsInner implements FloatViewLimits {
    minSize: window.Size;
    maxSize: window.Size;
    ratioLimits: Array<RatioLimit>;
    constructor(min: window.Size, max: window.Size, limitList: Array<RatioLimit>) {
      this.minSize = min;
      this.maxSize = max;
      this.ratioLimits = limitList;
    }
  }

  export enum FloatViewState {
    STARTED = 1,
    HIDDEN = 2,
    STOPPED = 3,
    IN_SIDEBAR = 4,
    IN_FLOATING_BALL = 5,
    ERROR = 6,
  }

  export enum FloatViewTemplateType {
    ROUNDED_RECTANGLE = 0,
  }

  function runOnStateChangeEvent(cb: object, info: FloatViewStateChangeInfo): void {
    const func = cb as (info: FloatViewStateChangeInfo) => void;
    func(info);
  }

  function runOnRectChangeEvent(cb: object, rect: FloatViewRectChangeInfo): void {
    const func = cb as (rect: FloatViewRectChangeInfo) => void;
    func(rect);
  }

  function runOnLimitsChangeEvent(cb: object, limits: FloatViewLimits): void {
    const func = cb as (limits: FloatViewLimits) => void;
    func(limits);
  }

  // nativeObj : the address of AniFvController
  let nativeObj: long = 0;
  export function setNativeObj(nativeObject: long): void {
    nativeObj = nativeObject;
  }

  export native function createSync(nativeObj: long, config: FloatViewConfiguration): FloatViewController;
  function create(config: FloatViewConfiguration): Promise<FloatViewController> {
    return new Promise<FloatViewController>(
      (resolve: (value: FloatViewController) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): FloatViewController => {
            return createSync(nativeObj, config);
        }).then((ret: Any) => {
            resolve(ret as FloatViewController);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
  }

  export native function isFloatViewEnabled(): boolean;
  export native function getFloatViewLimits(templateType: FloatViewTemplateType): FloatViewLimits;

  export native function bindSync(fvController: FloatViewController,
    fbController: floatingBall.FloatingBallController, fbOption: floatingBall.FloatingBallParams): void;
  function bind(fvController: FloatViewController,
    fbController: floatingBall.FloatingBallController, fbOption: floatingBall.FloatingBallParams): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute((): void => {
          bindSync(fvController, fbController, fbOption);
      }).then((ret: Any) => {
          resolve(undefined);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
    });
  }

  export native function unbindSync(fvController: FloatViewController,
    fbController: floatingBall.FloatingBallController): void;
  function unbind(fvController: FloatViewController, fbController: floatingBall.FloatingBallController): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
      taskpool.execute((): void => {
          unbindSync(fvController, fbController);
      }).then((ret: Any) => {
          resolve(undefined);
      }).catch((err: Any) => {
          reject(err as BusinessError);
      });
    });
  }

  export native function fvControllerFinalizerCallback(nativeObj: long): void;
  let fvControllerFinalizerRegister = new FinalizationRegistry<long>(fvControllerFinalizerCallback);

  export class FvControllerInternal implements FloatViewController {
    private nativeObj: long = 0;
    public setNativeObj(nativeObject: long): void {
      if (this.nativeObj) {
        fvControllerFinalizerRegister.unregister(this);
      }
      this.nativeObj = nativeObject;
      fvControllerFinalizerRegister.register(this, this.nativeObj, this);
    }

    public native startFloatViewNative(nativeObj: long): void;
    public start(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        try {
          this.startFloatViewNative(this.nativeObj);
          resolve(undefined);
        } catch (err) {
          reject(err as BusinessError);
        }
      });
    }

    public native stopFloatViewNative(nativeObj: long): void;
    public stop(): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.stopFloatViewNative(this.nativeObj);
        }).then((ret: Any) :void => {
          resolve(undefined);
        }).catch((err: Any) :void => {
          reject(err as BusinessError);
        });
      });
    }

    public native setUIContextNative(nativeObj: long, path: string, contentStorage?: LocalStorage): void;
    public setUIContext(path: string, contentStorage?: LocalStorage): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        try {
          this.setUIContextNative(this.nativeObj, path, contentStorage);
          resolve(undefined);
        } catch (err) {
          reject(err as BusinessError);
        }
      });
    }

    public native setFloatViewVisibilityInAppNative(nativeObj: long, visibleInApp: boolean): void;
    public setFloatViewVisibilityInApp(visibleInApp: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setFloatViewVisibilityInAppNative(this.nativeObj, visibleInApp);
        }).then((ret: Any) :void => {
          resolve(undefined);
        }).catch((err: Any) :void => {
          reject(err as BusinessError);
        });
      });
    }

    public native setWindowSizeNative(nativeObj: long, size: window.Size): void;
    public setWindowSize(size: window.Size): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.setWindowSizeNative(this.nativeObj, size);
        }).then((ret: Any) :void => {
          resolve(undefined);
        }).catch((err: Any) :void => {
          reject(err as BusinessError);
        });
      });
    }

    native getWindowPropertiesNative(nativeObj: long): FloatViewProperties;
    public getWindowProperties(): FloatViewProperties {
      let res = this.getWindowPropertiesNative(this.nativeObj) as FloatViewProperties;
      return res;
    }

    public native restoreMainWindowNative(nativeObj: long, wantParameters?: Record<string, object>): void;
    public restoreMainWindow(wantParameters?: Record<string, object>): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.restoreMainWindowNative(this.nativeObj, wantParameters);
        }).then((ret: Any) :void => {
          resolve(undefined);
        }).catch((err: Any) :void => {
          reject(err as BusinessError);
        });
      });
    }

    native onStateChangeNative(nativeObj: long, callback: Object): void;
    public onStateChange(callback: Callback<FloatViewStateChangeInfo>): void {
      if (!this.nativeObj || !callback) {
        return;
      }
      this.onStateChangeNative(this.nativeObj, callback);
    }

    native offStateChangeNative(nativeObj: long, callback?: Object): void;
    public offStateChange(callback?: Callback<FloatViewStateChangeInfo>): void {
      if (!this.nativeObj) {
        return;
      }
      this.offStateChangeNative(this.nativeObj, callback);
    }

    native onRectChangeNative(nativeObj: long, callback: Object): void;
    public onRectChange(callback: Callback<FloatViewRectChangeInfo>): void {
      if (!this.nativeObj || !callback) {
        return;
      }
      this.onRectChangeNative(this.nativeObj, callback);
    }

    native offRectChangeNative(nativeObj: long, callback?: Object): void;
    public offRectChange(callback?: Callback<FloatViewRectChangeInfo>): void {
      if (!this.nativeObj) {
        return;
      }
      this.offRectChangeNative(this.nativeObj, callback);
    }

    native onLimitsChangeNative(nativeObj: long, callback: Object): void;
    public onLimitsChange(callback: Callback<FloatViewLimits>): void {
      if (!this.nativeObj || !callback) {
        return;
      }
      this.onLimitsChangeNative(this.nativeObj, callback);
    }

    native offLimitsChangeNative(nativeObj: long, callback?: Object): void;
    public offLimitsChange(callback?: Callback<FloatViewLimits>): void {
      if (!this.nativeObj) {
        return;
      }
      this.offLimitsChangeNative(this.nativeObj, callback);
    }
  }  // class FvControllerInternal
}  // namespace floatView