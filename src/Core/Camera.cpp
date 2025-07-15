#include "Camera.h"

void Camera::update_vectors() {
    glm::vec3 f;

    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    front = glm::normalize(f);
    right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));

    up = glm::normalize(glm::cross(right, front));
}

glm::mat4 Camera::get_projection(float aspect_ratio) const {
    glm::mat4 proj = glm::perspective(glm::radians(this->fov), aspect_ratio, 0.1f, 100.0f);
    proj[1][1] *= -1.0f; // Vulkan correction
    return proj;
}