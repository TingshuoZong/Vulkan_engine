#include "InputSystem.h"

namespace InputSystem {
    bool mouse_captured = true;
    bool escape_repeat_acctuations = false;     // Stops the mouse from capturing and uncapturing every frame is escape is held down for more than a frame

    void process_input(GLFWwindow* window, Camera& camera, float delta_time) {
        auto* app = static_cast<GLFW_Window::AppWindow*>(glfwGetWindowUserPointer(window));
        if (!app || !app->camera_ptr) return;

        process_mouse(window, camera);

        float velocity = camera.movement_speed * delta_time;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.position += camera.front * velocity;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.position -= camera.right * velocity;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.position -= camera.front * velocity;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.position += camera.right * velocity;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            camera.position += camera.up * velocity;
        if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            camera.position -= camera.up * velocity;
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            if (!escape_repeat_acctuations) {
                if (mouse_captured) {
                    app->set_mouse_capture(false);
                    mouse_captured = false;
                }
                else {
                    app->set_mouse_capture(true);
                    mouse_captured = true;
                }
                escape_repeat_acctuations = true;
            }
        } else { escape_repeat_acctuations = false; }
    }

    void process_mouse(GLFWwindow* window, Camera& camera) {
        static bool first_mouse = true;
        static float last_x = 800.0f / 2;
        static float last_y = 600.0f / 2;

        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        if (first_mouse) {
            last_x = xpos;
            last_y = ypos;
            first_mouse = false;
        }

        float xoffset = xpos - last_x;
        float yoffset = last_y - ypos;  // Reversed since y-coordinates go bottom-to-top
        last_x = xpos;
        last_y = ypos;

        xoffset *= camera.mouse_sensitivity;
        yoffset *= camera.mouse_sensitivity;

        camera.yaw += xoffset;
        camera.pitch += yoffset;

        if (camera.pitch > 89.0f) camera.pitch = 89.0f;
        if (camera.pitch < -89.0f) camera.pitch = -89.0f;

        camera.update_vectors();

        if (mouse_captured) {
            glfwSetCursorPos(window, 400.0, 300.0);
            last_x = 400.0;
            last_y = 300.0;
        }
    }
}