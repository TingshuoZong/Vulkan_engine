#extension GL_EXT_debug_printf : enable

#include <skybox_rendering_shared.inl>

// Enabled the push constant PushConstant we specified in shared.inl
DAXA_DECL_PUSH_CONSTANT(PushConstant, push)

layout(location = 0) out daxa_f32vec3 v_ray_dir;

void main() {
    const vec2 POS[6] = vec2[](
        vec2(-1.0, -1.0), vec2( 1.0, -1.0), vec2( 1.0,  1.0), // first triangle
        vec2(-1.0, -1.0), vec2( 1.0,  1.0), vec2(-1.0,  1.0)  // second triangle
    );

    gl_Position = vec4(POS[gl_VertexIndex], 0.0f, 1.0f);
    UniformBufferObject ubo = deref(push.ubo_ptr);

    vec4 clip = vec4(POS[gl_VertexIndex], 1.0, 1.0);
    vec4 view = inverse(ubo.proj) * clip;
    vec3 view_dir = normalize(view.xyz / view.w);

    v_ray_dir = (transpose(ubo.view_rot) * vec4(view_dir, 0.0)).xyz;
}