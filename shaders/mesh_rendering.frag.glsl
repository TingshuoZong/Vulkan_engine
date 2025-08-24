// Enabled the extension GL_EXT_debug_printf
#extension GL_EXT_debug_printf : enable

#include <mesh_rendering_shared.inl>

// Enabled the push constant PushConstant we specified in shared.inl
DAXA_DECL_PUSH_CONSTANT(PushConstant, push)

layout(location = 0) in daxa_f32vec2 v_uv;
layout(location = 1) flat in int v_InstanceIndex;
layout(location = 0) out daxa_f32vec4 color;

void main() {
    PerInstanceData instData = deref(push.instance_buffer_ptr[v_InstanceIndex]);
    vec4 tex = texture(daxa_sampler2D(instData.texture, instData.tex_sampler), v_uv);
    color = tex;

    // Debug printf is not necessary, we just use it here to show how it can be used.
    // To be able to see the debug printf output, you need to open Vulkan Configurator and enable it there.
    // debugPrintfEXT("test\n");
}