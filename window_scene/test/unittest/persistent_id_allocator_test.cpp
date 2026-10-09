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

#include <cstdint>
#include <set>

#include <gtest/gtest.h>

#include "session/persistent_id/include/persistent_id_allocator.h"
#include "ws_common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace Rosen {
namespace {
constexpr uint32_t EXTENSION_ID_FLAG = 0x40000000;
constexpr int32_t KEY_ID_STRIDE_DEFAULT = 4;
constexpr int32_t KEY_ID_STRIDE_LARGE_SCENE = 16;
constexpr int32_t LOCAL_FALLBACK_ID_BASE = 1000000;
constexpr int32_t LOCAL_FALLBACK_ID_LIMIT = 2000000;
constexpr int32_t LOCAL_ID_STRIDE = 1000;
constexpr int32_t LOCAL_ID_RECORD_COUNT = 1000;
} // namespace

class PersistentIdAllocatorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: ComposeSessionId01
 * @tc.desc: The keyId plan composes a session id as recordId * stride + keyId; with
 *           a fixed stride, different keyIds never produce the same id rail and one
 *           rail advances by exactly the stride between consecutive recordIds.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ComposeSessionId01, TestSize.Level1)
{
    // Basics of the formula for both product scenes.
    EXPECT_EQ(PersistentIdAllocator::ComposeSessionId(2, 1, KEY_ID_STRIDE_DEFAULT), 2 * 4 + 1);
    EXPECT_EQ(PersistentIdAllocator::ComposeSessionId(2, 4, KEY_ID_STRIDE_DEFAULT), 2 * 4 + 4);
    EXPECT_EQ(PersistentIdAllocator::ComposeSessionId(2, 1, KEY_ID_STRIDE_LARGE_SCENE), 2 * 16 + 1);
    EXPECT_EQ(PersistentIdAllocator::ComposeSessionId(2, 16, KEY_ID_STRIDE_LARGE_SCENE), 2 * 16 + 16);

    // Different keyIds with the same recordId never collide within one stride space.
    for (uint32_t keyId = 1; keyId <= KEY_ID_STRIDE_DEFAULT; ++keyId) {
        for (uint32_t other = keyId + 1; other <= KEY_ID_STRIDE_DEFAULT; ++other) {
            EXPECT_NE(PersistentIdAllocator::ComposeSessionId(7, keyId, KEY_ID_STRIDE_DEFAULT),
                PersistentIdAllocator::ComposeSessionId(7, other, KEY_ID_STRIDE_DEFAULT));
        }
    }

    // One rail advances by the stride between consecutive recordIds.
    for (uint32_t keyId = 1; keyId <= KEY_ID_STRIDE_LARGE_SCENE; ++keyId) {
        EXPECT_EQ(PersistentIdAllocator::ComposeSessionId(9, keyId, KEY_ID_STRIDE_LARGE_SCENE) -
            PersistentIdAllocator::ComposeSessionId(8, keyId, KEY_ID_STRIDE_LARGE_SCENE),
            KEY_ID_STRIDE_LARGE_SCENE);
    }
}

