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

#ifndef RENDER_SERVICE_CLIENT_CORE_COMMON_RS_VECTOR4_H
#define RENDER_SERVICE_CLIENT_CORE_COMMON_RS_VECTOR4_H

#include <algorithm>
#include <cmath>

namespace OHOS {
namespace Rosen {
namespace {
    constexpr uint32_t INDEX_ONE = 1;
    constexpr uint32_t INDEX_TWO = 2;
    constexpr uint32_t INDEX_THREE = 3;
}
template<typename T>
inline constexpr bool ROSEN_EQ4(const T& x, const T& y)
{
    if constexpr (std::is_floating_point<T>::value) {
        return (std::abs((x) - (y)) <= (std::numeric_limits<T>::epsilon()));
    } else {
        return x == y;
    }
}

template<typename T>
class Vector4 {
public:
    static constexpr uint32_t V4SIZE = 4;
    static constexpr size_t DATA_SIZE = sizeof(T) * V4SIZE;
    union {
        struct {
            T x_;
            T y_;
            T z_;
            T w_;
        };
        T data_[V4SIZE];
    };

    Vector4();
    Vector4(T value);
    Vector4(const Vector4<T>& value);
    Vector4(T x, T y, T z, T w);
    explicit Vector4(const T* array);
    ~Vector4();

    Vector4 Normalized() const;
    T Dot(const Vector4<T>& other) const;
    T GetSqrLength() const;
    T GetLength() const;
    T Normalize();
    void Identity();
    bool IsInfinite() const;
    bool IsNaN() const;
    bool IsValid() const;
    bool IsIdentity() const;
    bool IsZero() const;
    void SetValues(T x, T y, T z, T w);
    void SetZero();
    uint32_t Size();
    Vector4 operator-() const;
    Vector4 operator-(const Vector4<T>& other) const;
    Vector4 operator+(const Vector4<T>& other) const;
    Vector4 operator/(float scale) const;
    Vector4 operator*(float scale) const;
    Vector4 operator*(const Vector4<T>& other) const;
    Vector4& operator*=(const Vector4<T>& other);
    Vector4& operator+=(const Vector4<T>& other);
    Vector4& operator=(const Vector4<T>& other);
    bool operator==(const Vector4& other) const;
    bool operator!=(const Vector4& other) const;
    bool IsNearEqual(const Vector4& other, T threshold = std::numeric_limits<T>::epsilon()) const;

    T operator[](int index) const;
    T& operator[](int index);
    T* GetData();

