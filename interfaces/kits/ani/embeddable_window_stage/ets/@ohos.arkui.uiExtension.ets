/*
 * Copyright (c) 2025-2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { BusinessError } from '@ohos.base';
import hilog from '@ohos.hilog';
import window from '@ohos.window';
import { Callback } from '@ohos.base';

/**
 * uiExtension.
 *
 * @namespace uiExtension
 * @syscap SystemCapability.ArkUI.ArkUI.Full
 * @atomicservice
 * @since 12
 */
namespace uiExtension {
  // UIExtension WmsLogTag
  const WMS_UIEXT: int = 0xC0420D;
  const WMS_UIEXT_TAG: string = "WMSUiext";
  const WINDOW_SIZE_CHANGE_CB: string = "windowSizeChange";
  const AVOID_AREA_CHANGE_CB: string = "avoidAreaChange";
  const WINDOW_RECT_CHANGE_CB: string = "rectChange";
  const enum WmErrorCode {
    WM_OK = 0,
    WM_ERROR_INVALID_PARAM = 401
  }

  export type CallbackRect = (area: RectChangeOptions) => void;

  function windowRectChangeCallback(cb: object, rect: object, reason: int): void {
    const func = cb as (cbArg: RectChangeOptions) => void;
    func({ rect: rect as window.Rect, reason: reason as RectChangeReason });
  }

  function avoidAreaChangeCallback(cb: object, area: object, type: int): void {
    const func = cb as (data: AvoidAreaInfo) => void;
    let v: AvoidAreaInfo = {type: type as window.AvoidAreaType, area: area as window.AvoidArea}
    func(v);
  }

  class UIEProperties implements WindowProxyProperties {
    constructor() {
      this.uiExtensionHostWindowProxyRect = new UIERect();
    }

    uiExtensionHostWindowProxyRect: window.Rect;
  }

  class UIERect implements window.Rect {
    left: int;
    top: int;
    width: int;
    height: int;
  }

  class UIEAvoidArea implements window.AvoidArea {
    constructor() {
      this.leftRect = new UIERect();
      this.topRect = new UIERect();
      this.rightRect = new UIERect();
      this.bottomRect = new UIERect();
    }

    visible: boolean;
    leftRect: window.Rect;
    topRect: window.Rect;
    rightRect: window.Rect;
    bottomRect: window.Rect;
  }

  export native function destroyWindowProxy(nativeObj: long): void;
  let windowProxyRegister = new FinalizationRegistry<long>(destroyWindowProxy);

  // API implementation
  export class UIExtensionInternal implements WindowProxy {
    static { loadLibrary("embeddablewindowstageani_module.z") }
    private nativeObj: long = 0;

    private setNativeObj(nativeObj: long): void {
      this.nativeObj = nativeObj;
      windowProxyRegister.register(this, nativeObj);
    }

    public native getProperties(nativeObj: long, property: object): int;
    public native setProperties(nativeObj: long, property: object): int;
    public native getWindowAvoidArea(nativeObj: long, type: int, area: object): int;
    public native setWaterMarkFlag(nativeObj: long, enable: boolean): int;
    public native createSubWindowWithOptions(nativeObj: long, name: string,
        subWindowOptions: window.SubWindowOptions, followCreatorLifecycle: boolean): window.Window;
    public native occupyEvents(nativeObj: long, eventFlags: int): void;
    public native hideNonSecureWindows(nativeObj: long, shouldHide: boolean): int;
    public native onSync(nativeObj: long, type: string, callback: object): void;
    public native offSync(nativeObj: long, type: string, callback?: object): void;
    public native onRectChange(nativeObj: long, reason: int, callback?: object): void;
    public native offRectChange(nativeObj: long, callback?: object): void;

    get properties(): WindowProxyProperties {
      let property = new UIEProperties();
      let ret = this.getProperties(this.nativeObj, property as object);
      if (ret !== WmErrorCode.WM_OK) {
        hilog.error(WMS_UIEXT, WMS_UIEXT_TAG, `getProperties failed, ret: ${ret}`);
      }
      return property;
    }

    set properties(properties: WindowProxyProperties) {
      this.setProperties(this.nativeObj, properties as object);
    }

