/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

const pip = requireNapi('pip');

const TAG = 'WebPiPContent';
class WebPiPContent extends ViewPU {
    constructor(r, s, i1, j1 = -1, v1 = undefined, w1) {
        super(r, i1, j1, w1);
        if (typeof v1 === 'function') {
            this.paramsGenerator_ = v1;
        }
        this.xComponentController = new XComponentController();
        this.xComponentId = 'webPipContent';
        this.xComponentType = XComponentType.SURFACE;
        this.setInitiallyProvidedValue(s);
    }
    setInitiallyProvidedValue(q) {
        if (q.xComponentController !== undefined) {
            this.xComponentController = q.xComponentController;
        }
        if (q.xComponentId !== undefined) {
            this.xComponentId = q.xComponentId;
        }
        if (q.xComponentType !== undefined) {
            this.xComponentType = q.xComponentType;
        }
    }
    updateStateVars(p) {
    }
    purgeVariableDependenciesOnElmtId(o) {
    }
    aboutToBeDeleted() {
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    initialRender() {
        this.observeComponentCreation2((m, n) => {
            Stack.create();
            Stack.size({ width: '100%', height: '100%' });
        }, Stack);
        this.buildXComponent.bind(this)();
        Stack.pop();
    }
    buildXComponent(j = null) {
        this.observeComponentCreation2((k, l) => {
            XComponent.create({
                id: this.xComponentId,
                type: this.xComponentType,
                controller: this.xComponentController
            }, 'webPipContent_XComponent');
            XComponent.onLoad(() => {
                pip.initWebXComponentController(this.xComponentController, this.xComponentController.getXComponentSurfaceId());
                console.debug(TAG, 'WebXComponent onLoad done');
            });
            XComponent.size({ width: '100%', height: '100%' });
            XComponent.backgroundColor(Color.Transparent);
        }, XComponent);
    }
    rerender() {
        this.updateDirtyElements();
    }
    static getEntryName() {
        return 'WebPiPContent';
    }
}

ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new WebPiPContent(void 0, {}));
ViewStackProcessor.StopGetAccessRecording();