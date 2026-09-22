#ifndef _POINT3D_H
#define _POINT3D_H

#include <sstream>
#include <corecrt_math.h>
#include "DirectXMath.h"

#include <nlohmann/json.hpp>

struct point3d {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    point3d();
    point3d(float X, float Y, float Z);
    point3d(float A);

    float magnitude() const;
    float volume() const;
    point3d normalized() const;
    point3d cross(const point3d& other) const;
    point3d lerp(const point3d& other, float a) const;
    point3d floor3() const;
    point3d fract() const;
    float dot(const point3d& other) const;
    DirectX::XMVECTOR toXMVector() const;
    DirectX::XMFLOAT4 toXMFloat4() const;

    bool operator==(const point3d& other) const;
    bool operator!=(const point3d& other) const;
    point3d& operator=(const point3d& other);

    point3d operator+(const point3d& other) const;
    point3d operator+(float scalar) const;
    point3d& operator+=(const point3d& other);
    point3d& operator+=(float scalar);

    point3d operator-(const point3d& other) const;
    point3d operator-(float scalar) const;
    point3d operator-() const;
    point3d& operator-=(const point3d& other);
    point3d& operator-=(float scalar);

    point3d operator*(float scalar) const;
    point3d operator*(const point3d& other) const;
    point3d& operator*=(float scalar);
    point3d& operator*=(const point3d& other);

    point3d operator/(const point3d& other) const;
    point3d operator/(float scalar) const;
    point3d& operator/=(const point3d& other);
    point3d& operator/=(float scalar);
};

inline point3d operator*(float scalar, const point3d& p) {
    return p * scalar;
}

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(point3d,
    x,
    y,
    z)

#endif