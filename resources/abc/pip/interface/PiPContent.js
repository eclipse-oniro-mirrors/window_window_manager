/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
const NodeController = requireNapi('arkui.node').NodeController;
const FrameNode = requireNapi('arkui.node').FrameNode;

const TAG = 'PiPContent';
const ABOUT_TO_STOP = 3;

class XCNodeController extends NodeController {
    constructor(m2) {
        super();
        this.node = null;
        this.mXComponent = m2;
    }
    makeNode(l2) {
        this.node = new FrameNode(l2);
        try {
            this.node.appendChild(this.mXComponent);
        } catch (err) {
            console.error('appendChild failed');
        }
        return this.node;
    }
    replaceNode(k) {
        this.node?.removeChild(this.mXComponent);
        this.mXComponent = k;
        this.node?.appendChild(this.mXComponent);
    }
    removeNode() {
        this.node?.removeChild(this.mXComponent);
    }
    aboutToDisappear() {
        this.node?.clearChildren();
    }
}

class PiPContent extends ViewPU {
    constructor(f2, g2, h2, i2 = -1, j2 = undefined, k2) {
        super(f2, h2, i2, k2);
        if (typeof f2 === 'function') {
            this.paramsGenerator_ = j2;
        }
        this.xComponentController = new XComponentController();
        this.nodeController = null;
        this.mXCNodeController = null;
        this.__useNode = new ObservedPropertySimplePU(false, this, 'useNode');
        this.__nodeChange = new ObservedPropertySimplePU(false, this, 'nodeChange');
        this.xComponent = null;
        this.xComponentId = 'pipContent';
        this.xComponentType = 'surface';
        this.setInitiallyProvidedValue(g2);
    }
    setInitiallyProvidedValue(e2) {
        if (e2.xComponentController !== undefined) {
            this.xComponentController = e2.xComponentController;
        }
        if (e2.nodeController !== undefined) {
            this.nodeController = e2.nodeController;
        }
        if (e2.mXCNodeController !== undefined) {
            this.mXCNodeController = e2.mXCNodeController;
        }
        if (e2.useNode !== undefined) {
            this.useNode = e2.useNode;
        }
        if (e2.nodeChange !== undefined) {
            this.nodeChange = e2.nodeChange;
        }
        if (e2.xComponent !== undefined) {
            this.xComponent = e2.xComponent;
        }
        if (e2.xComponentId !== undefined) {
            this.xComponentId = e2.xComponentId;
        }
        if (e2.xComponentType !== undefined) {
            this.xComponentType = e2.xComponentType;
        }
    }
    updateStateVars(d2) {
    }
    purgeVariableDependenciesOnElmtId(c2) {
        if (c2 === null || c2 === undefined) {
            return;
        }
        this.__useNode.purgeDependencyOnElmtId(c2);
        this.__nodeChange.purgeDependencyOnElmtId(c2);
    }
    aboutToBeDeleted() {
        this.__useNode.aboutToBeDeleted();
        this.__nodeChange.aboutToBeDeleted();
        SubscriberManager.Get().delete(this.id__());
        this.aboutToBeDeletedInternal();
    }
    get useNode() {
        return this.__useNode.get();
    }
    set useNode(j) {
        this.__useNode.set(j);
    }
    get nodeChange() {
        return this.__nodeChange.get();
    }
    set nodeChange(i) {
        this.__nodeChange.set(i);
    }
    validateNode(g) {
        if (g === null || g === undefined) {
            console.error(TAG, `validateNode node is null`);
            return false;
        }
        let h = g.getNodeType();
        if (h !== 'XComponent') {
            console.error(TAG, `node type mismatch: ${h}`);
            return false;
        }
        return true;
    }
    registerUpdateNodeListener() {
        pip.on('nodeUpdate', (f) => {
            console.info(TAG, `nodeUpdate`);
            if (!this.validateNode(f)) {
                return;
            }
            if (this.useNode) {
                pip.setPipNodeType(this.xComponent, false);
                this.updatePipNodeType(f);
                this.mXCNodeController?.replaceNode(f);
                this.nodeChange = true;
            }
            else {
                this.updatePipNodeType(f);
                this.mXCNodeController = new XCNodeController(f);
                console.info(TAG, 'update to Node Controller');
                this.registerStateChangeListener();
                this.useNode = true;
            }
        });
    }
    updatePipNodeType(c) {
        let d = c.getParent();
        if (d === null || d === undefined) {
            pip.setPipNodeType(c, false);
        }
        else {
            pip.setPipNodeType(c, true);
            d.removeChild(c);
        }
    }
    registerStateChangeListener() {
        pip.on('stateChange', (b) => {
            console.info(TAG, `stateChange state:${b}`);
            if (b === ABOUT_TO_STOP) {
                this.mXCNodeController?.removeNode();
            }
        });
    }
    aboutToAppear() {
        try {
            this.nodeController = pip.getCustomUIController();
            this.registerUpdateNodeListener();
            this.xComponent = pip.getTypeNode();
            if (!this.validateNode(this.xComponent)) {
                return;
            }
            if (this.xComponent === null) {
                console.error(TAG, `validateNode node is null`);
                return;
            }
            this.useNode = true;
            this.updatePipNodeType(this.xComponent);
            pip.setTypeNodeEnabled();
            this.mXCNodeController = new XCNodeController(this.xComponent);
            console.info(TAG, 'use Node Controller');
            this.registerStateChangeListener();
        } catch (a) {
            console.log(`aboutToAppear failed`);
        }
    }

