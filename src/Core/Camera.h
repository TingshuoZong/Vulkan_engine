#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
    glm::vec3 position = { 0.0f, 0.0f, 2.0f };
    float yaw = -90.0f;  // Looking towards -Z
    float pitch = 0.0f;

    float fov = 60.0f;

    float movement_speed = 2.5f;
    float mouse_sensitivity = 0.1f;

    glm::vec3 front = { 0.0f, 0.0f, -1.0f };
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 right = { 1.0f, 0.0f, 0.0f };

    /// @brief Takes the mouse pitch and yaw and turns it into the x,y,z components of the camera's euler rotation and normalizes them
    void update_vectors();

    /// @brief Gets the view matrix
    glm::mat4 get_view_matrix() { return glm::lookAt(position, position + front, up); }
    /// @brief Gets the view matrix but without the translation
    glm::mat4 get_view_rot_matrix() { return glm::lookAt(glm::vec3(0.0f), front, up); }
    /// @brief Gets the projection matrix
    glm::mat4 get_projection(float aspect_ratio) const;
};