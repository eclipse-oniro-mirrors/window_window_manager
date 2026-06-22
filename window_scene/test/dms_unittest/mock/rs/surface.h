/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef INTERFACES_INNERKITS_SURFACE_SURFACE_H
#define INTERFACES_INNERKITS_SURFACE_SURFACE_H

#include <refbase.h>
#include "iremote_broker.h"

namespace OHOS {
struct Rect {
    int32_t x;
    int32_t y;
    int32_t w;
    int32_t h;

    bool operator==(const Rect& other) const
    {
        return x == other.x &&
               y == other.y &&
               w == other.w &&
               h == other.h;
    }

    bool operator!=(const Rect& other) const
    {
        return !(*this == other);
    }
};

class IBufferProducer : public IRemoteBroker {
public:
DECLARE_INTERFACE_DESCRIPTOR(u"surf.IBufferProducer");
};

class Surface : public RefBase {
public:
    static sptr<Surface> CreateSurfaceAsConsumer(std::string name = "noname")
    {
        return nullptr;
    }
    static sptr<Surface> CreateSurfaceAsProducer(sptr<IBufferProducer>& producer)
    {
        return nullptr;
    }

    virtual ~Surface() = default;

    virtual sptr<IBufferProducer> GetProducer()
    {
        return nullptr;
    }
};
} // namespace OHOS

#endif // INTERFACES_INNERKITS_SURFACE_SURFACE_H
