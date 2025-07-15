#pragma once
#include "shared.inl"  // For UniformBufferObject or MyPushConstant

struct Drawable {
    daxa::BufferId vertex_buffer;
    daxa::BufferId index_buffer;
    u32 index_count;
    glm::mat4 model_matrix;
};