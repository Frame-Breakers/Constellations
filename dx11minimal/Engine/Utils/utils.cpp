#include "utils.h"

#include <random>
#include <cctype>
#include <locale>
#include <codecvt>

using namespace std;
using namespace DirectX;

//#undef min
//#undef max

namespace {
    point3d GetRatioScale(ScreenAspectRatio ratio, const point3d& frameScale)
    {
        switch (ratio)
        {
        case ScreenAspectRatio::XY:
            return point3d(frameScale.x, frameScale.y, 0.0f);
        case ScreenAspectRatio::YX:
            return point3d(frameScale.y * ConstBuf::frame.aspect.x, frameScale.x * ConstBuf::frame.aspect.y, 0.0f);
        case ScreenAspectRatio::XX:
            return point3d(frameScale.x, frameScale.x * ConstBuf::frame.aspect.y, 0.0f);
        case ScreenAspectRatio::YY:
            return point3d(frameScale.y * ConstBuf::frame.aspect.x, frameScale.y, 0.0f);
        default:
            return point3d(frameScale.x, frameScale.y, 0.0f);
        }
    }

    point3d RotateUiVector(const point3d& vector, float rotation)
    {
        point3d aspectSpaceVector = vector;
        aspectSpaceVector.x *= ConstBuf::frame.aspect.y;
        aspectSpaceVector = rotatePoint(aspectSpaceVector, rotation);
        aspectSpaceVector.x *= ConstBuf::frame.aspect.x;

        return aspectSpaceVector;
    }

    point3d GetTransform2DFrameCenter(const Transform2D& transform)
    {
        return transform.position + RotateUiVector(-transform.anchorPoint * transform.scale, transform.rotation);
    }
}

float clamp(float x, float a, float b) {
    return fmax(fmin(x, b), a);
}


float fract(float a) {
    return a - floor(a);
}


//float lerp(float x1, float x2, float a) {
//    return x1 * (1 - a) + x2 * a;
//}


float degreesToRadians(float degrees) {
    return DirectX::XMConvertToRadians(degrees);
}


float radiansToDegrees(float radians) {
    return DirectX::XMConvertToDegrees(radians);
}


point3d quaternionToEuler(float x, float y, float z, float w) {
    float magintude = sqrt(x * x + y * y + z * z);
    x /= magintude;
    y /= magintude;
    z /= magintude;

    float pitch = atan2(2 * (w * x + y * z), 1 - 2 * (pow(x, 2) + pow(y, 2)));
    float yaw = asin(2 * (w * y - z * x));
    float roll = atan2(2 * (w * z + x * y), 1 - 2 * (pow(y, 2) + pow(z, 2)));

    return point3d(pitch, yaw, roll);
}


DirectX::XMVECTOR eulerToQuanternion(float x, float y, float z) {
    DirectX::XMVECTOR qRotation = DirectX::XMQuaternionRotationRollPitchYaw(x, y, z);
    qRotation = DirectX::XMQuaternionNormalize(qRotation);

    return qRotation;
}


int getRandom(int min, int max) {
    if (max > min)
    {
        return rand() % (max - min) + min;
    }
    else
    {
        return rand() % min;
    }
}


float getRandomFloat(float min, float max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());

    return std::uniform_real_distribution<float>(min, max)(gen);
}


point3d getRandomDirection() {
    float r1 = getRandomFloat(0.0f, 1.0f);
    float r2 = getRandomFloat(0.0f, 1.0f);

    float z = 2.0f * r1 - 1.0f;
    float theta = 2.0f * XM_PI * r2;

    float r = sqrtf(1.0f - z * z);
    float x = r * cosf(theta);
    float y = r * sinf(theta);

    return point3d(x, y, z);
}


point3d rotatePoint(point3d pos, float angle)
{
    float radius = pos.magnitude();
    float angle0 = atan2(pos.y, pos.x);
    angle0 += angle;

    return point3d(cos(angle0), sin(angle0), 0) * radius;
}


Transform GetWorldTransform(Entity* entity) {
    Transform worldTransform = Transform();

    Transform* originTransform = entity->GetComponent<Transform>();
    if (originTransform != nullptr) {

        vector<Transform*> transforms = { originTransform };

        Entity* ancestor = entity->GetParent();
        while (ancestor != nullptr) {

            Transform* parentTransform = ancestor->GetComponent<Transform>();
            if (parentTransform != nullptr) {
                transforms.push_back(parentTransform);
                ancestor = ancestor->GetParent();
            }
            else {
                break;
            }
        }

        for (int i = transforms.size() - 1; i >= 0; i--) {
            worldTransform += *transforms[i];
        }
    }

    return worldTransform;
}


