#include "InputSystem.h"

namespace InputSystem {
    void process_input(GLFWwindow* window, Camera& camera, float delta_time) {
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
    }

    void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
        static bool first_mouse = true;
        static float last_x = 800.0f / 2;
        static float last_y = 600.0f / 2;

        static Camera* cam_ptr = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
        if (!cam_ptr) return;
        Camera& camera = *cam_ptr;

        if (first_mouse) {
            last_x = xpos;
            last_y = ypos;
            first_mouse = false;
        }

        float xoffset = xpos - last_x;
        float yoffset = last_y - ypos;  // reversed since y-coordinates go from bottom to top
        last_x = xpos;
        last_y = ypos;

        xoffset *= camera.mouse_sensitivity;
        yoffset *= camera.mouse_sensitivity;

        camera.yaw += xoffset;
        camera.pitch += yoffset;

        if (camera.pitch > 89.0f)
            camera.pitch = 89.0f;
        if (camera.pitch < -89.0f)
            camera.pitch = -89.0f;

        camera.update_vectors();
    }
}