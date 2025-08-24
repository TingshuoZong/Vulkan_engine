// Enabled the extension GL_EXT_debug_printf
#extension GL_EXT_debug_printf : enable

#include <skybox_rendering_shared.inl>

// Enabled the push constant PushConstant we specified in shared.inl
DAXA_DECL_PUSH_CONSTANT(PushConstant, push)

layout(location = 0) in daxa_f32vec3 v_ray_dir;
layout(location = 0) out daxa_f32vec4 color;

void main() {
    vec3 dir = normalize(v_ray_dir);

    float u = (atan(dir.z, dir.x) + 3.14159265) / (2.0 * 3.14159265);
    float v = 1 - (asin(dir.y) + 3.14159265 / 2.0) / 3.14159265;
    vec2 uv = vec2(u, v);

    vec4 tex = texture(daxa_sampler2D(push.texture, push.tex_sampler), uv);
    color = vec4(tex.rgb, 1.0f);
}