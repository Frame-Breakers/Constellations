#include "Point3d.h"

point3d::point3d() {
    x = 0;
    y = 0;
    z = 0;
}
point3d::point3d(float X, float Y, float Z) : x(X), y(Y), z(Z) {}
point3d::point3d(float A) : x(A), y(A), z(A) {}

float point3d::magnitude() const {

    float xx = isnan(x) ? 0.0f : x;
    float yy = isnan(y) ? 0.0f : y;
    float zz = isnan(z) ? 0.0f : z;

    return sqrt(xx * xx + yy * yy + zz * zz);
}

float point3d::volume() const {
    return x * y * z;
}

point3d point3d::normalized() const {
    float mag = magnitude();
    if (mag <= std::numeric_limits<float>::epsilon()) {
        return point3d(0.0f, 1.0f, 0.0f);
    }
    return *this / mag;
}

point3d point3d::cross(const point3d& other) const
{
    return point3d{
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    };
}

point3d point3d::lerp(const point3d& other, float a) const
{
    return *this + (other - *this) * a;
}

point3d point3d::floor3() const
{
    return { floor(x), floor(y), floor(z) };
}

point3d point3d::fract() const
{
    return { x - floor(x), y - floor(y), z - floor(z) };
}

float point3d::dot(const point3d& other) const {
    return x * other.x + y * other.y + z * other.z;
}

DirectX::XMVECTOR point3d::toXMVector() const {
    return DirectX::XMVectorSet(x, y, z, 0);
}

DirectX::XMFLOAT4 point3d::toXMFloat4() const {
    return DirectX::XMFLOAT4(x, y, z, 0);
}


bool point3d::operator==(const point3d& other) const {
    return fabs(x - other.x) < 0.001f &&
        fabs(y - other.y) < 0.001f &&
        fabs(z - other.z) < 0.001f;
}

bool point3d::operator!=(const point3d& other) const {
    return !(*this == other);
}

point3d& point3d::operator=(const point3d& other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}


point3d point3d::operator+(const point3d& other) const {
    return { x + other.x, y + other.y, z + other.z };
}
point3d point3d::operator+(float scalar) const {
    return { x + scalar, y + scalar, z + scalar };
}
point3d& point3d::operator+=(const point3d& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}
point3d& point3d::operator+=(float scalar) {
    x += scalar;
    y += scalar;
    z += scalar;
    return *this;
}


point3d point3d::operator-(const point3d& other) const {
    return { x - other.x, y - other.y, z - other.z };
}
point3d point3d::operator-(float scalar) const {
    return { x - scalar, y - scalar, z - scalar };
}
point3d point3d::operator-() const {
    return { -x, -y, -z };
}
point3d& point3d::operator-=(const point3d& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}
point3d& point3d::operator-=(float scalar) {
    x -= scalar;
    y -= scalar;
    z -= scalar;
    return *this;
}


point3d point3d::operator*(float scalar) const {
    return { x * scalar, y * scalar, z * scalar };
}
point3d point3d::operator*(const point3d& other) const {
    return { x * other.x, y * other.y, z * other.z };
}
point3d& point3d::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}
point3d& point3d::operator*=(const point3d& other) {
    x *= other.x;
    y *= other.y;
    z *= other.z;
    return *this;
}


point3d point3d::operator/(const point3d& other) const {
    return { x / other.x, y / other.y, z / other.z };
}
point3d point3d::operator/(float scalar) const {
    return { x / scalar, y / scalar, z / scalar };
}
point3d& point3d::operator/=(const point3d& other) {
    x /= other.x;
    y /= other.y;
    z /= other.z;
    return *this;
}
point3d& point3d::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}