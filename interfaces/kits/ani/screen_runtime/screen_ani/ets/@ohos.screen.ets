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

import { AsyncCallback, Callback } from '@ohos.base';
import { BusinessError } from '@ohos.base';
import image from '@ohos.multimedia.image';

export default namespace screen {

loadLibraryWithPermissionCheck("screenani_kit.z", "@ohos.screen");

const SCREEN_ARRAY_LENGTH = 15;
const SCREEN_MODE_INFO = 15;

export function on<T>(eventType: string, callback: Callback<T>): void{
    syncOn(eventType, callback, screenMgrRef);
}

export function onConnect(callback: Callback<long>): void{
    syncOn('connect', callback, screenMgrRef);
}

export function onDisconnect(callback: Callback<long>): void{
    syncOn('disconnect', callback, screenMgrRef);
}

export function onChange(callback: Callback<long>): void{
    syncOn('change', callback, screenMgrRef);
}

native function syncOn(eventType: string, callback: object, nativeObj: long): void;

export function off<T>(eventType: string, callback?: Callback<T>): void{
    syncOff(eventType, screenMgrRef, callback);
}

export function offConnect(callback?: Callback<long>): void{
    syncOff('connect', screenMgrRef, callback);
}

export function offDisconnect(callback?: Callback<long>): void{
    syncOff('disconnect', screenMgrRef, callback);
}

export function offChange(callback?: Callback<long>): void{
    syncOff('change', screenMgrRef, callback);
}

native function syncOff(eventType: string, nativeObj: long, callback?: object): void;

export function makeMirror(mainScreen: long, mirrorScreen: Array<long>, callback: AsyncCallback<long>): void {
    taskpool.execute((): long => {
        let res: long = makeMirrorInternal(mainScreen, mirrorScreen);
        return res;
    }).then((ret: Any) => {
        callback(null, ret as long);
    }).catch((err: Any) => {
        callback(err as BusinessError, -1);
    });
}

export function makeMirror(mainScreen: long, mirrorScreen: Array<long>): Promise<long> {
    return new Promise<long>((resolve: (value: long) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute((): long => {
            let res: long = makeMirrorInternal(mainScreen, mirrorScreen);
            return res;
        }).then((ret: Any) => {
            resolve(ret as long);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    })
}

export native function makeMirrorInternal(mainScreen: long, mirrorScreen: Array<long>): long;

export function getAllScreens(callback: AsyncCallback<Array<Screen>>): void {
    taskpool.execute((): Array<Screen> => {
        let screens: Array<Screen> = new Array<Screen>(SCREEN_ARRAY_LENGTH);
        for (let i = 0; i < 10; i++) {
            screens[i] = new ScreenImpl();
        }
        getAllScreensInternal(screens);
        minusScreens(screens);
        return screens;
    }).then((ret: Any) => {
        callback(null, ret as Array<Screen>);
    }).catch((err: Any) => {
        callback(err as BusinessError, new Array<Screen>());
    });
}

export function getAllScreens(): Promise<Array<Screen>> {
    return new Promise<Array<Screen>>((resolve: (value: Array<Screen>) => void,
        reject: (error: BusinessError) => void ) => {
            taskpool.execute((): Array<Screen> => {
                let screens: Array<Screen> = new Array<Screen>(SCREEN_ARRAY_LENGTH);
                for (let i = 0; i < 10; i++) {
                    screens[i] = new ScreenImpl();
                }
                getAllScreensInternal(screens);
                minusScreens(screens);
                return screens;
            }).then((ret: Any) => {
                resolve(ret as Array<Screen>);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        }
    );
}

function minusScreens(screens: Array<Screen>) {
    let i = 0;
    for (; i < screens.length; i++) {
        let currentScreen: ScreenImpl = screens[i] as ScreenImpl;
        if (currentScreen.id < 0) {
            screens.splice(i);
            break;
        }
        minusScreenModeInfo(currentScreen.supportedModeInfo);
    }
}

function minusScreenModeInfo(screenModeInfo: Array<ScreenModeInfo>) {
    let i = 0;
    for (; i < screenModeInfo.length; i++) {
        let currentScreenMode: ScreenModeInfoImpl = screenModeInfo[i] as ScreenModeInfoImpl;
        if (currentScreenMode.width <= 0) {
            break;
        }
    }
    screenModeInfo.splice(i);
}

export function createVirtualScreen(options: VirtualScreenOption, callback: AsyncCallback<Screen>): void {
    taskpool.execute((): Screen => {
        let screen = new ScreenImpl();
        createVirtualScreenInternal(options, screen);
        return screen;
    }).then((ret: Any) => {
        callback(null, ret as Screen);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function createVirtualScreen(options: VirtualScreenOption): Promise<Screen> {
    return new Promise<Screen>((resolve: (value: Screen) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): Screen => {
            let screen = new ScreenImpl();
            createVirtualScreenInternal(options, screen);
            return screen;
        }).then((ret: Any) => {
            resolve(ret as Screen);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function setVirtualScreenSurface(screenId: long, surfaceId: string, callback: AsyncCallback<void>): void {
    taskpool.execute((): void => {
        setVirtualScreenSurfaceInternal(screenId, surfaceId);
    }).then(() => {
        callback(null, undefined);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function setVirtualScreenSurface(screenId: long, surfaceId: string): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): void => {
            setVirtualScreenSurfaceInternal(screenId, surfaceId);
        }).then(() => {
            resolve(undefined);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function destroyVirtualScreen(screenId: long, callback: AsyncCallback<void>): void {
    taskpool.execute((): void => {
        destroyVirtualScreenInternal(screenId);
    }).then(() => {
        callback(null, undefined);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function destroyVirtualScreen(screenId: long): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): void => {
            destroyVirtualScreenInternal(screenId);
        }).then(() => {
            resolve(undefined);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function isScreenRotationLocked(callback: AsyncCallback<boolean>): void {
    taskpool.execute((): boolean => {
        return isScreenRotationLockedInternal();
    }).then((ret: Any) => {
        callback(null, ret as boolean);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function isScreenRotationLocked(): Promise<boolean> {
    return new Promise<boolean>((resolve: (value: boolean) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): boolean => {
            return isScreenRotationLockedInternal();
        }).then((ret: Any) => {
            resolve(ret as boolean);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function setScreenRotationLocked(isLocked: boolean, callback: AsyncCallback<void>): void {
    taskpool.execute((): void => {
        setScreenRotationLockedInternal(isLocked);
    }).then(() => {
        callback(null, undefined);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function setScreenRotationLocked(isLocked: boolean): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): void => {
            setScreenRotationLockedInternal(isLocked);
        }).then(() => {
            resolve(undefined);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function setMultiScreenRelativePosition(mainScreenOptions: MultiScreenPositionOptions,
    secondaryScreenOptions: MultiScreenPositionOptions): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): void => {
            setMultiScreenRelativePositionInternal(mainScreenOptions, secondaryScreenOptions);
        }).then(() => {
            resolve(undefined);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function setMultiScreenMode(primaryScreenId: long, secondaryScreenId: long,
    secondaryScreenMode: MultiScreenMode): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): void => {
            setMultiScreenModeInternal(primaryScreenId, secondaryScreenId, secondaryScreenMode);
        }).then(() => {
            resolve(undefined);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function setScreenPrivacyMaskImage(screenId: long, image?: image.PixelMap): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): void => {
            setScreenPrivacyMaskImageInternal(screenId, image);
        }).then(() => {
            resolve(undefined);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function makeUnique(uniqueScreen: Array<long>): Promise<Array<long>> {
    return new Promise<Array<long>>((resolve: (value: Array<long>) => void,
        reject: (error: BusinessError) => void ) => {
        taskpool.execute((): Array<long> => {
            return makeUniqueInternal(uniqueScreen);
        }).then((ret: Any) => {
            resolve(ret as Array<long>);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function makeMirrorWithRegion(mainScreen: long, mirrorScreen: Array<long>, mainScreenRegion: Rect):
    Promise<long> {
    return new Promise<long>((resolve: (value: long) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): long => {
            return makeMirrorWithRegionInternal(mainScreen, mirrorScreen, mainScreenRegion);
        }).then((ret: Any) => {
            resolve(ret as long);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    })
}

export function stopMirror(mirrorScreen: Array<long>, callback: AsyncCallback<void>): void {
    taskpool.execute((): void => {
        stopMirrorInternal(mirrorScreen);
    }).then(() => {
        callback(null, undefined);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function stopMirror(mirrorScreen: Array<long>): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): void => {
            stopMirrorInternal(mirrorScreen);
        }).then(() => {
            resolve(undefined);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function makeExpand(options: Array<ExpandOption>, callback: AsyncCallback<long>): void {
    taskpool.execute((): long => {
        return makeExpandInternal(options);
    }).then((ret: Any) => {
        callback(null, ret as long);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function makeExpand(options: Array<ExpandOption>): Promise<long> {
    return new Promise<long>((resolve: (value: long) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): long => {
            return makeExpandInternal(options);
        }).then((ret: Any) => {
            resolve(ret as long);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function stopExpand(expandScreen: Array<long>, callback: AsyncCallback<void>): void {
    taskpool.execute((): void => {
        stopExpandInternal(expandScreen);
    }).then(() => {
        callback(null, undefined);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}

export function stopExpand(expandScreen: Array<long>): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void) => {
        taskpool.execute((): void => {
            stopExpandInternal(expandScreen);
        }).then(() => {
            resolve(undefined);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export native function getAllScreensInternal(screens: Array<Screen>): void;
export native function createVirtualScreenInternal(options: VirtualScreenOption, screen: ScreenImpl): void;
export native function setVirtualScreenSurfaceInternal(screenId: long, surfaceId: string): void;
export native function destroyVirtualScreenInternal(screenId: long): void;
export native function isScreenRotationLockedInternal(): boolean;
export native function setScreenRotationLockedInternal(isLocked: boolean): void;
export native function setMultiScreenRelativePositionInternal(mainScreenOptions: MultiScreenPositionOptions,
    secondaryScreenOptions: MultiScreenPositionOptions): void;
export native function setMultiScreenModeInternal(primaryScreenId: long, secondaryScreenId: long,
    secondaryScreenMode: MultiScreenMode): void;
export native function setScreenPrivacyMaskImageInternal(screenId: long, image?: image.PixelMap): void;
export native function makeUniqueInternal(uniqueScreen: Array<long>): Array<long>;
export native function makeMirrorWithRegionInternal(mainScreen: long, mirrorScreen: Array<long>,
    mainScreenRegion: Rect): long;
export native function stopMirrorInternal(mirrorScreen: Array<long>): void;
export native function makeExpandInternal(options: Array<ExpandOption>): long;
export native function stopExpandInternal(options: Array<long>): void;

export interface Screen {

    readonly id: long;

    readonly parent: long;

    readonly supportedModeInfo: Array<ScreenModeInfo>;

    readonly activeModeIndex: long;

    readonly orientation: Orientation;

    readonly sourceMode: ScreenSourceMode;

    setDensityDpi(densityDpi: double, callback: AsyncCallback<void>): void;

    setDensityDpi(densityDpi: double): Promise<void>;

    setScreenActiveMode(modeIndex: long, callback: AsyncCallback<void>): void;

    setScreenActiveMode(modeIndex: long): Promise<void>;

    setOrientation(orientation: Orientation, callback: AsyncCallback<void>): void;
    
    setOrientation(orientation: Orientation): Promise<void>;
}

class ScreenImpl implements Screen {

    static { loadLibraryWithPermissionCheck("screenani_kit.z", "@ohos.screen"); }

    constructor() {
        this.id = -1;
        this.supportedModeInfo = new Array<ScreenModeInfo>(SCREEN_MODE_INFO);
        for (let i = 0; i < this.supportedModeInfo.length;i++) {
            this.supportedModeInfo[i] = new ScreenModeInfoImpl();
        }
    }

    readonly id: long;

    readonly parent: long;

    readonly supportedModeInfo: Array<ScreenModeInfo>;

    readonly activeModeIndex: long;

    readonly orientation: Orientation;

    readonly sourceMode: ScreenSourceMode;

    screenNativeObj: long;

    setDensityDpi(densityDpi: double, callback: AsyncCallback<void>): void {
        taskpool.execute((): void => {
            this.setDensityDpiInternal(densityDpi);
        }).then(() => {
            callback(null, undefined);
        }).catch((err: Any) => {
            callback(err as BusinessError, undefined);
        });
    }

    setDensityDpi(densityDpi: double): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
            taskpool.execute((): void => {
                this.setDensityDpiInternal(densityDpi);
            }).then(() => {
                resolve(undefined);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    static transferStatic(input: Any): Object {
        let screen = new ScreenImpl();
        let screen_input: ESValue = ESValue.wrap(input);
        let ret = false;
        if (screen_input !== ESValue.Undefined) {
            ret = ScreenImpl.nativeTransferStatic(screen_input, screen as Object);
            minusScreenModeInfo(screen.supportedModeInfo);
        }
        if (!ret) {
            return {};
        }
        return screen as Object;
    }

    static transferDynamic(input: Object): Any {
        return ScreenImpl.nativeTransferDynamic((input as ScreenImpl).screenNativeObj).unwrap();
    }

    setScreenActiveMode(modeIndex: long, callback: AsyncCallback<void>): void {
        taskpool.execute((): void => {
            this.setScreenActiveModeInternal(modeIndex);
        }).then(() => {
            callback(null, undefined);
        }).catch((err: Any) => {
            callback(err as BusinessError, undefined);
        });
    }

    setScreenActiveMode(modeIndex: long): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void):
            void => {
            taskpool.execute((): void => {
                this.setScreenActiveModeInternal(modeIndex);
            }).then(() => {
                resolve(undefined);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    setOrientation(orientation: Orientation, callback: AsyncCallback<void>): void {
        taskpool.execute((): void => {
            this.setOrientationInternal(orientation);
        }).then(() => {
            callback(null, undefined);
        }).catch((err: Any) => {
            callback(err as BusinessError, undefined);
        });
    }

    setOrientation(orientation: Orientation): Promise<void> {
        return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void):
            void => {
            taskpool.execute((): void => {
                this.setOrientationInternal(orientation);
            }).then(() => {
                resolve(undefined);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    native setDensityDpiInternal(densityDpi: double): void;
    native static nativeTransferStatic(input: ESValue, screen: Object): boolean;
    native static nativeTransferDynamic(nativeObj: long): ESValue;
    native setScreenActiveModeInternal(modeIndex: long): void;
    native setOrientationInternal(orientation: Orientation): void;
}

export interface ScreenModeInfo {
    id: long;

    width: long;

    height: long;

    refreshRate: int;
}

class ScreenModeInfoImpl implements ScreenModeInfo{
    id: long;

    width: long;

    height: long;

    refreshRate: int;
}

export interface MultiScreenPositionOptions {
    id: long;

    startX: long;

    startY: long;
}

class MultiScreenPositionOptionsImpl implements MultiScreenPositionOptions{
    id: long;

    startX: long;

    startY: long;
}

export enum MultiScreenMode {
    SCREEN_MIRROR = 0,

    SCREEN_EXTEND = 1
}

export enum ScreenSourceMode {
    SCREEN_MAIN = 0,

    SCREEN_MIRROR = 1,

    SCREEN_EXTEND = 2,

    SCREEN_ALONE = 3
}

export enum Orientation {
    UNSPECIFIED = 0,

    VERTICAL = 1,

    HORIZONTAL = 2,

    REVERSE_VERTICAL = 3,

    REVERSE_HORIZONTAL = 4
}

export interface VirtualScreenOption {
    name: string;

    width: long;

    height: long;

    density: double;

    surfaceId: string;

    supportsFocus?: boolean;
}

class VirtualScreenOptionImpl implements VirtualScreenOption {
    name: string;

    width: long;

    height: long;

    density: double;

    surfaceId: string;

    supportsFocus?: boolean;
}

export interface Rect {
    left: long;

    top: long;

    width: long;

    height: long;
}

class RectImpl implements Rect {
    left: long;

    top: long;

    width: long;

    height: long;
}

export interface ExpandOption {
    screenId: long;

    startX: long;

    startY: long;
}

class ExpandOptionImpl implements ExpandOption {
    screenId: long;

    startX: long;

    startY: long;
}

export let screenMgrRef: long;
export function setScreenMgrRef(nativeObj: long): void{
    screenMgrRef = nativeObj;
}

export function screenEventCallBack(cb: object, cbArg: long): void {
    const func = cb as (cbArg: long) => void;
    func(cbArg as long);
}
}