/**
 * @tc.name: ComposeSessionId02
 * @tc.desc: The last recordId that still composes an id inside the legacy 30-bit
 *           id space does so for the widest stride and the largest keyId.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ComposeSessionId02, TestSize.Level1)
{
    // Largest legal recordId for stride 16 / keyId 16: (0x3fffffff - keyId) / stride.
    constexpr uint32_t maxRecordId =
        (0x3fffffff - KEY_ID_STRIDE_LARGE_SCENE) / KEY_ID_STRIDE_LARGE_SCENE;
    int32_t persistentId = PersistentIdAllocator::ComposeSessionId(
        maxRecordId, KEY_ID_STRIDE_LARGE_SCENE, KEY_ID_STRIDE_LARGE_SCENE);
    EXPECT_GT(persistentId, 0);
    EXPECT_LE(static_cast<uint32_t>(persistentId), 0x3fffffffu);
}

/**
 * @tc.name: ComposeLocalSessionId01
 * @tc.desc: The fallback rule composes 1000000 + uid % 1000 * 1000 + recordId: ids
 *           stay inside the reserved range [1000000, 2000000) while recordId stays
 *           below 1000, and different uid tags never collide on the same recordId.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ComposeLocalSessionId01, TestSize.Level1)
{
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(0, 0), LOCAL_FALLBACK_ID_BASE);
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(999, 999),
        LOCAL_FALLBACK_ID_BASE + 999 * LOCAL_ID_STRIDE + 999); // last id of the reserved range
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(0, 1000), LOCAL_FALLBACK_ID_BASE); // uid mod 1000
    for (uint32_t recordId = 0; recordId < LOCAL_ID_RECORD_COUNT; ++recordId) {
        int32_t persistentId = PersistentIdAllocator::ComposeLocalSessionId(recordId, 4242);
        EXPECT_GE(persistentId, LOCAL_FALLBACK_ID_BASE);
        EXPECT_LT(persistentId, LOCAL_FALLBACK_ID_LIMIT);
    }
}

/**
 * @tc.name: ComposeLocalSessionId02
 * @tc.desc: Consecutive recordIds advance by 1 inside the uid-tag block and the
 *           recordId keeps growing past 1000 instead of wrapping, so the fallback
 *           generation never runs out.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ComposeLocalSessionId02, TestSize.Level1)
{
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(1, 77) -
        PersistentIdAllocator::ComposeLocalSessionId(0, 77), 1);
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(5, 8) -
        PersistentIdAllocator::ComposeLocalSessionId(5, 7), LOCAL_ID_STRIDE);
    // Past the reserved block the ids keep growing without wrapping.
    EXPECT_EQ(PersistentIdAllocator::ComposeLocalSessionId(1000, 7),
        PersistentIdAllocator::ComposeLocalSessionId(0, 8));
    EXPECT_GT(PersistentIdAllocator::ComposeLocalSessionId(1001, 999),
        LOCAL_FALLBACK_ID_LIMIT - 1);
}

/**
 * @tc.name: ExtensionLocalId01
 * @tc.desc: Extension ids keep the legacy pid-assembled 0x40000000 format and are
 *           generated locally without any IPC.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, ExtensionLocalId01, TestSize.Level1)
{
    for (int32_t i = 0; i < 3; ++i) {
        int32_t persistentId = PersistentIdAllocator::GetInstance().Acquire(true, INVALID_SESSION_ID);
        EXPECT_NE(persistentId, INVALID_SESSION_ID);
        EXPECT_NE(static_cast<uint32_t>(persistentId) & EXTENSION_ID_FLAG, 0u);
    }
}

/**
 * @tc.name: SpecifiedId01
 * @tc.desc: An unused specified id is adopted as-is; requesting the same id again
 *           falls back to the auto generation path (legacy semantics, no
 *           same-process takeover).
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, SpecifiedId01, TestSize.Level1)
{
    constexpr int32_t specifiedId = 50000001; // far away from ids the suite generates itself
    EXPECT_EQ(PersistentIdAllocator::GetInstance().Acquire(false, specifiedId), specifiedId);
    int32_t renumbered = PersistentIdAllocator::GetInstance().Acquire(false, specifiedId);
    EXPECT_NE(renumbered, specifiedId);
    EXPECT_NE(renumbered, INVALID_SESSION_ID);
    EXPECT_EQ(static_cast<uint32_t>(renumbered) & EXTENSION_ID_FLAG, 0u);
}

/**
 * @tc.name: LocalFallback01
 * @tc.desc: In a unit-test environment the foundation service is unreachable, so
 *           the keyId plan is unavailable and non-extension ids fall back to the
 *           reserved local range: valid ids at or above 1000000, without the
 *           extension flag, and never duplicated within this process.
 * @tc.type: FUNC
 */
HWTEST_F(PersistentIdAllocatorTest, LocalFallback01, TestSize.Level1)
{
    constexpr int32_t fallbackCount = 5;
    std::set<int32_t> collected;
    for (int32_t i = 0; i < fallbackCount; ++i) {
        int32_t persistentId = PersistentIdAllocator::GetInstance().Acquire(false, INVALID_SESSION_ID);
        EXPECT_NE(persistentId, INVALID_SESSION_ID);
        EXPECT_GE(persistentId, LOCAL_FALLBACK_ID_BASE);
        EXPECT_EQ(static_cast<uint32_t>(persistentId) & EXTENSION_ID_FLAG, 0u);
        collected.insert(persistentId);
    }
    EXPECT_EQ(collected.size(), static_cast<size_t>(fallbackCount));
}
} // namespace Rosen
} // namespace OHOS
