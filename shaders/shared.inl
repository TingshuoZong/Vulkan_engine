#pragma once

#include <daxa/daxa.inl>

#ifdef __cplusplus

    // CPU side only definitions
    #include <glm/glm.hpp>
    #include <glm/gtc/matrix_transform.hpp>

    struct UniformBufferObject {
        glm::mat4 model;
        glm::mat4 view;
        glm::mat4 proj;
    };

    struct PerInstanceData {
        glm::mat4 model_matrix;
        daxa_ImageViewId texture;
        daxa_SamplerId tex_sampler;
        daxa_u32 _pad0;
        daxa_u32 _pad1;
    };
#else

    // GPU side only definitions
    #include <daxa/daxa.glsl>

    struct UniformBufferObject {
        mat4 model;
        mat4 view;
        mat4 proj;
    };

    struct PerInstanceData {
        daxa_f32mat4x4 model_matrix;
        daxa_ImageViewId texture;
        daxa_SamplerId tex_sampler;
        daxa_u32 _pad0;
        daxa_u32 _pad1;
    };
#endif

// Code that can be 100% shared between CPU and GPU
struct Vertex {
    daxa_f32vec3 position;
    daxa_f32vec2 uv;
};

DAXA_DECL_BUFFER_PTR(Vertex)
DAXA_DECL_BUFFER_PTR(UniformBufferObject)
DAXA_DECL_BUFFER_PTR(PerInstanceData)

struct PushConstant {
    daxa_BufferPtr(Vertex) my_vertex_ptr;
    daxa_BufferPtr(UniformBufferObject) ubo_ptr;
    daxa_BufferPtr(PerInstanceData) instance_buffer_ptr;
};