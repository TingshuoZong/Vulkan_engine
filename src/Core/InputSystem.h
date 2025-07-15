#pragma once

#include "window.h"
#include "Camera.h"


namespace InputSystem {
	void process_input(GLFWwindow* window, Camera& camera, float delta_time);
	void mouse_callback(GLFWwindow* window, double xpos, double ypos);
};