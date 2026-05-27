/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

import hilog from '@ohos.hilog';
import window from '@ohos.window';
import uiExtension from '@ohos.arkui.uiExtension';
import { UIContext } from '@ohos.arkui.UIContext';
import type observer from '@ohos.arkui.observer';

type Callback<T> = (value: T) => void;

const ATTR_DOMAIN = 0x04217;
const ATTR_TAG = 'WMSAttribute';
const FOCUS_DOMAIN = 0x04207;
const FOCUS_TAG = 'WMSFocus';
const IMMS_DOMAIN = 0x04209;
const IMMS_TAG = 'WMSImms';
const LAYOUT_DOMAIN = 0x04208;
const LAYOUT_TAG = 'WMSLayout';

namespace env_native {
    export native function findWindowById(instanceId: long): Object;
    export native function getDisplayId(instanceId: long): long;
    export native function getWindowProperties(instanceId: long): window.WindowProperties;
    export native function isFocused(instanceId: long): boolean;
    export native function isHighlighted(instanceId: long): boolean;
}

export class WindowStaticEnv {
    static { loadLibrary('window_static_env_module.z'); }

    private instanceId: long;
    private context: UIContext;
    private window: window.Window | undefined = undefined;
    private windowProxy: uiExtension.WindowProxy | undefined = undefined;
    private windowEventCb: Callback<window.WindowEventType> | undefined;
    private cacheEnvAvoidArea: window.UIEnvWindowAvoidAreaInfoPX | undefined;
    private avoidAreaCbVp: Callback<window.AvoidAreaOptions> | undefined;
    private avoidAreaCbVpForUIE: Callback<uiExtension.AvoidAreaInfo> | undefined;
    private avoidAreaDensityCbVp: Callback<observer.DensityInfo> | undefined;
    private avoidAreaCbPx: Callback<window.AvoidAreaOptions> | undefined;
    private avoidAreaCbPxForUIE: Callback<uiExtension.AvoidAreaInfo> | undefined;
    private cacheWindowSize: window.Size | undefined;
    private windowSizeCbVp: Callback<window.Size> | undefined;
    private windowSizeDensityCbVp: Callback<observer.DensityInfo> | undefined;

