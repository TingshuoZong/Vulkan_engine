#pragma once

#include <daxa/daxa.inl>
#include <glm/glm.hpp>

/// @file This file contains functions to cast @c glm maths datatypes to @c daxa (just wrapped Vulkan) ones as they have the same memory layout, this allows the shared headers to use the daxa datatypes and glm to be used for the engine

/// @brief References and @c reinterpret_cast a @c glm::mat4 to a @c daxa_f32mat4x4 before dereferencign it
/// @param m A reference to the @c glm::mat4 to cast
/// @return A @c daxa_f32mat4x4 by value
inline daxa_f32mat4x4 to_daxa(const glm::mat4& m) {
	return *reinterpret_cast<const daxa_f32mat4x4*>(&m);
}