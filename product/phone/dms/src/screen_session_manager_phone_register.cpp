/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "screen_session_manager.h"
#include "../include/screen_session_manager_phone_extension.h"

using namespace OHOS::Rosen;
using namespace PhoneExtension;

// extern "C"后最近一个定义为C符号
extern "C" __attribute__((constructor)) void PhoneScreenSessionManagerRegisterFunc()
{
    TLOGI(WmsLogTag::DMS, "startphone");
    ScreenSessionManager::SetInstance(static_cast<ScreenSessionManager*>(&ScreenSessionManagerExt::GetInstance()));
    TLOGI(WmsLogTag::DMS, "phoneScreenSessionManagerExt registered successfully");
}