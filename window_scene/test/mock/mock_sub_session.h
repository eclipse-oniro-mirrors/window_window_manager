/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ROSEN_WINDOW_SCENE_MOCK_SUB_SESSION_H
#define OHOS_ROSEN_WINDOW_SCENE_MOCK_SUB_SESSION_H
#include "session/host/include/sub_session.h"

namespace OHOS {
namespace Rosen {
class SubSessionMocker : public SubSession {
public:
    SubSessionMocker(const SessionInfo& info, const sptr<SpecificSessionCallback>& specificCallback)
        : SubSession(info, specificCallback) {}
    ~SubSessionMocker() {}
    MOCK_METHOD(void, SetScreenId, (uint64_t screenId), (override));
    MOCK_METHOD(void, NotifySessionRectChange, (const WSRect& rect,
        SizeChangeReason reason, DisplayId displayId), (override));
};
}
}
#endif // OHOS_ROSEN_WINDOW_SCENE_MOCK_SUB_SESSION_H
