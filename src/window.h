#pragma once

#include <daxa/daxa.hpp>
using namespace daxa::types; // For types like `u32`

#include <GLFW/glfw3.h>

#if defined(_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
    #define GLFW_NATIVE_INCLUDE_NONE
    using HWND = void*;
#elif defined(__linux__)
    #define GLFW_EXPOSE_NATIVE_X11
    #define GLFW_EXPOSE_NATIVE_WAYLAND
#endif

#include <GLFW/glfw3native.h> // Platform-specific GLFW functions

struct AppWindow {
    GLFWwindow* glfw_window_ptr; // Pointer to the GLFW window object
    u32 width, height;           // Dimensions of the window
    bool minimized = false;      // Tracks if the window is minimized
    bool swapchain_out_of_date = false; // Tracks if the swapchain needs updating

    explicit AppWindow(char const* window_name, u32 sx = 1600, u32 sy = 900)
        : width{ sx }, height{ sy } {
        glfwInit(); // Initialize GLFW

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // No graphics API
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);    // Make the window resizable

        // Create the GLFW window
        glfw_window_ptr = glfwCreateWindow(
            static_cast<i32>(width), static_cast<i32>(height),
            window_name, nullptr, nullptr
        );

        // Associate the AppWindow object with the GLFW window
        glfwSetWindowUserPointer(glfw_window_ptr, this);

        // Set a callback to handle resizing events
        glfwSetWindowSizeCallback(glfw_window_ptr, [](GLFWwindow* window, int size_x, int size_y) {
            auto* win = static_cast<AppWindow*>(glfwGetWindowUserPointer(window));
            win->width = static_cast<u32>(size_x);
            win->height = static_cast<u32>(size_y);
            win->swapchain_out_of_date = true;
            });
    }

    ~AppWindow() {
        glfwDestroyWindow(glfw_window_ptr); // Destroy the GLFW window
        glfwTerminate();                    // Terminate GLFW
    }

    auto get_native_handle() const -> daxa::NativeWindowHandle {
#if defined(_WIN32)
            return glfwGetWin32Window(glfw_window_ptr);
#elif defined(__linux__)
            switch (get_native_platform()) {
            case daxa::NativeWindowPlatform::WAYLAND_API:
                return reinterpret_cast<daxa::NativeWindowHandle>(glfwGetWaylandWindow(glfw_window_ptr));
            case daxa::NativeWindowPlatform::XLIB_API:
            default:
                return reinterpret_cast<daxa::NativeWindowHandle>(glfwGetX11Window(glfw_window_ptr));
            }
#endif
    }

    static auto get_native_platform() -> daxa::NativeWindowPlatform {
        switch (glfwGetPlatform()) {
        case GLFW_PLATFORM_WIN32: return daxa::NativeWindowPlatform::WIN32_API;
        case GLFW_PLATFORM_X11: return daxa::NativeWindowPlatform::XLIB_API;
        case GLFW_PLATFORM_WAYLAND: return daxa::NativeWindowPlatform::WAYLAND_API;
        default: return daxa::NativeWindowPlatform::UNKNOWN;
        }
    }

    inline void set_mouse_capture(bool should_capture) const {
        glfwSetCursorPos(glfw_window_ptr, static_cast<f64>(width / 2.), static_cast<f64>(height / 2.));
        glfwSetInputMode(glfw_window_ptr, GLFW_CURSOR, should_capture ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        glfwSetInputMode(glfw_window_ptr, GLFW_RAW_MOUSE_MOTION, should_capture);
    }

    inline bool should_close() const {
        return glfwWindowShouldClose(glfw_window_ptr);
    }

    inline void update() const {
        glfwPollEvents();
        glfwSwapBuffers(glfw_window_ptr);
    }

    inline GLFWwindow* get_glfw_window() const {
        return glfw_window_ptr;
    }
};