    constructor(instanceId: long, context: UIContext) {
        this.instanceId = instanceId;
        this.context = context;
        try {
            const win: Object = env_native.findWindowById(instanceId);
            if (win === undefined) {
                hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] window is undefined, instanceId: ${instanceId}`);
            } else if (win instanceof window.Window) {
                this.window = win as window.Window;
            } else if (win instanceof uiExtension.WindowProxy) {
                this.windowProxy = win as uiExtension.WindowProxy;
            }
        } catch (error) {
            hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] env constructor failed, ${error.message}`);
        }
    }

    getEnvValueByKey(key: string): Any {
        switch (key) {
            case 'system.window.avoidarea':
                this.initEnvAvoidArea();
                return this.translateEnvAvoidAreaToVp(this.cacheEnvAvoidArea!);
            case 'system.window.avoidarea.px':
                this.initEnvAvoidArea();
                return this.cacheEnvAvoidArea!;
            case 'system.window.size':
                const sizeVpProps: window.WindowProperties = env_native.getWindowProperties(this.instanceId);
                const sizeVp: window.SizeInVP = {
                    width: this.context.px2vp(sizeVpProps.windowRect.width),
                    height: this.context.px2vp(sizeVpProps.windowRect.height),
                };
                return sizeVp;
            case 'system.window.size.px':
                const sizePxProps: window.WindowProperties = env_native.getWindowProperties(this.instanceId);
                const sizePx: window.Size = {
                    width: sizePxProps.windowRect.width,
                    height: sizePxProps.windowRect.height,
                };
                return sizePx;
            case 'system.window.focused':
                return env_native.isFocused(this.instanceId);
            case 'system.window.highlighted':
                return env_native.isHighlighted(this.instanceId);
            case 'system.window.density.system':
                const windowDensityInfo: window.WindowDensityInfo | undefined = this.window?.getWindowDensityInfo();
                return windowDensityInfo?.systemDensity ?? Int.toDouble(-1);
            case 'system.window.displayid':
                return env_native.getDisplayId(this.instanceId);
            default:
                return undefined;
        }
    }

    registerDisplayIdEnv(cb: Callback<long>): void {
        try {
            this.window?.onDisplayIdChange(cb);
        } catch (error) {
            hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] DisplayIdEnv register failed, ${error.message}`);
        }
    }

    unregisterDisplayIdEnv(cb: Callback<long>): void {
        try {
            this.window?.offDisplayIdChange(cb);
        } catch (error) {
            hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] DisplayIdEnv unregister failed, ${error.message}`);
        }
    }

    registerSystemDensityEnv(cb: Callback<double>): void {
        try {
            this.window?.onSystemDensityChange(cb);
        } catch (error) {
            hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] SystemDensityEnv register failed, ${error.message}`);
        }
    }

    unregisterSystemDensityEnv(cb: Callback<double>): void {
        try {
            this.window?.offSystemDensityChange(cb);
        } catch (error) {
            hilog.error(ATTR_DOMAIN, ATTR_TAG, `[env] SystemDensityEnv unregister failed, ${error.message}`);
        }
    }

    registerFocusEnv(cb: Callback<boolean>): void {
        try {
            if (!this.windowEventCb) {
                this.windowEventCb = (windowEventType: window.WindowEventType) => {
                    cb(windowEventType === window.WindowEventType.WINDOW_ACTIVE);
                }
                this.window?.onWindowEvent(this.windowEventCb!);
            }
        } catch (error) {
            hilog.error(FOCUS_DOMAIN, FOCUS_TAG, `[env] FocusEnv register failed, ${error.message}`);
        }
    }

    unregisterFocusEnv(cb: Callback<boolean>): void {
        try {
            if (this.windowEventCb) {
                this.window?.offWindowEvent(this.windowEventCb!);
                this.windowEventCb = undefined;
            }
        } catch (error) {
            hilog.error(FOCUS_DOMAIN, FOCUS_TAG, `[env] FocusEnv unregister failed, ${error.message}`);
        }
    }

    registerHighlightedEnv(cb: Callback<boolean>): void {
        try {
            this.window?.onWindowHighlightChange(cb);
        } catch (error) {
            hilog.error(FOCUS_DOMAIN, FOCUS_TAG, `[env] HighlightedEnv register failed, ${error.message}`);
        }
    }

    unregisterHighlightedEnv(cb: Callback<boolean>): void {
        try {
            this.window?.offWindowHighlightChange(cb);
        } catch (error) {
            hilog.error(FOCUS_DOMAIN, FOCUS_TAG , `[env] HighlightedEnv unregister failed, ${error.message}`);
        }
    }

    private initEnvAvoidArea(): void {
        const win: window.Window | uiExtension.WindowProxy | undefined = this.window ?? this.windowProxy;
        if (!this.cacheEnvAvoidArea) {
            const envObj: window.UIEnvWindowAvoidAreaInfoPX = {
                statusBar: win?.getWindowAvoidArea(window.AvoidAreaType.TYPE_SYSTEM) ?? this.createEmptyAvoidArea(),
                cutout: win?.getWindowAvoidArea(window.AvoidAreaType.TYPE_CUTOUT) ?? this.createEmptyAvoidArea(),
                keyboard: win?.getWindowAvoidArea(window.AvoidAreaType.TYPE_KEYBOARD) ?? this.createEmptyAvoidArea(),
                navigationIndicator: win?.getWindowAvoidArea(window.AvoidAreaType.TYPE_NAVIGATION_INDICATOR) ??
                    this.createEmptyAvoidArea(),
            };
            this.cacheEnvAvoidArea = envObj;
        }
    }

    private createEmptyAvoidArea(): window.AvoidArea {
        return {
            visible: false,
            leftRect: { left: 0, top: 0, width: 0, height: 0 },
            topRect: { left: 0, top: 0, width: 0, height: 0 },
            rightRect: { left: 0, top: 0, width: 0, height: 0 },
            bottomRect: { left: 0, top: 0, width: 0, height: 0 },
        }
    }

    private translateRectToVp(rect: window.Rect): window.RectInVP {
        return {
            left: this.context.px2vp(rect.left) ?? Int.toDouble(rect.left),
            top: this.context.px2vp(rect.top) ?? Int.toDouble(rect.top),
            width: this.context.px2vp(rect.width) ?? Int.toDouble(rect.width),
            height: this.context.px2vp(rect.height) ?? Int.toDouble(rect.height),
        }
    }

    private translateAvoidAreaToVp(area: window.AvoidArea): window.UIEnvAvoidAreaVP {
        return {
            visible: area.visible,
            leftRect: this.translateRectToVp(area.leftRect),
            topRect: this.translateRectToVp(area.topRect),
            rightRect: this.translateRectToVp(area.rightRect),
            bottomRect: this.translateRectToVp(area.bottomRect),
        }
    }

    private translateEnvAvoidAreaToVp(envObj: window.UIEnvWindowAvoidAreaInfoPX): window.UIEnvWindowAvoidAreaInfoVP {
        return {
            statusBar: this.translateAvoidAreaToVp(envObj.statusBar),
            cutout: this.translateAvoidAreaToVp(envObj.cutout),
            keyboard: this.translateAvoidAreaToVp(envObj.keyboard),
            navigationIndicator: this.translateAvoidAreaToVp(envObj.navigationIndicator),
        }
    }

    registerWindowAvoidAreaVpEnv(cb: Callback<window.UIEnvWindowAvoidAreaInfoVP>): void {
        this.initEnvAvoidArea();
        if (!this.avoidAreaCbVp && !this.avoidAreaCbVpForUIE && !this.avoidAreaDensityCbVp) {
            const handler = (options: window.AvoidAreaOptions): void => {
                switch (options.type) {
                    case window.AvoidAreaType.TYPE_SYSTEM:
                        this.cacheEnvAvoidArea!.statusBar = options.area;
                        break;
                    case window.AvoidAreaType.TYPE_CUTOUT:
                        this.cacheEnvAvoidArea!.cutout = options.area;
                        break;
                    case window.AvoidAreaType.TYPE_KEYBOARD:
                        this.cacheEnvAvoidArea!.keyboard = options.area;
                        break;
                    case window.AvoidAreaType.TYPE_NAVIGATION_INDICATOR:
                        this.cacheEnvAvoidArea!.navigationIndicator = options.area;
                        break;
                }
                cb(this.translateEnvAvoidAreaToVp(this.cacheEnvAvoidArea!));
            }
            this.avoidAreaCbVp = (options: window.AvoidAreaOptions) => {
                handler(options);
            }
            this.avoidAreaCbVpForUIE = (info: uiExtension.AvoidAreaInfo) => {
                const options: window.AvoidAreaOptions = { type: info.type, area: info.area };
                handler(options);
            }
            this.avoidAreaDensityCbVp = (info: observer.DensityInfo) => {
                cb(this.translateEnvAvoidAreaToVp(this.cacheEnvAvoidArea!));
            }
            try {
                this.window?.onAvoidAreaChange(this.avoidAreaCbVp!);
                this.windowProxy?.onAvoidAreaChange(this.avoidAreaCbVpForUIE!);
                this.context.getUIObserver().onDensityUpdate(this.avoidAreaDensityCbVp!);
            } catch (error) {
                hilog.error(IMMS_DOMAIN, IMMS_TAG, `[env] AvoidAreaVpEnv register failed, ${error.message}`);
            }
        }
    }

    unregisterWindowAvoidAreaVpEnv(cb: Callback<window.UIEnvWindowAvoidAreaInfoVP>): void {
        try {
            if (this.avoidAreaCbVp) {
                this.window?.offAvoidAreaChange(this.avoidAreaCbVp!);
                this.avoidAreaCbVp = undefined;
            }
            if (this.avoidAreaCbVpForUIE) {
                this.windowProxy?.offAvoidAreaChange(this.avoidAreaCbVpForUIE!);
                this.avoidAreaCbVpForUIE = undefined;
            }
            if (this.avoidAreaDensityCbVp) {
                this.context.getUIObserver().offDensityUpdate(this.avoidAreaDensityCbVp!);
                this.avoidAreaDensityCbVp = undefined;
            }
        } catch (error) {
            hilog.error(IMMS_DOMAIN, IMMS_TAG, `[env] AvoidAreaVpEnv unregister failed, ${error.message}`);
        }
    }

    registerWindowAvoidAreaPxEnv(cb: Callback<window.UIEnvWindowAvoidAreaInfoPX>): void {
        this.initEnvAvoidArea();
        if (!this.avoidAreaCbPx && !this.avoidAreaCbPxForUIE) {
            const handler = (options: window.AvoidAreaOptions): void => {
                switch (options.type) {
                    case window.AvoidAreaType.TYPE_SYSTEM:
                        this.cacheEnvAvoidArea!.statusBar = options.area;
                        break;
                    case window.AvoidAreaType.TYPE_CUTOUT:
                        this.cacheEnvAvoidArea!.cutout = options.area;
                        break;
                    case window.AvoidAreaType.TYPE_KEYBOARD:
                        this.cacheEnvAvoidArea!.keyboard = options.area;
                        break;
                    case window.AvoidAreaType.TYPE_NAVIGATION_INDICATOR:
                        this.cacheEnvAvoidArea!.navigationIndicator = options.area;
                        break;
                }
                const newEnvObj: window.UIEnvWindowAvoidAreaInfoPX = {
                    statusBar: this.cacheEnvAvoidArea!.statusBar,
                    cutout: this.cacheEnvAvoidArea!.cutout,
                    keyboard: this.cacheEnvAvoidArea!.keyboard,
                    navigationIndicator: this.cacheEnvAvoidArea!.navigationIndicator,
                }
                cb(newEnvObj);
            }
            this.avoidAreaCbPx = (options: window.AvoidAreaOptions) => {
                handler(options);
            }
            this.avoidAreaCbPxForUIE = (info: uiExtension.AvoidAreaInfo) => {
                const options: window.AvoidAreaOptions = { type: info.type, area: info.area };
                handler(options);
            }
            try {
                this.window?.onAvoidAreaChange(this.avoidAreaCbPx!);
                this.windowProxy?.onAvoidAreaChange(this.avoidAreaCbPxForUIE!);
            } catch (error) {
                hilog.error(IMMS_DOMAIN, IMMS_TAG, `[env] AvoidAreaPxEnv register failed, ${error.message}`);
            }
        }
    }

    unregisterWindowAvoidAreaPxEnv(cb: Callback<window.UIEnvWindowAvoidAreaInfoPX>): void {
        const win: window.Window | uiExtension.WindowProxy | undefined = this.window ?? this.windowProxy;
        try {
            if (this.avoidAreaCbPx) {
                this.window?.offAvoidAreaChange(this.avoidAreaCbPx!);
                this.avoidAreaCbPx = undefined;
            }
            if (this.avoidAreaCbPxForUIE) {
                this.windowProxy?.offAvoidAreaChange(this.avoidAreaCbPxForUIE!);
                this.avoidAreaCbPxForUIE = undefined;
            }
        } catch (error) {
            hilog.error(IMMS_DOMAIN, IMMS_TAG, `[env] AvoidAreaPxEnv unregister failed, ${error.message}`);
        }
    }

    registerWindowSizeVpEnv(cb: Callback<window.SizeInVP>): void {
        const win: window.Window | uiExtension.WindowProxy | undefined = this.window ?? this.windowProxy;
        if (!this.windowSizeCbVp && !this.windowSizeDensityCbVp) {
            this.windowSizeCbVp = (size: window.Size) => {
                this.cacheWindowSize = size;
                const sizeVp: window.SizeInVP = {
                    width: this.context.px2vp(size.width),
                    height: this.context.px2vp(size.height),
                };
                cb(sizeVp);
            }
            this.windowSizeDensityCbVp = (info: observer.DensityInfo) => {
                if (this.cacheWindowSize) {
                    const sizeVp: window.SizeInVP = {
                        width: this.context.px2vp(this.cacheWindowSize!.width),
                        height: this.context.px2vp(this.cacheWindowSize!.height),
                    };
                    cb(sizeVp);
                }
            }
            try {
                win?.onWindowSizeChange(this.windowSizeCbVp!);
                this.context.getUIObserver().onDensityUpdate(this.windowSizeDensityCbVp!);
            } catch (error) {
                hilog.error(LAYOUT_DOMAIN, LAYOUT_TAG, `[env] WindowSizeVpEnv register failed, ${error.message}`);
            }
        }
    }

    unregisterWindowSizeVpEnv(cb: Callback<window.SizeInVP>): void {
        try {
            if (this.windowSizeCbVp) {
                this.window?.offWindowSizeChange(this.windowSizeCbVp!);
                this.windowSizeCbVp = undefined;
            }
            if (this.windowSizeDensityCbVp) {
                this.context.getUIObserver().offDensityUpdate(this.windowSizeDensityCbVp!);
                this.windowSizeDensityCbVp = undefined;
            }
        } catch (error) {
            hilog.error(LAYOUT_DOMAIN, LAYOUT_TAG, `[env] WindowSizeVpEnv unregister failed, ${error.message}`);
        }
    }

    registerWindowSizePxEnv(cb: Callback<window.Size>): void {
        const win: window.Window | uiExtension.WindowProxy | undefined = this.window ?? this.windowProxy;
        try {
            win?.onWindowSizeChange(cb);
        } catch (error) {
            hilog.error(LAYOUT_DOMAIN, LAYOUT_TAG, `[env] WindowSizePxEnv register failed, ${error.message}`);
        }
    }

    unregisterWindowSizePxEnv(cb: Callback<window.Size>): void {
        const win: window.Window | uiExtension.WindowProxy | undefined = this.window ?? this.windowProxy;
        try {
            win?.offWindowSizeChange(cb);
        } catch (error) {
            hilog.error(LAYOUT_DOMAIN, LAYOUT_TAG, `[env] WindowSizePxEnv unregister failed, ${error.message}`);
        }
    }
}