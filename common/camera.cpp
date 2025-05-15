#include <common/camera.hpp>



Camera::Camera(const glm::vec3 Eye, const glm::vec3 Target)
{
    eye = Eye;
    target = Target;

}

void Camera::calculateMatrices()
{
    // Calculate camera vectors
    calculateCameraVectors();
    // Calculate the view matrix
    view = glm::lookAt(eye, eye + front, worldUp);

    // Calculate the projection matrix
    projection = glm::perspective(fov, aspect, near, far);

}
void Camera::calculateCameraVectors()
{
    front = glm::vec3(cos(yaw) * cos(pitch), sin(pitch), sin(yaw) * cos(pitch));
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::cross(right, front);
}
void Camera::quaternionCamera() {
    // Step 1: Compute target orientation from pitch/yaw
    Quaternion targetOrientation(-pitch, yaw);

    // Step 2: Smoothly interpolate toward it (SLERP)
    orientation = Maths::SLERP(orientation, targetOrientation, 0.2f);

    // Step 3: Construct the rotation matrix from orientation
    glm::mat4 rot = orientation.matrix();  // Assuming it returns a 4x4 mat

    // Step 4: Build view matrix from orientation and position
    glm::mat4 translate = Maths::translate(-eye);
    view = rot * translate;

    // Step 5: Update direction vectors from rotated basis
    right = glm::vec3(view[0][0], view[1][0], view[2][0]);
    up = glm::vec3(view[0][1], view[1][1], view[2][1]);
    front = -glm::vec3(view[0][2], view[1][2], view[2][2]);
}
