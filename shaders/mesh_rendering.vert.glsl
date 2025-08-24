#extension GL_EXT_debug_printf : enable

#include <mesh_rendering_shared.inl>

// Enabled the push constant PushConstant we specified in shared.inl
DAXA_DECL_PUSH_CONSTANT(PushConstant, push)


layout(location = 0) out daxa_f32vec2 v_uv;
layout(location = 1) flat out int v_InstanceIndex;

void main() {
    Vertex vert = deref(push.vertex_ptr[gl_VertexIndex]);
    PerInstanceData instData = deref(push.instance_buffer_ptr[gl_InstanceIndex]);
    UniformBufferObject ubo = deref(push.ubo_ptr);

//    vec4 world_pos = instData.model_matrix * vec4(vert.position, 1.0);
//    vec4 view_pos = ubo.view * world_pos;
//    gl_Position = ubo.proj * view_pos;

    gl_Position = ubo.proj * ubo.view * instData.model_matrix * vec4(vert.position, 1.0);

    v_uv = vert.uv;
    v_InstanceIndex = gl_InstanceIndex;
}