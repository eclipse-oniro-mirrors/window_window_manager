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

#ifndef THREAD_SAFETY_ANNOTATIONS_H
#define THREAD_SAFETY_ANNOTATIONS_H

// https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
// Enable thread safety attributes only with clang.
// The attributes can be safely erased when compiling with other compilers.
#if defined(__clang__) && (!defined(SWIG))
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   __attribute__((x))
#else
#define THREAD_ANNOTATION_ATTRIBUTE__(x)   // no-op
#endif

#define CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(capability(x))

#define SCOPED_CAPABILITY \
  THREAD_ANNOTATION_ATTRIBUTE__(scoped_lockable)

#define GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(guarded_by(x))

#define PT_GUARDED_BY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(pt_guarded_by(x))

#define ACQUIRED_BEFORE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_before(__VA_ARGS__))

#define ACQUIRED_AFTER(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquired_after(__VA_ARGS__))

#define REQUIRES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_capability(__VA_ARGS__))

#define REQUIRES_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(requires_shared_capability(__VA_ARGS__))

#define ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_capability(__VA_ARGS__))

#define ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(acquire_shared_capability(__VA_ARGS__))

#define RELEASE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_capability(__VA_ARGS__))

#define RELEASE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_shared_capability(__VA_ARGS__))

#define RELEASE_GENERIC(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(release_generic_capability(__VA_ARGS__))

#define TRY_ACQUIRE(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_capability(__VA_ARGS__))

#define TRY_ACQUIRE_SHARED(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(try_acquire_shared_capability(__VA_ARGS__))

#define EXCLUDES(...) \
  THREAD_ANNOTATION_ATTRIBUTE__(locks_excluded(__VA_ARGS__))

#define ASSERT_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_capability(x))

#define ASSERT_SHARED_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(assert_shared_capability(x))

#define RETURN_CAPABILITY(x) \
  THREAD_ANNOTATION_ATTRIBUTE__(lock_returned(x))

#define NO_THREAD_SAFETY_ANALYSIS \
  THREAD_ANNOTATION_ATTRIBUTE__(no_thread_safety_analysis)

namespace OHOS::Rosen {
class CAPABILITY("mutex") SingleThreadGuard {};

// ONLY used for those accessed ONLY on scene session manager thread.
// If other looper threads(NOT ffrt or ipc), define a new one.
constexpr SingleThreadGuard SCENE_SESSION_THREAD;

template <typename Guard>
struct SCOPED_CAPABILITY ScopedGuard final {
    explicit ScopedGuard(const Guard& guard) ACQUIRE(guard) {}
    ~ScopedGuard() RELEASE() {}

    ScopedGuard(const ScopedGuard&) = delete;
    ScopedGuard& operator=(const ScopedGuard&) = delete;
};

} // namespace OHOS::Rosen

// Use this for lambdas. THREAD_SAFETY_GUARD is preferred.
#define LOCK_GUARD(guard)  \
    ACQUIRE(guard) RELEASE(guard)

// Use this for expressions. THREAD_SAFETY_GUARD is preferred.
#define LOCK_GUARD_TWO(guard, expr)  \
    (OHOS::Rosen::ScopedGuard(guard), expr)

// Use this when LOCK_GUARD_TWO/THREAD_SAFETY_GUARD works failed.
#define LOCK_GUARD_EXPR(guard, expr)           \
    [&] {                                      \
        OHOS::Rosen::ScopedGuard lock(guard);  \
        return (expr);                         \
    }()

// Do not use this.
#define CREATE_THREAD_SAFETY_GUARD(opt1, opt2, guard, ...) guard

// Use THREAD_SAFETY_GUARD for common cases, including LOCK_GUARD and LOCK_GUARD_TWO.
#define THREAD_SAFETY_GUARD(...) \
    CREATE_THREAD_SAFETY_GUARD(__VA_ARGS__, LOCK_GUARD_TWO, LOCK_GUARD,)(__VA_ARGS__)

#endif // THREAD_SAFETY_ANNOTATIONS_H
