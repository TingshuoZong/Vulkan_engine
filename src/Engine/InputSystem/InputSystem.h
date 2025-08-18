#pragma once

#include "window.h"
#include "Core/Camera.h"


namespace InputSystem {
	extern bool mouse_captured;

	void process_input(GLFWwindow* window, Camera& camera, float delta_time);
	void process_mouse(GLFWwindow* window, Camera& camera);
};