    public on<T>(type: string, callback: Callback<T>): void {
      this.onSync(this.nativeObj, type, callback);
    }

    public off<T>(type: string, callback?: Callback<T>): void {
      this.offSync(this.nativeObj, type, callback);
    }

    public onAvoidAreaChange(callback: Callback<AvoidAreaInfo>): void {
      this.onSync(this.nativeObj, AVOID_AREA_CHANGE_CB, callback);
    }

    public offAvoidAreaChange(callback?: Callback<AvoidAreaInfo>): void {
      this.offSync(this.nativeObj, AVOID_AREA_CHANGE_CB, callback);
    }

    public onWindowSizeChange(callback: Callback<window.Size>): void {
      this.onSync(this.nativeObj, WINDOW_SIZE_CHANGE_CB, callback);
    }

    public offWindowSizeChange(callback?: Callback<window.Size>): void {
      this.offSync(this.nativeObj, WINDOW_SIZE_CHANGE_CB, callback);
    }

    public onRectChange(reason: int, callback: Callback<RectChangeOptions>): void {
      this.onRectChange(this.nativeObj, reason, callback);
    }

    public offRectChange(callback?: Callback<RectChangeOptions>): void {
      this.offRectChange(this.nativeObj, callback);
    }

    public getWindowAvoidArea(type: window.AvoidAreaType): window.AvoidArea {
      let area: window.AvoidArea = new UIEAvoidArea();
      let ret = this.getWindowAvoidArea(this.nativeObj, type as int, area as object);
      if (ret !== WmErrorCode.WM_OK) {
        hilog.error(WMS_UIEXT, WMS_UIEXT_TAG, `getWindowAvoidArea failed, ret: ${ret}`);
      }
      return area;
    }

