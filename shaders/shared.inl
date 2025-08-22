#pragma once

#include <daxa/daxa.inl>

#ifdef __cplusplus

    // CPU side only definitions
    #include <glm/glm.hpp>
    #include <glm/gtc/matrix_transform.hpp>
    #include <Tools/maths_type_casts.h>

namespace meshRenderer {
#else
// GPU side only definitions
#include <daxa/daxa.glsl>
#endif

// Code that can be 100% shared between CPU and GPU

struct UniformBufferObject {
    daxa_f32mat4x4 view;
    daxa_f32mat4x4 proj;
};

struct PerInstanceData {
    daxa_f32mat4x4 model_matrix;
    daxa_ImageViewId texture;
    daxa_SamplerId tex_sampler;
    daxa_u32 _pad0;
    daxa_u32 _pad1;
};

struct Vertex {
    daxa_f32vec3 position;
    daxa_f32vec2 uv;
};

DAXA_DECL_BUFFER_PTR(Vertex)
DAXA_DECL_BUFFER_PTR(UniformBufferObject)
DAXA_DECL_BUFFER_PTR(PerInstanceData)

struct PushConstant {
    daxa_BufferPtr(Vertex) vertex_ptr;
    daxa_BufferPtr(UniformBufferObject) ubo_ptr;
    daxa_BufferPtr(PerInstanceData) instance_buffer_ptr;
};

#ifdef __cplusplus
    }
#endif