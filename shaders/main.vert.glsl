#include <daxa/daxa.inl>

// Enabled the extension GL_EXT_debug_printf
#extension GL_EXT_debug_printf : enable

#include <shared.inl>

// Enabled the push constant MyPushConstant we specified in shared.inl
DAXA_DECL_PUSH_CONSTANT(MyPushConstant, push)


layout(location = 0) out daxa_f32vec3 v_col;

void main() {
    MyVertex vert = deref(push.my_vertex_ptr[gl_VertexIndex]);
    UniformBufferObject ubo = deref(push.ubo_ptr);
    PerInstanceData pid = deref(push.instance_buffer_ptr[gl_InstanceIndex]);

    vec4 world_pos = pid.model_matrix * vec4(vert.position, 1.0);
    vec4 view_pos = ubo.view * world_pos;
    gl_Position = ubo.proj * view_pos;

    // gl_Position = daxa_f32vec4(vert.position, 1);
    v_col = vert.color;
}