    public setWaterMarkFlag(enable: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          let ret = this.setWaterMarkFlag(this.nativeObj, enable);
          if (ret !== WmErrorCode.WM_OK) {
            hilog.error(WMS_UIEXT, WMS_UIEXT_TAG, `setWaterMarkFlag failed, ret: ${ret}`);
          }
        }).then((ret: Any) :void => {
          resolve(undefined);
        }).catch((err: Any) :void => {
          reject(err as BusinessError);
        });
      });
    }

    public hideNonSecureWindows(shouldHide: boolean): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          let ret = this.hideNonSecureWindows(this.nativeObj, shouldHide);
          if (ret !== WmErrorCode.WM_OK) {
            hilog.error(WMS_UIEXT, WMS_UIEXT_TAG, `hideNonSecureWindows failed, ret: ${ret}`);
          }
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }

    public createSubWindowWithOptions(name: string, subWindowOptions: window.SubWindowOptions): Promise<window.Window> {
      return new Promise<window.Window>((resolve: (value: window.Window) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): window.Window => {
          return this.createSubWindowWithOptions(this.nativeObj, name, subWindowOptions, false);
        }).then((ret: Any) => {
            resolve(ret as window.Window);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
      });
    }

    public createSubWindowWithOptions(name: string, subWindowConfig: window.SubWindowOptions,
      followCreatorLifecycle: boolean): Promise<window.Window> {
      return new Promise<window.Window>((resolve: (value: window.Window) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): window.Window => {
          return this.createSubWindowWithOptions(this.nativeObj, name, subWindowConfig, followCreatorLifecycle);
        }).then((ret: Any) => {
            resolve(ret as window.Window);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
      });
    }

    public occupyEvents(eventFlags: int): Promise<void> {
      return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
          this.occupyEvents(this.nativeObj, eventFlags);
        }).then((ret: Any) => {
          resolve(undefined);
        }).catch((err: Any) => {
          reject(err as BusinessError);
        });
      });
    }
  }

  /**
   * Defines the avoid area information.
   *
   * @interface AvoidAreaInfo
   * @syscap SystemCapability.ArkUI.ArkUI.Full
   * @atomicservice
   * @since 12
   */
  export interface AvoidAreaInfo {
    type: window.AvoidAreaType;
    area: window.AvoidArea;
  }

  /**
   * Defines the reason of component rect change
   *
   * @enum { number }
   * @syscap SystemCapability.ArkUI.ArkUI.Full
   * @atomicservice
   * @since 14
   */
  export enum RectChangeReason {
    HOST_WINDOW_RECT_CHANGE = 0x0001,
  }

  /**
   * Gesture event type
   *
   * @enum { number }
   * @syscap SystemCapability.ArkUI.ArkUI.Full
   * @atomicservice
   * @since 18
   */
  export enum EventFlag {
    /**
     * None gesture event
     *
     * @syscap SystemCapability.ArkUI.ArkUI.Full
     * @atomicservice
     * @since 18
     */
    EVENT_NONE = 0x00000000,
    /**
     * Pan gesture left events
     *
     * @syscap SystemCapability.ArkUI.ArkUI.Full
     * @atomicservice
     * @since 18
     */
    EVENT_PAN_GESTURE_LEFT = 0x00000001,
    /**
     * Pan gesture right events
     *
     * @syscap SystemCapability.ArkUI.ArkUI.Full
     * @atomicservice
     * @since 18
     */
    EVENT_PAN_GESTURE_RIGHT = 0x00000002,
    /**
     * Pan gesture up events
     *
     * @syscap SystemCapability.ArkUI.ArkUI.Full
     * @atomicservice
     * @since 18
     */
    EVENT_PAN_GESTURE_UP = 0x00000004,
    /**
     * Pan gesture down events
     *
     * @syscap SystemCapability.ArkUI.ArkUI.Full
     * @atomicservice
     * @since 18
     */
    EVENT_PAN_GESTURE_DOWN = 0x00000008,
    /**
     * Click gesture events
     *
     * @syscap SystemCapability.ArkUI.ArkUI.Full
     * @atomicservice
     * @since 18
     */
    EVENT_CLICK = 0x00000100,
    /**
     * Long press gesture events
     *
     * @syscap SystemCapability.ArkUI.ArkUI.Full
     * @atomicservice
     * @since 18
     */
    EVENT_LONG_PRESS = 0x00000200,
  }

  /**
   * Rect change options
   *
   * @interface RectChangeOptions
   * @syscap SystemCapability.ArkUI.ArkUI.Full
   * @atomicservice
   * @since 14
   */
  export interface RectChangeOptions {
    rect: window.Rect,
    reason: RectChangeReason
  }

  /**
   * The proxy of the UIExtension window.
   *
   * @interface WindowProxy
   * @syscap SystemCapability.ArkUI.ArkUI.Full
   * @atomicservice
   * @since 12
   */
  export interface WindowProxy {
    getWindowAvoidArea(type: window.AvoidAreaType): window.AvoidArea;
    setWaterMarkFlag(enable: boolean): Promise<void>;
    hideNonSecureWindows(shouldHide: boolean): Promise<void>;
    createSubWindowWithOptions(name: string, subWindowOptions: window.SubWindowOptions): Promise<window.Window>;
    createSubWindowWithOptions(name: string, subWindowConfig: window.SubWindowOptions,
      followCreatorLifecycle: boolean): Promise<window.Window>;
    occupyEvents(eventFlags: int): Promise<void>;
    on<T>(type: string, callback: Callback<T>): void;
    off<T>(type: string, callback?: Callback<T>): void;
    onAvoidAreaChange(callback: Callback<AvoidAreaInfo>): void;
    offAvoidAreaChange(callback?: Callback<AvoidAreaInfo>): void;
    onWindowSizeChange(callback: Callback<window.Size>): void;
    offWindowSizeChange(callback?: Callback<window.Size>): void;
    onRectChange(reasons: int, callback: Callback<RectChangeOptions>): void;
    offRectChange(callback?: Callback<RectChangeOptions>): void;
    properties: WindowProxyProperties;
  }

  /**
   * Properties of UIExtension window
   *
   * @interface WindowProxyProperties
   * @syscap SystemCapability.ArkUI.ArkUI.Full
   * @atomicservice
   * @since 14
   * 
   */
  export interface WindowProxyProperties {
    /**
     * Position and size of the component (EmbeddedComponent or UIExtensionComponent).
     *
     * @type { window.Rect } 
     * @syscap SystemCapability.ArkUI.ArkUI.Full
     * @atomicservice
     * @since 14
     */
    uiExtensionHostWindowProxyRect: window.Rect;
  }
}

export default uiExtension;