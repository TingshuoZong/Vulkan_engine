#pragma once

#include <daxa/daxa.inl>
#include <glm/glm.hpp>

inline daxa_f32mat4x4 to_daxa(const glm::mat4& m) {
	return *reinterpret_cast<const daxa_f32mat4x4*>(&m);
}