    void Scale(float arg);
    void Sub(const Vector4<T>& arg);
    void Add(const Vector4<T>& arg);
    void Multiply(const Vector4<T>& arg);
    void Negate();
    void Absolute();
    static void Min(const Vector4<T>& a, const Vector4<T>& b, Vector4<T>& result);
    static void Max(const Vector4<T>& a, const Vector4<T>& b, Vector4<T>& result);
    static void Mix(const Vector4<T>& min, const Vector4<T>& max, T a, Vector4<T>& result);
};

typedef Vector4<float> Vector4f;
typedef Vector4<double> Vector4d;

class Quaternion : public Vector4f {
public:
    Quaternion()
    {
        Identity();
    }
    Quaternion(float x, float y, float z, float w) : Vector4f(x, y, z, w) {}
    Quaternion(const Vector4f& other) : Vector4f(other) {}
    Quaternion(const Vector4f&& other) : Vector4f(other) {}
    Quaternion Flip() const;
};

template<typename T>
Vector4<T>::Vector4()
{
    SetZero();
}

template<typename T>
Vector4<T>::Vector4(T value)
{
    data_[0] = value;
    data_[INDEX_ONE] = value;
    data_[INDEX_TWO] = value;
    data_[INDEX_THREE] = value;
}

template<typename T>
Vector4<T>::Vector4(const Vector4<T>& value)
{
    // Tell the compiler there is no alias and to select wider load/store
    // instructions.
    T data0 = value[0];
    T data1 = value[INDEX_ONE];
    T data2 = value[INDEX_TWO];
    T data3 = value[INDEX_THREE];
    data_[0] = data0;
    data_[INDEX_ONE] = data1;
    data_[INDEX_TWO] = data2;
    data_[INDEX_THREE] = data3;
}

template<typename T>
Vector4<T>::Vector4(T x, T y, T z, T w)
{
    data_[0] = x;
    data_[INDEX_ONE] = y;
    data_[INDEX_TWO] = z;
    data_[INDEX_THREE] = w;
}

template<typename T>
Vector4<T>::Vector4(const T* array)
{
    std::copy_n(array, std::size(data_), data_);
}

template<typename T>
Vector4<T>::~Vector4()
{}

inline Quaternion Quaternion::Flip() const
{
    return { -data_[0], -data_[INDEX_ONE], -data_[INDEX_TWO], -data_[INDEX_THREE] };
}

template<typename T>
Vector4<T> Vector4<T>::Normalized() const
{
    Vector4<T> rNormalize(*this);
    rNormalize.Normalize();
    return rNormalize;
}

template<typename T>
T Vector4<T>::Dot(const Vector4<T>& other) const
{
    const T* oData = other.data_;
    T sum = data_[0] * oData[0];
    sum += data_[INDEX_ONE] * oData[INDEX_ONE];
    sum += data_[INDEX_TWO] * oData[INDEX_TWO];
    sum += data_[INDEX_THREE] * oData[INDEX_THREE];
    return sum;
}

template<typename T>
T Vector4<T>::GetSqrLength() const
{
    T sum = data_[0] * data_[0];
    sum += data_[INDEX_ONE] * data_[INDEX_ONE];
    sum += data_[INDEX_TWO] * data_[INDEX_TWO];
    sum += data_[INDEX_THREE] * data_[INDEX_THREE];
    return sum;
}

template<typename T>
T Vector4<T>::GetLength() const
{
    return sqrt(GetSqrLength());
}

template<typename T>
T Vector4<T>::Normalize()
{
    T l = GetLength();
    if (ROSEN_EQ4<T>(l, 0.0)) {
        return (T)0.0;
    }

    const T d = 1.0f;
    if (l != 0) {
        d = 1.0f / l;
    }
    data_[0] *= d;
    data_[INDEX_ONE] *= d;
    data_[INDEX_TWO] *= d;
    data_[INDEX_THREE] *= d;
    return l;
}

template<typename T>
void Vector4<T>::Min(const Vector4<T>& a, const Vector4<T>& b, Vector4<T>& result)
{
    T* resultData = result.data_;
    const T* aData = a.data_;
    const T* bData = b.data_;
    // Tell the compiler there is no alias and to select wider load/store
    // instructions.
    T aData3 = aData[INDEX_THREE];
    T aData2 = aData[INDEX_TWO];
    T aData1 = aData[INDEX_ONE];
    T aData0 = aData[0];
    T bData3 = bData[INDEX_THREE];
    T bData2 = bData[INDEX_TWO];
    T bData1 = bData[INDEX_ONE];
    T bData0 = bData[0];
    resultData[INDEX_THREE] = std::min(aData3, bData3);
    resultData[INDEX_TWO] = std::min(aData2, bData2);
    resultData[INDEX_ONE] = std::min(aData1, bData1);
    resultData[0] = std::min(aData0, bData0);
}

template<typename T>
void Vector4<T>::Max(const Vector4<T>& a, const Vector4<T>& b, Vector4<T>& result)
{
    T* resultData = result.data_;
    const T* aData = a.data_;
    const T* bData = b.data_;
    // Tell the compiler there is no alias and to select wider load/store
    // instructions.
    T aData3 = aData[INDEX_THREE];
    T aData2 = aData[INDEX_TWO];
    T aData1 = aData[INDEX_ONE];
    T aData0 = aData[0];
    T bData3 = bData[INDEX_THREE];
    T bData2 = bData[INDEX_TWO];
    T bData1 = bData[INDEX_ONE];
    T bData0 = bData[0];
    resultData[INDEX_THREE] = std::max(aData3, bData3);
    resultData[INDEX_TWO] = std::max(aData2, bData2);
    resultData[INDEX_ONE] = std::max(aData1, bData1);
    resultData[0] = std::max(aData0, bData0);
}

template<typename T>
void Vector4<T>::Mix(const Vector4<T>& min, const Vector4<T>& max, T a, Vector4<T>& result)
{
    T* resultData = result.data_;
    const T* minData = min.data_;
    const T* maxData = max.data_;
    // Tell the compiler there is no alias and to select wider load/store
    // instructions.
    T minData3 = minData[INDEX_THREE];
    T minData2 = minData[INDEX_TWO];
    T minData1 = minData[1];
    T minData0 = minData[0];
    T maxData3 = maxData[INDEX_THREE];
    T maxData2 = maxData[INDEX_TWO];
    T maxData1 = maxData[1];
    T maxData0 = maxData[0];
    resultData[INDEX_THREE] = minData3 + a * (maxData3 - minData3);
    resultData[INDEX_TWO] = minData2 + a * (maxData2 - minData2);
    resultData[1] = minData1 + a * (maxData1 - minData1);
    resultData[0] = minData0 + a * (maxData0 - minData0);
}

template<typename T>
inline T* Vector4<T>::GetData()
{
    return data_;
}

template<typename T>
void Vector4<T>::Identity()
{
    SetValues(0.f, 0.f, 0.f, 1.f);
}

template<typename T>
bool Vector4<T>::IsIdentity() const
{
    return operator==(Vector4<T>(0.f, 0.f, 0.f, 1.f));
}

template<typename T>
bool Vector4<T>::IsZero() const
{
    return ROSEN_EQ4<T>(data_[0], 0.f) && ROSEN_EQ4<T>(data_[1], 0.f) &&
           ROSEN_EQ4<T>(data_[INDEX_TWO], 0.f) && ROSEN_EQ4<T>(data_[INDEX_THREE], 0.f);
}

template<typename T>
void Vector4<T>::SetValues(T x, T y, T z, T w)
{
    data_[0] = x;
    data_[1] = y;
    data_[INDEX_TWO] = z;
    data_[INDEX_THREE] = w;
}

template<typename T>
void Vector4<T>::SetZero()
{
    SetValues(T(0.f), T(0.f), T(0.f), T(0.f));
}

template<typename T>
uint32_t Vector4<T>::Size()
{
    return V4SIZE;
}

template<typename T>
Vector4<T> Vector4<T>::operator-(const Vector4<T>& other) const
{
    const T* otherData = other.data_;

    return Vector4<T>(
        data_[0] - otherData[0],
        data_[1] - otherData[1],
        data_[INDEX_TWO] - otherData[INDEX_TWO],
        data_[INDEX_THREE] - otherData[INDEX_THREE]);
}

template<typename T>
Vector4<T> Vector4<T>::operator+(const Vector4<T>& other) const
{
    const T* thisData = data_;
    const T* otherData = other.data_;

    return Vector4<T>(
        thisData[0] + otherData[0],
        thisData[1] + otherData[1],
        thisData[INDEX_TWO] + otherData[INDEX_TWO],
        thisData[INDEX_THREE] + otherData[INDEX_THREE]);
}

template<typename T>
Vector4<T> Vector4<T>::operator/(float scale) const
{
    if (ROSEN_EQ4<float>(scale, 0)) {
        return *this;
    }
    Vector4<T> clone(data_);
    clone.Scale(1.0f / scale);
    return clone;
}

template<typename T>
Vector4<T> Vector4<T>::operator*(float scale) const
{
    Vector4<T> clone(data_);
    clone.Scale(scale);
    return clone;
}

template<typename T>
Vector4<T> Vector4<T>::operator*(const Vector4<T>& other) const
{
    Vector4<T> rMult(data_);
    return rMult *= other;
}

template<typename T>
Vector4<T>& Vector4<T>::operator*=(const Vector4<T>& other)
{
    const T* oData = other.data_;
    // Tell the compiler there is no alias and to select wider load/store
    // instructions.
    T data3 = oData[INDEX_THREE];
    T data2 = oData[INDEX_TWO];
    T data1 = oData[1];
    T data0 = oData[0];
    data_[0] *= data0;
    data_[1] *= data1;
    data_[INDEX_TWO] *= data2;
    data_[INDEX_THREE] *= data3;
    return *this;
}

template<typename T>
Vector4<T>& Vector4<T>::operator+=(const Vector4<T>& other)
{
    const T* oData = other.data_;
    data_[0] += oData[0]; // 0, x component of the quaternion
    data_[1] += oData[1]; // 1, y component of the quaternion
    data_[INDEX_TWO] += oData[INDEX_TWO]; // 2, z component of the quaternion
    data_[INDEX_THREE] += oData[INDEX_THREE]; // 3, w component of the quaternion
    return *this;
}

template<typename T>
Vector4<T>& Vector4<T>::operator=(const Vector4<T>& other)
{
    const T* oData = other.data_;
    // Tell the compiler there is no alias and to select wider load/store
    // instructions.
    T data3 = oData[INDEX_THREE];
    T data2 = oData[INDEX_TWO];
    T data1 = oData[1];
    T data0 = oData[0];
    data_[0] = data0;
    data_[1] = data1;
    data_[INDEX_TWO] = data2;
    data_[INDEX_THREE] = data3;
    return *this;
}

template<typename T>
inline bool Vector4<T>::operator==(const Vector4& other) const
{
    const T* oData = other.data_;

    return (ROSEN_EQ4<T>(data_[0], oData[0])) && (ROSEN_EQ4<T>(data_[1], oData[1])) &&
        (ROSEN_EQ4<T>(data_[INDEX_TWO], oData[INDEX_TWO])) && (ROSEN_EQ4<T>(data_[INDEX_THREE], oData[INDEX_THREE]));
}

template<typename T>
inline bool Vector4<T>::operator!=(const Vector4& other) const
{
    return !operator==(other);
}

template<typename T>
bool Vector4<T>::IsNearEqual(const Vector4& other, T threshold) const
{
    const T* value = other.data_;

    return (ROSEN_EQ4<T>(data_[0], value[0], threshold)) && (ROSEN_EQ4<T>(data_[1], value[1], threshold)) &&
        (ROSEN_EQ4<T>(data_[INDEX_TWO], value[INDEX_TWO], threshold)) &&
        (ROSEN_EQ4<T>(data_[INDEX_THREE], value[INDEX_THREE], threshold));
}

template<typename T>
Vector4<T> Vector4<T>::operator-() const
{
    return Vector4<T>(-data_[0], -data_[1], -data_[INDEX_TWO], -data_[INDEX_THREE]);
}

template<typename T>
T Vector4<T>::operator[](int index) const
{
    return data_[index];
}

template<typename T>
T& Vector4<T>::operator[](int index)
{
    return data_[index];
}

template<typename T>
void Vector4<T>::Scale(float arg)
{
    data_[INDEX_THREE] *= arg;
    data_[INDEX_TWO] *= arg;
    data_[1] *= arg;
    data_[0] *= arg;
}

template<typename T>
void Vector4<T>::Sub(const Vector4<T>& arg)
{
    const T* argData = arg.data_;
    // Tell the compiler there is no alias and to select wider load/store
    // instructions.
    T data3 = argData[INDEX_THREE];
    T data2 = argData[INDEX_TWO];
    T data1 = argData[1];
    T data0 = argData[0];
    data_[INDEX_THREE] -= data3;
    data_[INDEX_TWO] -= data2;
    data_[1] -= data1;
    data_[0] -= data0;
}

template<typename T>
void Vector4<T>::Add(const Vector4<T>& arg)
{
    const T* argData = arg.data_;
    // Tell the compiler there is no alias and to select wider load/store
    // instructions.
    T data3 = argData[INDEX_THREE];
    T data2 = argData[INDEX_TWO];
    T data1 = argData[1];
    T data0 = argData[0];
    data_[INDEX_THREE] += data3;
    data_[INDEX_TWO] += data2;
    data_[1] += data1;
    data_[0] += data0;
}

template<typename T>
void Vector4<T>::Multiply(const Vector4<T>& arg)
{
    const T* argData = arg.data_;
    // Tell the compiler there is no alias and to select wider load/store
    // instructions.
    T data3 = argData[INDEX_THREE];
    T data2 = argData[INDEX_TWO];
    T data1 = argData[1];
    T data0 = argData[0];
    data_[INDEX_THREE] *= data3;
    data_[INDEX_TWO] *= data2;
    data_[1] *= data1;
    data_[0] *= data0;
}

template<typename T>
void Vector4<T>::Negate()
{
    data_[INDEX_THREE] = -data_[INDEX_THREE];
    data_[INDEX_TWO] = -data_[INDEX_TWO];
    data_[1] = -data_[1];
    data_[0] = -data_[0];
}

template<typename T>
void Vector4<T>::Absolute()
{
    data_[INDEX_THREE] = abs(data_[INDEX_THREE]);
    data_[INDEX_TWO] = abs(data_[INDEX_TWO]);
    data_[1] = abs(data_[1]);
    data_[0] = abs(data_[0]);
}

template<typename T>
bool Vector4<T>::IsInfinite() const
{
    return std::isinf(data_[0]) || std::isinf(data_[1]) ||
        std::isinf(data_[INDEX_TWO]) || std::isinf(data_[INDEX_THREE]);
}

template<typename T>
bool Vector4<T>::IsNaN() const
{
    return std::isnan(data_[0]) || std::isnan(data_[1]) ||
        std::isnan(data_[INDEX_TWO]) || std::isnan(data_[INDEX_THREE]);
}

template<typename T>
bool Vector4<T>::IsValid() const
{
    return !IsInfinite() && !IsNaN();
}
} // namespace Rosen
} // namespace OHOS
#endif // RENDER_SERVICE_CLIENT_CORE_COMMON_RS_VECTOR4_H
