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
import type { AsyncCallback, Callback} from '@ohos.base';
import { BusinessError } from '@ohos.base';
import colorSpaceManager from '@ohos.graphics.colorSpaceManager';
import hdrCapability from '@ohos.graphics.hdrCapability';

export default namespace display {

loadLibraryWithPermissionCheck("displayani_kit.z", "@ohos.display");

const DISPLAY_ARRAY_LENGTH = 15;
const CUTO_ARRAY_LENGTH = 5;
const CREASE_RECTS_LENGTH = 10;
const DISPLAY_PHYSICAL_RESOLUTION_LENGTH = 15;
const ERROR_INVALID_PARAM = 401;
const ROUNDED_CORNER_LENGTH = 4;

export interface Rect {
    left: long;

    top: long;

    width: long;

    height: long;
}

export class RectImpl implements Rect {
    left: long;

    top: long;

    width: long;

    height: long;
}

interface WaterfallDisplayAreaRects {
    readonly left: Rect;

    readonly right: Rect;

    readonly top: Rect;

    readonly bottom: Rect;
}

export class WaterfallDisplayAreaRectsImpl implements WaterfallDisplayAreaRects {
    constructor() {
        this.left = new RectImpl();
        this.right = new RectImpl();
        this.top = new RectImpl();
        this.bottom = new RectImpl();
    }
    readonly left: Rect;

    readonly right: Rect;

    readonly top: Rect;

    readonly bottom: Rect;
}

export interface CutoutInfo {
    readonly boundingRects: Array<Rect>;

    readonly waterfallDisplayAreaRects: WaterfallDisplayAreaRects;
}

export class CutoutInfoImpl implements CutoutInfo {
    constructor() {
        this.boundingRects = new Array<Rect>(CUTO_ARRAY_LENGTH);
        for (let i = 0; i < this.boundingRects.length; i++) {
            this.boundingRects[i] = new RectImpl();
        }
        this.waterfallDisplayAreaRects = new WaterfallDisplayAreaRectsImpl();
    }

    readonly boundingRects: Array<Rect>;

    readonly waterfallDisplayAreaRects: WaterfallDisplayAreaRects;
}

export interface RoundedCorner {
    readonly type: CornerType;

    readonly position: Position;

    readonly radius: int;
}

export class RoundedCornerImpl implements RoundedCorner {
    constructor() {
        this.position = new PositionImpl();
    }

    readonly type: CornerType;

    readonly position: Position;

    readonly radius: int;
}

export interface FoldCreaseRegion {
    readonly displayId: long;

    readonly creaseRects: Array<Rect>;
}

export class FoldCreaseRegionImpl implements FoldCreaseRegion {
    constructor() {
        this.creaseRects = new Array<Rect>(CREASE_RECTS_LENGTH);
        for (let i = 0; i < this.creaseRects.length; i++) {
            this.creaseRects[i] = new RectImpl();
        }
    }

    readonly displayId: long;

    readonly creaseRects: Array<Rect>;
}

export interface Position {
    x: long;

    y: long;
}

export class PositionImpl implements Position {
    x: long;

    y: long;
}

export interface RelativePosition {
    displayId: long;

    position: Position;
}

export class RelativePositionImpl implements RelativePosition {
    constructor() {
        this.position = new PositionImpl();
    }

    displayId: long;

    position: Position;
}

export interface VirtualScreenConfig {
    name: string;
 
    width: long;
 
    height: long;
 
    density: double;
 
    surfaceId: string;

    supportsFocus?: boolean;
}
 
export class VirtualScreenConfigImpl implements VirtualScreenConfig {
    name: string;
 
    width: long;
 
    height: long;
 
    density: double;
 
    surfaceId: string;

    supportsFocus?: boolean;
}

export enum FoldDisplayMode {
    FOLD_DISPLAY_MODE_UNKNOWN = 0,

    FOLD_DISPLAY_MODE_FULL,

    FOLD_DISPLAY_MODE_MAIN,

    FOLD_DISPLAY_MODE_SUB,

    FOLD_DISPLAY_MODE_COORDINATION
}

export enum Orientation {
    PORTRAIT = 0,

    LANDSCAPE = 1,

    PORTRAIT_INVERTED = 2,

