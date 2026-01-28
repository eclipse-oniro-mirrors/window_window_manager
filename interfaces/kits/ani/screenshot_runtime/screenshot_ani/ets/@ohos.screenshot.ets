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
 
export default namespace screenshot {
 
loadLibrary('screenshotani_kit.z');
 
export function save(options: ScreenshotOptions, callback: AsyncCallback<image.PixelMap | undefined>): void {
    taskpool.execute((): image.PixelMap => {
        let res: image.PixelMap = saveSync(options);
        return res;
    }).then((ret: Any) => {
        callback(null, ret as image.PixelMap);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}
 
export function save(callback: AsyncCallback<image.PixelMap | undefined>): void {
    taskpool.execute((): image.PixelMap => {
        let res: image.PixelMap = saveSync();
        return res;
    }).then((ret: Any) => {
        callback(null, ret as image.PixelMap);
    }).catch((err: Any) => {
        callback(err as BusinessError, undefined);
    });
}
 
export function save(options?: ScreenshotOptions): Promise<image.PixelMap> {
    return new Promise<image.PixelMap>((resolve: (value: image.PixelMap) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute((): image.PixelMap => {
            let res: image.PixelMap = saveSync(options);
            return res;
        }).then((ret: Any) => {
            resolve(ret as image.PixelMap);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function saveHdrPicture(hdrOptions?: HdrScreenshotOptions): Promise<Array<image.PixelMap>> {
    return new Promise<Array<image.PixelMap>>((resolve: (value: Array<image.PixelMap>) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute((): Array<image.PixelMap> => {
            let res: Array<image.PixelMap> = saveHdrSync(hdrOptions);
            return res;
        }).then((ret: Any) => {
            resolve(ret as Array<image.PixelMap>);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function capture(options?: CaptureOption): Promise<image.PixelMap> {
    return new Promise<image.PixelMap>((resolve: (value: image.PixelMap) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute((): image.PixelMap => {
            let res: image.PixelMap = captureSync(options);
            return res;
        }).then((ret: Any) => {
            resolve(ret as image.PixelMap);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}

export function pick(): Promise<PickInfo> {
    return new Promise<PickInfo>((resolve: (value: PickInfo) => void, reject: (error: BusinessError) => void ) => {
        taskpool.execute((): PickInfo => {
            let res: PickInfo = pickSync();
            return res;
        }).then((ret: Any) => {
            resolve(ret as PickInfo);
        }).catch((err: Any) => {
            reject(err as BusinessError);
        });
    });
}
 
native function saveSync(screenshotOptions?: ScreenshotOptions): image.PixelMap;
native function saveHdrSync(hdrOptions?: HdrScreenshotOptions): Array<image.PixelMap>;
native function captureSync(options?: CaptureOption): image.PixelMap;
native function pickSync(): PickInfo;
 
export interface ScreenshotOptions {
    screenRect?: Rect;
 
    imageSize?: Size;
 
    rotation?: int;
 
    displayId?: long;
 
    isNotificationNeeded?: boolean;
 
    isCaptureFullOfScreen?: boolean;
}
 
export class ScreenshotOptionsImpl implements ScreenshotOptions{
    screenRect?: Rect;
 
    imageSize?: Size;
 
    rotation?: int;
 
    displayId?: long;
 
    isNotificationNeeded?: boolean;
    
    isCaptureFullOfScreen?: boolean;
}

export interface HdrScreenshotOptions {
    displayId?: long;
 
    isNotificationNeeded?: boolean;
 
    isCaptureFullOfScreen?: boolean;
}
 
export class HdrScreenshotOptionsImpl implements HdrScreenshotOptions {
    displayId?: long;
 
    isNotificationNeeded?: boolean;
 
    isCaptureFullOfScreen?: boolean;
}
 
export interface Size {
    width: long;
 
    height: long;
}
 
export class SizeImpl implements Size {
    width: long;
 
    height: long;
}
 
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

export interface CaptureOption {
    displayId?: Long;
}
 
export class CaptureOptionImpl implements CaptureOption {
    displayId?: Long;
}
 
export interface PickInfo {
    pickRect: Rect;
 
    pixelMap: image.PixelMap;
}
 
export class PickInfoImpl implements PickInfo {
    pickRect: Rect;
 
    pixelMap: image.PixelMap;
}
}