

#include <daxa/daxa.inl>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct MyVertex {
    daxa_f32vec3 position;
    daxa_f32vec3 color;
};

DAXA_DECL_BUFFER_PTR(MyVertex)


struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

DAXA_DECL_BUFFER_PTR(UniformBufferObject)

struct MyPushConstant {
    daxa_BufferPtr(MyVertex) my_vertex_ptr;
    daxa_BufferPtr(UniformBufferObject) ubo_ptr;
};