Transform2D GetWorldTransform2D(Entity* entity) {
    Transform2D worldTransform2D = Transform2D();

    Transform2D* originTransform2D = entity->GetComponent<Transform2D>();
    if (originTransform2D != nullptr) {

        vector<Transform2D*> transforms2d = { originTransform2D };

        Entity* ancestor = entity->GetParent();
        while (ancestor != nullptr) {

            Transform2D* parentTransform2D = ancestor->GetComponent<Transform2D>();
            if (parentTransform2D != nullptr) {
                transforms2d.push_back(parentTransform2D);
                ancestor = ancestor->GetParent();
            }
            else {
                break;
            }
        }

        for (int i = transforms2d.size() - 1; i >= 0; i--) {
            Transform2D* localTransform = transforms2d[i];
            const point3d parentFrameCenter = GetTransform2DFrameCenter(worldTransform2D);
            const point3d localPosition = (localTransform->parentAnchor + localTransform->position) * worldTransform2D.scale;

            Transform2D nextWorldTransform2D = Transform2D();
            nextWorldTransform2D.position = parentFrameCenter + RotateUiVector(localPosition, worldTransform2D.rotation);
            nextWorldTransform2D.anchorPoint = localTransform->anchorPoint;
            nextWorldTransform2D.parentAnchor = localTransform->parentAnchor;
            nextWorldTransform2D.scale = localTransform->scale * GetRatioScale(localTransform->ratio, worldTransform2D.scale);
            nextWorldTransform2D.rotation = worldTransform2D.rotation + localTransform->rotation;
            nextWorldTransform2D.ratio = ScreenAspectRatio::XY;

            worldTransform2D = nextWorldTransform2D;
        }
    }

    return worldTransform2D;
}


bool IsEntityValid(Entity* entity) {
    return entity && entity->IsActive();
}


vector<string> split(string s, string temp) {
    vector<string> tokens;
    size_t pos = 0;
    string token;

    while ((pos = s.find(temp)) != string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + temp.length());
    }
    tokens.push_back(s);

    return tokens;
}


DirectX::XMMATRIX GetMatrixBetweenLookVector(Transform& transform, point3d direction) {
    point3d currentLookVector = transform.GetLookVector();
    point3d rotationAxis = currentLookVector.cross(direction).normalized();
    DirectX::XMVECTOR rotationAxisVector = DirectX::XMVectorSet(rotationAxis.x, rotationAxis.y, rotationAxis.z, 0.0f);
    float angleBetweenVectors = acosf(currentLookVector.dot(direction));
    DirectX::XMVECTOR quaternionRotation = DirectX::XMQuaternionRotationAxis(rotationAxisVector, angleBetweenVectors);
    DirectX::XMMATRIX matrixRotation = DirectX::XMMatrixRotationQuaternion(quaternionRotation);

    return matrixRotation * transform.mRotation;
}

