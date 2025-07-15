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
};
#else
// GPU side only definitions
struct UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
};

struct PerInstanceData {
    mat4 model_matrix;
};
#endif

struct MyVertex {
    daxa_f32vec3 position;
    daxa_f32vec3 color;
};

DAXA_DECL_BUFFER_PTR(MyVertex)
DAXA_DECL_BUFFER_PTR(UniformBufferObject)
DAXA_DECL_BUFFER_PTR(PerInstanceData)

struct MyPushConstant {
    daxa_BufferPtr(MyVertex) my_vertex_ptr;
    daxa_BufferPtr(UniformBufferObject) ubo_ptr;
    daxa_BufferPtr(PerInstanceData) instance_buffer_ptr;
};