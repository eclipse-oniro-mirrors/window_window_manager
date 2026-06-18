/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <gtest/gtest.h>
#include "window_extension_proxy.h"
#include "window_extension_client_proxy.h"
#include "window_extension_stub.h"
#include "window_extension_stub_impl.h"
#include "window_extension_client_interface.h"
#include "window_extension_client_stub_impl.h"
#include "mock_message_parcel.h"
#include "iremote_object_mocker.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
class WindowExtensionProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    sptr<WindowExtensionStub> mockWindowExtensionStub_;
    sptr<WindowExtensionProxy> windowExtensionProxy_;
    sptr<IRemoteObject> impl_;
    sptr<WindowExtensionClientProxy> windowExtensionClientProxy_;
};

void WindowExtensionProxyTest::SetUpTestCase()
{
}

void WindowExtensionProxyTest::TearDownTestCase()
{
}

void WindowExtensionProxyTest::SetUp()
{
    mockWindowExtensionStub_ = new WindowExtensionStubImpl("name");
    windowExtensionProxy_ = new WindowExtensionProxy(mockWindowExtensionStub_);
    impl_ = new IRemoteObjectMocker();
    ASSERT_NE(nullptr, impl_);
    windowExtensionClientProxy_ = new WindowExtensionClientProxy(impl_);
}

void WindowExtensionProxyTest::TearDown()
{
    MockMessageParcel::ClearAllErrorFlag();
}

namespace {

/**
 * @tc.name: OnWindowReady
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionProxyTest, OnWindowReady, TestSize.Level1)
{
    ASSERT_NE(nullptr, windowExtensionClientProxy_);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowExtensionClientProxy_->OnWindowReady(nullptr);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    windowExtensionClientProxy_->OnWindowReady(nullptr);

    struct RSSurfaceNodeConfig config;
    auto surfaceNode = RSSurfaceNode::Create(config);
    windowExtensionClientProxy_->OnWindowReady(surfaceNode);
}

/**
 * @tc.name: OnBackPress
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionProxyTest, OnBackPress, TestSize.Level1)
{
    ASSERT_NE(nullptr, windowExtensionClientProxy_);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowExtensionClientProxy_->OnBackPress();
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    windowExtensionClientProxy_->OnBackPress();
}

/**
 * @tc.name: OnKeyEvent
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionProxyTest, OnKeyEvent, TestSize.Level1)
{
    ASSERT_NE(nullptr, windowExtensionClientProxy_);
    std::shared_ptr<MMI::KeyEvent> keyEvent = MMI::KeyEvent::Create();
    ASSERT_NE(nullptr, keyEvent);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowExtensionClientProxy_->OnKeyEvent(keyEvent);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    windowExtensionClientProxy_->OnKeyEvent(keyEvent);
}

/**
 * @tc.name: OnPointerEvent
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionProxyTest, OnPointerEvent, TestSize.Level1)
{
    ASSERT_NE(nullptr, windowExtensionClientProxy_);
    std::shared_ptr<MMI::PointerEvent> pointerEvent = MMI::PointerEvent::Create();
    ASSERT_NE(nullptr, pointerEvent);
    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(true);
    windowExtensionClientProxy_->OnPointerEvent(pointerEvent);

    MockMessageParcel::SetWriteInterfaceTokenErrorFlag(false);
    windowExtensionClientProxy_->OnPointerEvent(pointerEvent);
}

/**
 * @tc.name: OnRemoteRequest
 * @tc.desc: test success
 * @tc.type: FUNC
 */
HWTEST_F(WindowExtensionProxyTest, OnRemoteRequest, TestSize.Level1)
{
    uint32_t code = 1000;
    MessageParcel data = {};
    MessageParcel reply = {};
    MessageOption option = {MessageOption::TF_SYNC};
    ASSERT_NE(-1, mockWindowExtensionStub_->OnRemoteRequest(code, data, reply, option));
}
}
}
}