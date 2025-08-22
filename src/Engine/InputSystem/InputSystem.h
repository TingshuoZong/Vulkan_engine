#pragma once

#include "window.h"
#include "Core/Camera.h"

/// @brief This namespace contains all of the functions that process input
namespace InputSystem {
	extern bool mouse_captured;
	/// @brief Processes keyboard and mouse inputs
	/// @param window Takes a pointer to @c GLFWwindow which you want to poll inputs for
	/// @param camera Takes in the @ref Camera so it can modify it's positon
	/// @param delta_time The time elapsed since the start of the last frame
	void process_input(GLFWwindow* window, Camera& camera, float delta_time);
	/// @brief Called automatically by @ref InputSystem::process_input you don't have to call it manually
	void process_mouse(GLFWwindow* window, Camera& camera);
};