    LANDSCAPE_INVERTED = 3
  }

export enum FoldStatus {
    FOLD_STATUS_UNKNOWN = 0,

    FOLD_STATUS_EXPANDED,

    FOLD_STATUS_FOLDED,

    FOLD_STATUS_HALF_FOLDED,

    FOLD_STATUS_EXPANDED_WITH_SECOND_EXPANDED = 11,

    FOLD_STATUS_EXPANDED_WITH_SECOND_HALF_FOLDED = 21,

    FOLD_STATUS_FOLDED_WITH_SECOND_EXPANDED = 12,

    FOLD_STATUS_FOLDED_WITH_SECOND_HALF_FOLDED = 22,

    FOLD_STATUS_HALF_FOLDED_WITH_SECOND_EXPANDED = 13,

    FOLD_STATUS_HALF_FOLDED_WITH_SECOND_HALF_FOLDED = 23
  }

export enum DisplayState {
    STATE_UNKNOWN = 0,

    STATE_OFF,

    STATE_ON,

    STATE_DOZE,

    STATE_DOZE_SUSPEND,

    STATE_VR,

    STATE_ON_SUSPEND
}

export enum ScreenShape {
    
    RECTANGLE = 0,

    ROUND = 1
}

export enum DisplaySourceMode {
    
    NONE = 0,

    MAIN = 1,

    MIRROR = 2,

    EXTEND = 3,

    ALONE = 4
}

export enum CornerType {

    TOP_LEFT = 0,

    TOP_RIGHT = 1,

    BOTTOM_RIGHT = 2,

    BOTTOM_LEFT = 3
}

export interface Display {
    id: long;

    name: string;

    alive: boolean;

    state: DisplayState;

    refreshRate: int;

    rotation: int;

    width: long;
 
    height: long;

    availableHeight: long;

    availableWidth: long;

    densityDPI: double;

    orientation: Orientation;

    densityPixels: double;

    scaledDensity: double;

    xDPI: double;

    yDPI: double;

    screenShape?: ScreenShape;

    colorSpaces: Array<colorSpaceManager.ColorSpace>;

    hdrFormats: Array<hdrCapability.HDRFormat>;

    sourceMode?:DisplaySourceMode;

    x?: long;

    y?: long;

    supportedRefreshRates?: Array<int>;

    getCutoutInfo(callback: AsyncCallback<CutoutInfo>): void;

    getCutoutInfo(): Promise<CutoutInfo>;

    getRoundedCorner(): Array<RoundedCorner>;

    getAvailableArea(): Promise<Rect>;

    getLiveCreaseRegion(): FoldCreaseRegion;

    hasImmersiveWindow(callback: AsyncCallback<boolean>): void;

    hasImmersiveWindow(): Promise<boolean>;

    onAvailableAreaChange(callback: Callback<Rect>): void;

    offAvailableAreaChange(callback?: Callback<Rect>): void;

    on(type: string, callback: Callback<Rect>): void;

    off(type: string, callback?: Callback<Rect>): void;

    getDisplayCapability(): string;
}

export class DisplayImpl implements Display {
    static { loadLibraryWithPermissionCheck("displayani_kit.z", "@ohos.display"); }

    constructor() {
        this.colorSpaces = new Array<colorSpaceManager.ColorSpace>();
        this.hdrFormats = new Array<hdrCapability.HDRFormat>();
        this.supportedRefreshRates = new Array<int>();
    }

    id: long;

    name: string;

    alive: boolean;

    state: DisplayState;

    refreshRate: int;

    rotation: int;

    width: long;
 
    height: long;

    availableHeight: long;

    availableWidth: long;

    densityDPI: double;

    orientation: Orientation;

    densityPixels: double;

    scaledDensity: double;

    xDPI: double;

    yDPI: double;

    screenShape?: ScreenShape;

    colorSpaces: Array<colorSpaceManager.ColorSpace>;

    hdrFormats: Array<hdrCapability.HDRFormat>;

    sourceMode?: DisplaySourceMode;

    x?: long;

    y?: long;

    supportedRefreshRates?: Array<int>;

    displayRef: long;

    setDisplayRef(nativeObj : long) : void{
        this.displayRef = nativeObj;
    }