DirectX::XMMATRIX GetMatrixFromDirection(const point3d& direction, point3d upVector) {
    float length = direction.magnitude();
    if (length < 0.001f) {
        return DirectX::XMMatrixIdentity();
    }

    point3d lookDirection = direction.normalized();

    point3d right = upVector.cross(lookDirection).normalized();
    if (right.magnitude() < 0.001f) {
        if (upVector != point3d(0.0f, 1.0f, 0.0f)) {
            upVector = point3d(0.0f, 1.0f, 0.0f);
            right = upVector.cross(lookDirection).normalized();

            if (right.magnitude() < 0.001f) {
                upVector = point3d(0.0f, 0.0f, 1.0f);
                right = upVector.cross(lookDirection).normalized();

                if (right.magnitude() < 0.001f) {
                    return DirectX::XMMatrixIdentity();
                }
            }
        }
        else {
            upVector = point3d(0.0f, 0.0f, 1.0f);
            right = upVector.cross(lookDirection).normalized();

            if (right.magnitude() < 0.001f) {
                return DirectX::XMMatrixIdentity();
            }
        }
    }

    right = right.normalized();
    point3d up = lookDirection.cross(right).normalized();

    DirectX::XMMATRIX matrixRotation = DirectX::XMMatrixSet(
        right.x, right.y, right.z, 0.0f,
        up.x, up.y, up.z, 0.0f,
        lookDirection.x, lookDirection.y, lookDirection.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    return matrixRotation;
}

DirectX::XMMATRIX TransformMatrixToLookVector(const DirectX::XMMATRIX& originMatrix, const point3d& direction) {
    point3d currentLookVector = point3d(originMatrix.r[2].m128_f32[0], originMatrix.r[2].m128_f32[1], originMatrix.r[2].m128_f32[2]).normalized();
    point3d rotationAxis = currentLookVector.cross(direction).normalized();
    DirectX::XMVECTOR rotationAxisVector = DirectX::XMVectorSet(rotationAxis.x, rotationAxis.y, rotationAxis.z, 0.0f);

    float angleBetweenVectors = acosf(currentLookVector.dot(direction));
    DirectX::XMVECTOR quaternionRotation = DirectX::XMQuaternionRotationAxis(rotationAxisVector, angleBetweenVectors);
    DirectX::XMMATRIX matrixRotation = DirectX::XMMatrixRotationQuaternion(quaternionRotation);

    return matrixRotation;
}

DirectX::XMMATRIX TransformMatrixToUpVector(const DirectX::XMMATRIX& originMatrix, const point3d& targetUpVector) {
    point3d currentRight = point3d(
        originMatrix.r[0].m128_f32[0],
        originMatrix.r[0].m128_f32[1],
        originMatrix.r[0].m128_f32[2]
    ).normalized();

    point3d currentUp = point3d(
        originMatrix.r[1].m128_f32[0],
        originMatrix.r[1].m128_f32[1],
        originMatrix.r[1].m128_f32[2]
    ).normalized();

    point3d currentLook = point3d(
        originMatrix.r[2].m128_f32[0],
        originMatrix.r[2].m128_f32[1],
        originMatrix.r[2].m128_f32[2]
    ).normalized();

    point3d targetUp = targetUpVector.normalized();

    float dotWithLook = targetUp.dot(currentLook);
    if (fabsf(dotWithLook) > 0.999f) {
        point3d alternativeUp = point3d(0.0f, 1.0f, 0.0f);
        if (fabsf(alternativeUp.dot(currentLook)) > 0.999f) {
            alternativeUp = point3d(1.0f, 0.0f, 0.0f);
        }

        point3d newRight = alternativeUp.cross(currentLook).normalized();
        point3d newUp = currentLook.cross(newRight).normalized();

        return DirectX::XMMatrixSet(
            newRight.x, newRight.y, newRight.z, originMatrix.r[0].m128_f32[3],
            newUp.x, newUp.y, newUp.z, originMatrix.r[1].m128_f32[3],
            currentLook.x, currentLook.y, currentLook.z, originMatrix.r[2].m128_f32[3],
            originMatrix.r[3].m128_f32[0], originMatrix.r[3].m128_f32[1],
            originMatrix.r[3].m128_f32[2], originMatrix.r[3].m128_f32[3]
        );
    }

    point3d newRight = targetUp.cross(currentLook).normalized();

    if (newRight.magnitude() < 0.001f) {
        newRight = currentRight;
    }
    else {
        newRight = newRight.normalized();
    }
    point3d newUp = currentLook.cross(newRight).normalized();

    DirectX::XMMATRIX resultMatrix = DirectX::XMMatrixSet(
        newRight.x, newRight.y, newRight.z, originMatrix.r[0].m128_f32[3],
        newUp.x, newUp.y, newUp.z, originMatrix.r[1].m128_f32[3],
        currentLook.x, currentLook.y, currentLook.z, originMatrix.r[2].m128_f32[3],
        originMatrix.r[3].m128_f32[0], originMatrix.r[3].m128_f32[1],
        originMatrix.r[3].m128_f32[2], originMatrix.r[3].m128_f32[3]
    );

    return resultMatrix;
}


float GetSignedAngleBetweenLookVectors(const XMMATRIX& matrix1, const XMMATRIX& matrix2, bool inDegrees)
{
    XMVECTOR look1 = XMVector3Normalize(matrix1.r[2]);
    XMVECTOR look2 = XMVector3Normalize(matrix2.r[2]);

    float dot = XMVectorGetX(XMVector3Dot(look1, look2));
    dot = max(-1.0f, min(1.0f, dot));
    float angleRad = acosf(dot);

    XMVECTOR cross = XMVector3Cross(look1, look2);
    float sign = XMVectorGetX(XMVector3Dot(cross, XMVectorSet(0, 1, 0, 0)));

    angleRad *= (sign < 0) ? -1.0f : 1.0f;

    return inDegrees ? XMConvertToDegrees(angleRad) : angleRad;
}


XMMATRIX GetRelativeMatrix(const XMMATRIX& matrixA, const XMMATRIX& matrixB)
{
    XMMATRIX invA = XMMatrixInverse(nullptr, matrixA);
    XMMATRIX delta = XMMatrixMultiply(matrixB, invA);

    return delta;
}


XMMATRIX GetRelativeRotationMatrix(const XMMATRIX& matrixA, const XMMATRIX& matrixB)
{
    XMMATRIX rotationA = XMMatrixIdentity();
    rotationA.r[0] = XMVector3Normalize(matrixA.r[0]);  // Right
    rotationA.r[1] = XMVector3Normalize(matrixA.r[1]);  // Up
    rotationA.r[2] = XMVector3Normalize(matrixA.r[2]);  // Look
    rotationA.r[3] = XMVectorSet(0, 0, 0, 1);

    XMMATRIX rotationB = XMMatrixIdentity();
    rotationB.r[0] = XMVector3Normalize(matrixB.r[0]);
    rotationB.r[1] = XMVector3Normalize(matrixB.r[1]);
    rotationB.r[2] = XMVector3Normalize(matrixB.r[2]);
    rotationB.r[3] = XMVectorSet(0, 0, 0, 1);

    XMMATRIX invA = XMMatrixInverse(nullptr, rotationA);
    XMMATRIX delta = XMMatrixMultiply(rotationB, invA);

    return delta;
}


float GetSignedAngleBetweenVectors(const point3d& vector1, const point3d& vector2, bool inDegrees)
{
    float dot = vector1.dot(vector2);
    dot = max(-1.0f, min(1.0f, dot));
    float angleRad = acosf(dot);

    point3d cross = vector1.cross(vector2);
    float sign = cross.dot(point3d(0, 1, 0));

    angleRad *= (sign < 0) ? -1.0f : 1.0f;

    return inDegrees ? XMConvertToDegrees(angleRad) : angleRad;
}


float hueToRgb(float p, float q, float t) {
    if (t < 0.0f) t += 1.0f;
    if (t > 1.0f) t -= 1.0f;

    if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
    if (t < 1.0f / 2.0f) return q;
    if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
    return p;
}


point3d hslToRgb(const point3d& hsl) {
    float h = hsl.x;
    float s = hsl.y;
    float l = hsl.z;

    float r, g, b;

    if (s == 0.0f) {
        r = g = b = l;
    }
    else {
        float q = (l < 0.5f) ? l * (1.0f + s) : l + s - l * s;
        float p = 2.0f * l - q;

        r = hueToRgb(p, q, h + 1.0f / 3.0f);
        g = hueToRgb(p, q, h);
        b = hueToRgb(p, q, h - 1.0f / 3.0f);
    }

    return point3d(r, g, b);
}


point3d rgbToHsl(const point3d& rgb) {
    float r = rgb.x;
    float g = rgb.y;
    float b = rgb.z;

    float maxVal = max(r, max(g, b));
    float minVal = min(r, max(g, b));
    float h = 0.0f, s = 0.0f, l = (maxVal + minVal) / 2.0f;

    if (maxVal != minVal) {
        float diff = maxVal - minVal;
        s = (l > 0.5f) ? diff / (2.0f - maxVal - minVal) : diff / (maxVal + minVal);

        if (maxVal == r) {
            h = (g - b) / diff + ((g < b) ? 6.0f : 0.0f);
        }
        else if (maxVal == g) {
            h = (b - r) / diff + 2.0f;
        }
        else {
            h = (r - g) / diff + 4.0f;
        }

        h /= 6.0f;
    }

    return point3d(h, s, l);
}


point3d rotateInPlane(const point3d& a, const point3d& b, float theta) {
    point3d u1 = a.normalized();
    point3d u2 = b.normalized();

    return (u1 * cos(theta) + u2 * sin(theta));
}


DirectX::XMMATRIX LerpMatrix(const DirectX::XMMATRIX& from, const DirectX::XMMATRIX& to, float t)
{
    DirectX::XMVECTOR fromQuat = XMQuaternionRotationMatrix(from);
    DirectX::XMVECTOR toQuat = XMQuaternionRotationMatrix(to);

    DirectX::XMVECTOR resultQuat = XMQuaternionSlerp(fromQuat, toQuat, t);

    return XMMatrixRotationQuaternion(resultQuat);
}


point3d GetLookVectorFromMatrix(const DirectX::XMMATRIX& matrix) {
    return point3d(matrix.r[2].m128_f32[0], matrix.r[2].m128_f32[1], matrix.r[2].m128_f32[2]).normalized();
}

point3d GetUpVectorFromMatrix(const DirectX::XMMATRIX& matrix) {
    return point3d(matrix.r[1].m128_f32[0], matrix.r[1].m128_f32[1], matrix.r[1].m128_f32[2]).normalized();
}

point3d GetRightVectorFromMatrix(const DirectX::XMMATRIX& matrix) {
    return point3d(matrix.r[0].m128_f32[0], matrix.r[0].m128_f32[1], matrix.r[0].m128_f32[2]).normalized();
}


bool isFirstStringHigher(const string& str1, const string& str2) {
    size_t i = 0;
    while (i < str1.length() && i < str2.length()) {
        char c1 = tolower(static_cast<unsigned char>(str1[i]));
        char c2 = tolower(static_cast<unsigned char>(str2[i]));

        if (c1 != c2) {
            return c1 < c2;
        }
        i++;
    }

    // If all characters match, the shorter string is considered "higher"
    return str1.length() < str2.length();
}

wstring string_to_wstring(const string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.length(), &wstr[0], size_needed);
    return wstr;
}