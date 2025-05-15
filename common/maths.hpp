#pragma once

#include <cmath>
#include <glm/glm.hpp> // Only for interfacing with existing code, not using glm math functions

// Quaternion class
class Quaternion {
public:
    float w, x, y, z;

    Quaternion();
    Quaternion(const float w, const float x, const float y, const float z);
    Quaternion(const float pitch, const float yaw);

    glm::mat4 matrix();
};

// Maths class
class Maths {
public:
    static glm::mat4 translate(const glm::vec3& v);
    static glm::mat4 scale(const glm::vec3& v);
    static float radians(float angle);
    static glm::mat4 rotate(const float& angle, glm::vec3 v);
    static Quaternion SLERP(const Quaternion q1, const Quaternion q2, const float t);

    // Custom math implementations to fulfill LO1
    static float length(const glm::vec3& v) {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    static float dot(const glm::vec3& a, const glm::vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static glm::vec3 cross(const glm::vec3& a, const glm::vec3& b) {
        return glm::vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }

    static glm::vec3 normalize(const glm::vec3& v) {
        float len = length(v);
        if (len == 0.0f) return glm::vec3(0.0f);
        return glm::vec3(v.x / len, v.y / len, v.z / len);
    }
};