    updatePipNodeType(a2) {
        let b2 = a2.getParent();
        if (b2 === null || b2 === undefined) {
            pip.setPipNodeType(a2, false);
        } else {
            pip.setPipNodeType(a2, true);
            b2.removeChild(a2);
        }
    }


    aboutToDisappear() {
        try {
            pip.off('stateChange');
            pip.off('nodeUpdate');
        } catch (b) {
            console.log(`aboutToDisappear failed`);
        }
    }

    initialRender() {
        this.observeComponentCreation2((s1, t1) => {
            Stack.create();
            Stack.size({ width: '100%', height: '100%' });
        }, Stack);
        this.observeComponentCreation2((h1, i1) => {
            If.create();
            if (this.useNode || this.nodeChange) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.buildNode.bind(this)();
                });
            } else {
                this.ifElseBranchUpdateFunction(1, () => {
                    this.buildXComponent.bind(this)();
                });
            }
        }, If);
        If.pop();
        this.observeComponentCreation2((a1, b1) => {
            If.create();
            if (this.nodeController !== null) {
                this.ifElseBranchUpdateFunction(0, () => {
                    this.buildCustomUI.bind(this)();
                });
            } else {
                this.ifElseBranchUpdateFunction(1, () => {
                });
            }
        }, If);
        If.pop();
        Stack.pop();
    }

    buildCustomUI(g1 = null) {
        this.observeComponentCreation2((i1, j1) => {
            NodeContainer.create(this.nodeController);
            NodeContainer.size({ width: '100%', height: '100%'});
        }, NodeContainer);
    }

    buildXComponent(b1 = null) {
        this.observeComponentCreation2((d1, e1) => {
            XComponent.create({
                id: this.xComponentId,
                type: this.xComponentType,
                controller: this.xComponentController
            }, 'pipContent_XComponent');
            XComponent.onLoad((() => {
                pip.initXComponentController(this.xComponentController);
                console.info(TAG, 'XComponent onLoad done');
            }));
            XComponent.size({ width: '100%', height: '100%' });
            XComponent.backgroundColor(Color.Transparent);
        }, XComponent);
    }

    buildNode(x = null) {
        this.observeComponentCreation2((z, a1) => {
            NodeContainer.create(this.mXCNodeController);
            NodeContainer.size({ width: '100%', height: '100%' });
        }, NodeContainer);
    }

    rerender() {
        this.updateDirtyElements();
    }

    static getEntryName() {
        return 'PiPContent';
    }
}

ViewStackProcessor.StartGetAccessRecordingFor(ViewStackProcessor.AllocateNewElmetIdForNextComponent());
loadDocument(new PiPContent(void 0, {}));
ViewStackProcessor.StopGetAccessRecording();