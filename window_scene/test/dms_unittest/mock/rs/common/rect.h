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

#ifndef RECT_H
#define RECT_H

#include <cmath>
#include <string>

typedef float scalar;

namespace OHOS {
namespace Rosen {
namespace Drawing {
class RectF;

typedef RectF Rect;

#define DRAWING_MAX_S32_FITS_IN_FLOAT    2147483520
#define DRAWING_MIN_S32_FITS_IN_FLOAT    (-DRAWING_MAX_S32_FITS_IN_FLOAT)

class RectI {
public:
    inline RectI() noexcept;
    inline RectI(const RectI& r) noexcept;
    inline RectI(const int32_t l, const int32_t t, const int32_t r, const int32_t b) noexcept;

    ~RectI() {}

    inline int32_t GetLeft() const
    {
        return left_;
    }
    inline int32_t GetTop() const
    {
        return top_;
    }
    inline int32_t GetRight() const
    {
        return right_;
    }
    inline int32_t GetBottom() const
    {
        return bottom_;
    }

    int32_t left_;
    int32_t top_;
    int32_t right_;
    int32_t bottom_;
};

inline RectI::RectI() noexcept : left_(0), top_(0), right_(0), bottom_(0) {}

inline RectI::RectI(const RectI& r) noexcept
{
    // Tell the compiler there is no alias and to select wider load/store instructions.
    int32_t left = r.GetLeft();
    int32_t top = r.GetTop();
    int32_t right = r.GetRight();
    int32_t bottom = r.GetBottom();
    left_ = left;
    top_ = top;
    right_ = right;
    bottom_ = bottom;
}

inline RectI::RectI(const int l, const int t, const int r, const int b) noexcept
    : left_(l), top_(t), right_(r), bottom_(b)
{}

class RectF {
public:
    inline RectF() noexcept;
    inline RectF(const RectF& r) noexcept;
    inline RectF(const RectI& r) noexcept;
    inline RectF(const scalar l, const scalar t, const scalar r, const scalar b) noexcept;

    ~RectF() {}

    inline bool IsValid() const;
    inline bool IsEmpty() const;

    inline scalar GetLeft() const;
    inline scalar GetTop() const;
    inline scalar GetRight() const;
    inline scalar GetBottom() const;

    inline scalar GetWidth() const;
    inline scalar GetHeight() const;

    inline void SetLeft(scalar pos);
    inline void SetTop(scalar pos);
    inline void SetRight(scalar pos);
    inline void SetBottom(scalar pos);

    scalar left_;
    scalar top_;
    scalar right_;
    scalar bottom_;
};

inline RectF::RectF() noexcept : left_(0.0), top_(0.0), right_(0.0), bottom_(0.0) {}

inline RectF::RectF(const RectF& r) noexcept
{
    // Tell the compiler there is no alias and to select wider load/store instructions.
    scalar left = r.GetLeft();
    scalar top = r.GetTop();
    scalar right = r.GetRight();
    scalar bottom = r.GetBottom();
    left_ = left;
    top_ = top;
    right_ = right;
    bottom_ = bottom;
}

inline RectF::RectF(const RectI& r) noexcept
{
    // Tell the compiler there is no alias and to select wider load/store instructions.
    scalar left = r.GetLeft();
    scalar top = r.GetTop();
    scalar right = r.GetRight();
    scalar bottom = r.GetBottom();
    left_ = left;
    top_ = top;
    right_ = right;
    bottom_ = bottom;
}

inline RectF::RectF(const scalar l, const scalar t, const scalar r, const scalar b) noexcept
    : left_(l), top_(t), right_(r), bottom_(b)
{}

inline bool RectF::IsValid() const
{
    return left_ < right_ && top_ < bottom_;
}

inline bool RectF::IsEmpty() const
{
    return !(left_ < right_ && top_ < bottom_);
}

inline scalar RectF::GetLeft() const
{
    return left_;
}

inline scalar RectF::GetTop() const
{
    return top_;
}

inline scalar RectF::GetRight() const
{
    return right_;
}

inline scalar RectF::GetBottom() const
{
    return bottom_;
}

inline scalar RectF::GetWidth() const
{
    return right_ - left_;
}

inline scalar RectF::GetHeight() const
{
    return bottom_ - top_;
}

inline void RectF::SetLeft(scalar pos)
{
    left_ = pos;
}

inline void RectF::SetTop(scalar pos)
{
    top_ = pos;
}

inline void RectF::SetRight(scalar pos)
{
    right_ = pos;
}

inline void RectF::SetBottom(scalar pos)
{
    bottom_ = pos;
}


} // namespace Drawing
} // namespace Rosen
} // namespace OHOS
#endif