    getCutoutInfo(callback: AsyncCallback<CutoutInfo>): void {
        taskpool.execute((): CutoutInfo => {
            let res: CutoutInfo = new CutoutInfoImpl();
            this.getCutoutInfoInternal(res);
            minusRectArray(res.boundingRects);
            return res;
        }).then((ret: Any) => {
            callback(null, ret as CutoutInfo);
        }).catch((err: Any) => {
            callback(err as BusinessError, new CutoutInfoImpl());
        });
    }

    getCutoutInfo(): Promise<CutoutInfo> {
        return new Promise<CutoutInfo>((resolve: (value: CutoutInfo) => void, reject: (error: BusinessError) => void ) => {
            taskpool.execute((): CutoutInfo => {
                let res: CutoutInfo = new CutoutInfoImpl()
                this.getCutoutInfoInternal(res);
                minusRectArray(res.boundingRects);
                return res;
            }).then((ret: Any) => {
                resolve(ret as CutoutInfo);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    getRoundedCorner(): Array<RoundedCorner> {
        let res: Array<RoundedCorner> = new Array<RoundedCorner>(ROUNDED_CORNER_LENGTH);
        for (let i = 0; i < res.length; i++) {
            res[i] = new RoundedCornerImpl();
        }
        this.getRoundedCornerInternal(res);
        minusRoundedCornerArray(res);
        return res;
    }

    hasImmersiveWindow(callback: AsyncCallback<boolean>): void {
        taskpool.execute((): boolean => {
            return this.hasImmersiveWindowInternal();
        }).then((ret: Any) => {
            callback(null, ret as boolean);
        }).catch((err: Any) => {
            callback(err as BusinessError, false);
        });
    }

    hasImmersiveWindow(): Promise<boolean> {
        return new Promise<boolean>((resolve: (value: boolean) => void, reject: (error: BusinessError) => void ) => {
            taskpool.execute((): boolean => {
                return this.hasImmersiveWindowInternal();
            }).then((ret: Any) => {
                resolve(ret as boolean);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
    }

    getAvailableArea(): Promise<Rect> {
        return new Promise<Rect>((resolve: (value: Rect) => void, reject: (error: BusinessError) => void ) => {
            taskpool.execute((): Rect => {
                let res: Rect = new RectImpl();
                this.getAvailableAreaInternal(res);
                return res;
            }).then((ret: Any) => {
                resolve(ret as Rect);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        })
    }

    getLiveCreaseRegion(): FoldCreaseRegion {
        let foldCreaseRegion: FoldCreaseRegion = new FoldCreaseRegionImpl();
        this.getLiveCreaseRegionInternal(foldCreaseRegion as object);
        minusCreaseRects(foldCreaseRegion);
        return foldCreaseRegion;
    }

    on(type: string, callback: Callback<Rect>): void {
        this.syncOn(type, callback as object);
    }

    off(type: string, callback?: Callback<Rect>): void {
        this.syncOff(type, callback);
    }

    onAvailableAreaChange(callback: Callback<Rect>): void {
        this.syncOn('availableAreaChange', callback as object);
    }

    offAvailableAreaChange(callback?: Callback<Rect>): void {
        this.syncOff('availableAreaChange', callback);
    }

    getDisplayCapability(): string {
        let displayCapability: string = this.getDisplayCapabilityInternal();
        return displayCapability;
    }

    static transferStatic(input: Any): Object {
        let display = new DisplayImpl();
        let display_input: ESValue = ESValue.wrap(input);
        let ret = false;
        if (display_input !== ESValue.Undefined) {
            ret = DisplayImpl.nativeTransferStatic(display_input, display as Object);
        }
        if (!ret) {
            return {};
        }
        return display as Object;
    }

    static transferDynamic(input: Object): Any {
        return DisplayImpl.nativeTransferDynamic((input as DisplayImpl).displayRef).unwrap();
    }

    native syncOn(type: string, callback: object): void;
    native syncOff(type: string, callback?: object): void;
    native getAvailableAreaInternal(rect: Rect): void;
    native getLiveCreaseRegionInternal(foldCreaseRegion: object):FoldCreaseRegion;
    native getCutoutInfoInternal(cutoutInfo: CutoutInfo): void;
    native getRoundedCornerInternal(roundedCorner: Array<RoundedCorner>): void;
    native getDisplayCapabilityInternal(): string;
    native hasImmersiveWindowInternal(): boolean;

    native static nativeTransferStatic(input: ESValue, display: Object): boolean;
    native static nativeTransferDynamic(nativeObj: long): ESValue;
}

function minusRectArray(rectArr: Array<Rect>): void {
    let i = 0;
    for (; i < rectArr.length; i++) {
        if (rectArr[i].width == 0) {
            rectArr.splice(i);
            break;
        }
    }
}

function minusRoundedCornerArray(cornerArr: Array<RoundedCorner>): void {
    if (cornerArr[0].radius == 0) {
        cornerArr.splice(0);
    }
}

export native function isFoldable(): boolean;

export native function isCaptured(): boolean;

export function getFoldDisplayMode(): FoldDisplayMode {
    let res = getFoldDisplayModeNative();
    return res as FoldDisplayMode;
}

export native function getFoldDisplayModeNative(): int;

export function setFoldDisplayMode(mode: FoldDisplayMode, reason: string): void {
    setFoldDisplayModeReasonNative(mode as int, reason, displayMgrRef);
}

export function setFoldDisplayMode(mode: FoldDisplayMode): void {
    setFoldDisplayModeNative(mode as int, displayMgrRef);
}

export native function setFoldDisplayModeReasonNative(mode: int, reason: string, nativeObj: long): void;
export native function setFoldDisplayModeNative(mode: int, nativeObj: long): void;

export function getFoldStatus(): FoldStatus{
    let res = getFoldStatusNative();
    return res as FoldStatus;
}

export native function getFoldStatusNative(): int;

export function getCurrentFoldCreaseRegion(): FoldCreaseRegion {
    let foldCreaseRegion: FoldCreaseRegion = new FoldCreaseRegionImpl();
    getCurrentFoldCreaseRegionNative(foldCreaseRegion as object, displayMgrRef);
    minusCreaseRects(foldCreaseRegion);
    return foldCreaseRegion;
}

function minusCreaseRects(foldCreaseRegionObj: FoldCreaseRegion): void {
    let foldCreaseRegion: FoldCreaseRegion = foldCreaseRegionObj as FoldCreaseRegion;
    let creaseRects: Array<Rect> = foldCreaseRegion.creaseRects;
    let i = 0;
    for (; i < creaseRects.length; i++) {
        if (creaseRects[i].left == 0 && creaseRects[i].top == 0 && 
            creaseRects[i].width == 0 && creaseRects[i].height == 0) {
                creaseRects.splice(i);
                break;
        }
    }
}

export native function getCurrentFoldCreaseRegionNative(foldCreaseRegion: object, nativeObj: long): void;

export function getPrimaryDisplaySync(): Display {
    let display = new DisplayImpl();
    getPrimaryDisplaySyncNative(display as object);
    return display;
}

export native function getPrimaryDisplaySyncNative(obj: object): void;

// display sync interfaces
export function getDisplayByIdSync(displayId: long): Display {
    let display = new DisplayImpl();
    getDisplayByIdSyncNative(display as object, displayId);
    return display;
}

export native function getDisplayByIdSyncNative(obj: object, displayId: long): void;

export function getDefaultDisplaySync(): Display {
    let display = new DisplayImpl();
    getDefaultDisplaySyncNative(display as object);
    return display;
}

export native function getDefaultDisplaySyncNative(obj: object): void;

export native function getAllDisplaysSyncNative(displayArray: Array<Display>): void;

export function getAllDisplays(callback: AsyncCallback<Array<Display>>): void {
    let res: Array<Display> = new Array<Display>(DISPLAY_ARRAY_LENGTH);
    for (let i = 0; i < res.length; i++) {
        res[i] = new DisplayImpl();
    }
    try {
        getAllDisplaysSyncNative(res);
        minusDisplayArray(res);
        callback(null, res);
    } catch (err) {
        err = err as BusinessError;
        callback(err, res);
    }
    return;
}
export function getAllDisplays(): Promise<Array<Display>> {
    return new Promise<Array<Display>>((resolve: (value: Array<Display>) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute((): Array<Display> => {
            let res: Array<Display> = new Array<Display>(DISPLAY_ARRAY_LENGTH);
            for (let i = 0; i < res.length; i++) {
                res[i] = new DisplayImpl();
            }
            getAllDisplaysSyncNative(res);
            minusDisplayArray(res)
            return res;
        }).then((ret: Any) => {
            resolve(ret as Array<Display>);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

function minusDisplayArray(displayArr: Array<Display>): void {
    let i = 0;
    for (; i < displayArr.length; i++) {
        if (displayArr[i].width == 0) {
            displayArr.splice(i);
            break;
        }
        let dp = displayArr[i] as DisplayImpl;
    }
}

export function getNewDisplay(): DisplayImpl {
    return new DisplayImpl();
}

export type BrightnessCallback<T1, T2> = (id:T1, data:T2) => void;

export function on<T>(type: string, callback: Callback<T>): void {
    syncOn(type, callback as object, displayMgrRef);
}

export function onAdd(callback: Callback<long>): void {
    syncOn('add', callback as object, displayMgrRef);
}

export function onRemove(callback: Callback<long>): void {
    syncOn('remove', callback as object, displayMgrRef);
}

export function onChange(callback: Callback<long>): void {
    syncOn('change', callback as object, displayMgrRef);
}

export function onPrivateModeChange(callback: Callback<boolean>): void {
    syncOn('privateModeChange', callback as object, displayMgrRef);
}

export function onFoldStatusChange(callback: Callback<FoldStatus>): void {
    syncOn('foldStatusChange', callback as object, displayMgrRef);
}

export function onFoldAngleChange(callback: Callback<Array<double>>): void {
    syncOn('foldAngleChange', callback as object, displayMgrRef);
}

export function onCaptureStatusChange(callback: Callback<boolean>): void {
    syncOn('captureStatusChange', callback as object, displayMgrRef);
}

export function onFoldDisplayModeChange(callback: Callback<FoldDisplayMode>): void {
    syncOn('foldDisplayModeChange', callback as object, displayMgrRef);
}

export function onBrightnessInfoChange<T1, T2>(callback: BrightnessCallback<T1, T2>): void {
    syncOn('brightnessInfoChange', callback as object, displayMgrRef);
}

export function off<T>(type: string, callback?: Callback<T>): void {
    syncOff(type, displayMgrRef, callback);
}

export function offAdd(callback?: Callback<long>): void {
    syncOff('add', displayMgrRef, callback);
}

export function offRemove(callback?: Callback<long>): void {
    syncOff('remove', displayMgrRef, callback);
}

export function offChange(callback?: Callback<long>): void {
    syncOff('change', displayMgrRef, callback);
}

export function offPrivateModeChange(callback?: Callback<boolean>): void {
    syncOff('privateModeChange', displayMgrRef, callback);
}

export function offFoldStatusChange(callback?: Callback<FoldStatus>): void {
    syncOff('foldStatusChange', displayMgrRef, callback);
}

export function offFoldAngleChange(callback?: Callback<Array<double>>): void {
    syncOff('foldAngleChange', displayMgrRef, callback);
}

export function offCaptureStatusChange(callback?: Callback<boolean>): void {
    syncOff('captureStatusChange', displayMgrRef, callback);
}

export function offFoldDisplayModeChange(callback?: Callback<FoldDisplayMode>): void {
    syncOff('foldDisplayModeChange', displayMgrRef, callback);
}

export function offBrightnessInfoChange<T1, T2>(callback?: BrightnessCallback<T1, T2>): void {	
    syncOff('brightnessInfoChange', displayMgrRef, callback);
}

export native function syncOn(type: string, callback: object, nativeObj: long): void;
export native function syncOff(type: string, nativeObj: long, callback?: object) : void;

export function displayEventCallBack(cb: object, cbArg: long): void {
    const func = cb as (cbArg: long) => void;
    func(cbArg as long);
}

export function foldStatusCallback(cb: object, cbArg: int): void {
    const func = cb as (cbArg: FoldStatus) => void;
    func(cbArg as FoldStatus);
}

export function foldDisplayModeCallback(cb: object, cbArg: int): void {
    const func = cb as (cbArg: FoldDisplayMode) => void;
    func(cbArg as FoldDisplayMode);
}

export function foldAngleChangeCallback(cb: object, cbArg: object): void {
    const func = cb as (cbArg: Array<double>) => void;
    func(cbArg as Array<double>);
}

export function captureStatusChangedCallback(cb: object, cbArg: boolean): void {
    const func = cb as (cbArg: boolean) => void;
    func(cbArg as boolean);
}

export function availableAreaChangedCallback(cb: object, cbArg: object): void {
    const func = cb as (cbArg: Rect) => void;
    func(cbArg as Rect);
}

export native function hasPrivateWindow(displayId: long): boolean;

export function getAllDisplayPhysicalResolution(): Promise<Array<DisplayPhysicalResolution>> {
    return new Promise<Array<DisplayPhysicalResolution>>((resolve: (value: Array<DisplayPhysicalResolution>) => void,
        reject: (error: BusinessError) => void ) => {
            taskpool.execute((): Array<DisplayPhysicalResolution> => {
                let res: Array<DisplayPhysicalResolution> =
                    new Array<DisplayPhysicalResolution>(DISPLAY_PHYSICAL_RESOLUTION_LENGTH);
                for (let i = 0; i < res.length; i++) {
                    res[i] = new DisplayPhysicalResolutionImpl();
                }
                getAllDisplayPhysicalResolutionNative(res, displayMgrRef);
                minusResolutionArray(res);
                return res;
            }).then((ret: Any) => {
                resolve(ret as Array<DisplayPhysicalResolution>);
            }).catch((err: Any) => {
                reject(err as BusinessError);
            });
        });
}

function minusResolutionArray(resolutions: Array<DisplayPhysicalResolution>): void {
    let index = 0;
    for (; index < resolutions.length; index++) {
        if (resolutions[index].physicalWidth == 0) {
            resolutions.splice(index);
            break;
        }
        let resolution = resolutions[index] as DisplayPhysicalResolutionImpl;
        resolution.foldDisplayMode = resolution.foldDisplayMode_ as FoldDisplayMode;
    }
}

export native function getAllDisplayPhysicalResolutionNative(PhysicalResolutions: Array<DisplayPhysicalResolution>, nativeObj: long): void;

export interface DisplayPhysicalResolution {
    foldDisplayMode: FoldDisplayMode;

    physicalWidth: long;

    physicalHeight: long;
}

export class DisplayPhysicalResolutionImpl implements DisplayPhysicalResolution{
    foldDisplayMode: FoldDisplayMode;
    foldDisplayMode_: int;

    physicalWidth: long;

    physicalHeight: long;
}

export function convertGlobalToRelativeCoordinate(position: Position, displayId?: long): RelativePosition {
    let relativePostion: RelativePosition = new RelativePositionImpl();
    convertGlobalToRelativeCoordinateNative(position, displayMgrRef, relativePostion, displayId);
    return relativePostion;
}

export function convertRelativeToGlobalCoordinate(relativePosition: RelativePosition): Position {
    let position: Position = new PositionImpl();
    convertRelativeToGlobalCoordinateNative(relativePosition, displayMgrRef, position);
    return position;
}

export native function convertGlobalToRelativeCoordinateNative(
    position: Position, nativeObj: long, relativePostion: RelativePosition, displayId?: long): void;

export native function convertRelativeToGlobalCoordinateNative(
    relativePosition: RelativePosition, nativeObj: long, position: Position): void;

export function makeUnique(screenId: long): Promise<void> {
    if (screenId < 0) {
        throw new BusinessError(ERROR_INVALID_PARAM, new Error(`[ANI] screenId is invalid: ${screenId}`));
    }
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ) :void => {
        taskpool.execute((): void => {
            makeUniqueNative(screenId, displayMgrRef);
        }).then((ret: Any) :void => {
            resolve(undefined);
        }).catch((err: Any) :void => {
            reject(err as BusinessError);
        });
    });
}
 
export native function makeUniqueNative(screenId: long, nativeObj: long): void;

export function addVirtualScreenBlocklist(windowIds: Array<int>): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
            addVirtualScreenBlocklistNative(windowIds, displayMgrRef);
        }).then((ret: Any): void => {
            resolve(undefined);
        }).catch((err: Any): void => {
            reject(err as BusinessError);
        });
    });
}
 
export function removeVirtualScreenBlocklist(windowIds: Array<int>): Promise<void> {
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void): void => {
        taskpool.execute((): void => {
            removeVirtualScreenBlocklistNative(windowIds, displayMgrRef);
        }).then((ret: Any): void => {
            resolve(undefined);
        }).catch((err: Any): void => {
            reject(err as BusinessError);
        });
    });
}
 
export native function addVirtualScreenBlocklistNative(windowIds: Array<int>, nativeObj: long): Promise<void>;
export native function removeVirtualScreenBlocklistNative(windowIds: Array<int>, nativeObj: long): Promise<void>;
 
export function setFoldStatusLocked(locked: boolean): void {
    setFoldStatusLockedNative(locked, displayMgrRef);
}
 
export native function setFoldStatusLockedNative(locked: boolean, nativeObj: long):void;
 
export function createVirtualScreen(config: VirtualScreenConfig): Promise<long> {
    return new Promise<long>((resolve: (value: long) => void, reject: (error: BusinessError) => void ) :void => {
        taskpool.execute((): long => {
            let screenId: long;
            screenId = createVirtualScreenNative(config as object, displayMgrRef);
            return screenId;
        }).then((ret: Any) :void => {
            resolve(ret as long);
        }).catch((err: Any) :void => {
            reject(err as BusinessError);
        });
    });  
}
 
export native function createVirtualScreenNative(config: object, nativeObj: long): long;
 
export function destroyVirtualScreen(screenId: long): Promise<void> {
    if (screenId < 0) {
        throw new BusinessError(ERROR_INVALID_PARAM, new Error(`[ANI] screenId is invalid: ${screenId}`));
    }
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ) :void => {
        taskpool.execute((): void => {
            destroyVirtualScreenNative(screenId, displayMgrRef);
        }).then((ret: Any) :void => {
            resolve(undefined);
        }).catch((err: Any) :void => {
            reject(err as BusinessError);
        });
    });
}
 
export native function destroyVirtualScreenNative(screenId: long, nativeObj: long): void;
 
export function setVirtualScreenSurface(screenId: long, surfaceId: string): Promise<void> {
    if (screenId < 0) {
        throw new BusinessError(ERROR_INVALID_PARAM, new Error(`[ANI] screenId is invalid: ${screenId}`));
    }
    return new Promise<void>((resolve: (value: undefined) => void, reject: (error: BusinessError) => void ) :void => {
        taskpool.execute((): void => {
            setVirtualScreenSurfaceNative(screenId, surfaceId, displayMgrRef);
        }).then((ret: Any) :void => {
            resolve(undefined);
        }).catch((err: Any) :void => {
            reject(err as BusinessError);
        });
    });
}
 
export native function setVirtualScreenSurfaceNative(screenId: long, surfaceId: string, nativeObj: long): void;

export function disposeDisplay(display: DisplayImpl): void {
    finalizerDisplayNative(display as object, displayMgrRef);
}

export native function finalizerDisplayNative(obj: object, nativeObj: long): void

export interface BrightnessInfo {
    readonly sdrNits: double;
    readonly currentHeadroom: double;
    readonly maxHeadroom: double;
}

export class ScreenBrightnessInfoImpl implements BrightnessInfo {
    readonly sdrNits: double;
    readonly currentHeadroom: double;
    readonly maxHeadroom: double;
}

export function getBrightnessInfo(displayId: long): BrightnessInfo {
    return getBrightnessInfoNative(displayId, displayMgrRef);
}

export function brightnessInfoChangeCallback(cb: object, cbArg: long, cbArgBrightness: BrightnessInfo): void {
    const func = cb as (cbArg: long, cbArgBrightness: BrightnessInfo) => void;
    func(cbArg, cbArgBrightness);
}

export native function getBrightnessInfoNative(display: long, displayMgrRef: long): BrightnessInfo;

export let displayMgrRef: long;

export function setDisplayMgrRef(nativeObj : long) : void{
    displayMgrRef = nativeObj;
}

export function onChangeWithAttribute(displayAttributeOption: Array<string>, callback: Callback<long>): void {
    onChangeWithAttributeNative(displayAttributeOption, callback, displayMgrRef);
}

export native function onChangeWithAttributeNative(displayAttributeOption: Array<string>, callback: Callback<long>,
    nativeObj: long): void;
}