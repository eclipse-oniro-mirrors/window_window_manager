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

#ifndef OHOS_ROSEN_WM_OCCLUSION_REGION_H
#define OHOS_ROSEN_WM_OCCLUSION_REGION_H

#include <refbase.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

namespace OHOS::Rosen::WmOcclusion {
class Rect {
public:
    int left_ = 0;
    int top_ = 0;
    int right_ = 0;
    int bottom_ = 0;
    static Rect _s_empty_rect_;
    static Rect _s_invalid_rect_;

    Rect() : left_(0), top_(0), right_(0), bottom_(0) {}
    Rect(int l, int t, int r, int b) : left_(l), top_(t), right_(r), bottom_(b) {}

    std::string GetRectInfo() const
    {
        return std::string("[" +
            std::to_string(left_) + ", " +
            std::to_string(top_) + ", " +
            std::to_string(right_ - left_) + ", " +
            std::to_string(bottom_ - top_) + "]");
    }

    bool IsEmpty() const
    {
        return left_ >= right_ || top_ >= bottom_;
    }
};

std::ostream& operator<<(std::ostream& os, const Rect& r);

/*
    Event: Used for record a rect edge in/out event
    y_: rect edge Y value
    type: OPEN/CLOSE: lhs rect in/out; VOID_OPEN/VOID_CLOSE: rhs rect in/out
 */
class Event {
public:
    // Use different value to differentiate lhs and rhs ranges.
    enum Type { OPEN = 1, CLOSE = -1, VOID_OPEN = 2, VOID_CLOSE = -2 };
    int y_ = 0;
    Type type_ = Type::OPEN;
    int left_ = 0;
    int right_ = 0;

    Event(int y, Type type, int l, int r) : y_(y), type_(type), left_(l), right_(r) {}
};

bool EventSortByY(const Event& e1, const Event& e2);

class Range {
public:
    int start_ = 0;
    int end_ = 0;
    Range(int start, int end) : start_(start), end_(end) {}
    bool operator==(const Range& r)
    {
        return start_ == r.start_ && end_ == r.end_;
    }
};

class Node {
public:
    int start_ = 0;
    int end_ = 0;
    int mid_ = 0;
    Node* left_ = nullptr;
    Node* right_ = nullptr;
    int positive_count_ = 0; // used for counting current lhs ranges
    int negative_count_ = 0; // used for counting current rhs ranges

    Node(int s, int e) : start_(s), end_(e), mid_((s + e) >> 1) {}
    ~Node()
    {
        if (right_ != nullptr) {
            delete right_;
            right_ = nullptr;
        }
        if (left_ != nullptr) {
            delete left_;
            left_ = nullptr;
        }
    }

    inline bool IsLeaf()
    {
        return left_ == nullptr && right_ == nullptr;
    }

    // push current node [start, end] into range result, merge last range if possible
    inline void PushRange(std::vector<Range>& res)
    {
        if (res.size() > 0 && start_ == res[res.size() - 1].end_) {
            // merge range with previous range if their end and start share same point
            res[res.size() - 1].end_ = end_;
        } else {
            res.emplace_back(Range { start_, end_ });
        }
    }

    // update segment tree
    void Update(int updateStart, int updateEnd, Event::Type type);
    // get ranges where positive_count_ or negtive_count_ is positive
    void GetOrRange(std::vector<Range>& res, bool isParentNodePos, bool isParentNodeNeg);
    // get ranges where positive_count_ and negtive_count_ are both positive
    void GetAndRange(std::vector<Range>& res, bool isParentNodePos, bool isParentNodeNeg);
    // get ranges where positive_count_ is positive and negtive_count_ not
    void GetSubRange(std::vector<Range>& res, bool isParentNodePos, bool isParentNodeNeg);
    // get ranges where either positive_count_ and negtive_count_ are both positive
    void GetXOrRange(std::vector<Range>& res, bool isParentNodePos, bool isParentNodeNeg);
};

class Region {
public:
    enum OP {
        // bit index 0: lhs
        // bit index 1: lhs & rhs
        // bit index 2: rhs
        AND = 2, // 010
        OR  = 7, // 111
        XOR = 5, // 101
        SUB = 1  // 001
    };

    Region() = default;
    explicit Region(Rect& rect)
    {
        rects_.push_back(rect);
        bound_ = Rect { rect };
    }

    Region(const Region& region) : rects_(region.rects_), bound_(region.bound_) {}
    Region& operator=(const Region& region)
    {
        rects_ = region.rects_;
        bound_ = region.bound_;
        return *this;
    }
    ~Region() {}

    std::vector<Rect>& GetRegionRects()
    {
        return rects_;
    }

    std::vector<Rect> GetRegionRects() const
    {
        return rects_;
    }

    bool IsEmpty() const
    {
        return rects_.size() == 0;
    }

    int GetSize() const
    {
        return rects_.size();
    }

    Rect& GetBoundRef()
    {
        return bound_;
    }

    Rect GetBound() const
    {
        return bound_;
    }

    std::string GetRegionInfo() const
    {
        std::string info = "{ Region Size " + std::to_string(rects_.size()) + ": ";
        for (auto& rect : rects_) {
            info.append(rect.GetRectInfo());
        }
        info.append(" }");
        return info;
    }

    inline size_t Size() const
    {
        return rects_.size();
    }

    inline std::vector<Rect>::const_iterator CBegin() const
    {
        return rects_.cbegin();
    }

    inline std::vector<Rect>::iterator Begin()
    {
        return rects_.begin();
    }

    inline std::vector<Rect>::const_iterator CEnd() const
    {
        return rects_.cend();
    }

    inline std::vector<Rect>::const_iterator End()
    {
        return rects_.end();
    }

    /* core Region logic operation function, the return region's rects is guaranteed no-intersection
        (rect in rects_ do not intersect with each other)
    */
    void RegionOp(Region& r1, Region& r2, Region& res, Region::OP op);
    void RegionOpLocal(Region& r1, Region& r2, Region& res, Region::OP op);
    // bound of all region rects
    void MakeBound();

    // return merge region
    Region Or(Region& r);
    // return intersection region
    Region And(Region& r);
    // return region belongs to Region(lhs) but not Region(rhs)
    Region Sub(Region& r);
    // return merge region subtract intersection region
    Region Xor(Region& r);

    Region& OperationSelf(Region& r, Region::OP op);
    // replace region with or result
    Region& OrSelf(Region& r);
    // replace region with and result
    Region& AndSelf(Region& r);
    // replace region with sub result
    Region& SubSelf(Region& r);
    // replace region with xor result
    Region& XOrSelf(Region& r);

private:
    class Rects {
    public:
        int preY = 0;
        int curY = 0;
        std::vector<Rect> preRects;
        std::vector<Rect> curRects;
    };
    // update tmp rects and region according to current ranges
    void UpdateRects(Rects& r, std::vector<Range>& ranges, std::vector<int>& indexAt, Region& res);
    // get ranges from segmentTree node according to logical operation type
    void getRange(std::vector<Range>& ranges, Node& node, OP op);

private:
    std::vector<Rect> rects_;
    Rect bound_;
    static bool _s_so_loaded_;
};
std::ostream& operator<<(std::ostream& os, const Region& r);
} // namespace OHOS::Rosen::Occlusion
#endif // OHOS_ROSEN_WM_OCCLUSION_REGION_H