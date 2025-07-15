#include <daxa/daxa.inl>

// Enabled the extension GL_EXT_debug_printf
#extension GL_EXT_debug_printf : enable

#include <shared.inl>

// Enabled the push constant MyPushConstant we specified in shared.inl
DAXA_DECL_PUSH_CONSTANT(MyPushConstant, push)

layout(location = 0) in daxa_f32vec3 v_col;
layout(location = 0) out daxa_f32vec4 color;
void main() {
    color = daxa_f32vec4(v_col, 1);

    // Debug printf is not necessary, we just use it here to show how it can be used.
    // To be able to see the debug printf output, you need to open Vulkan Configurator and enable it there.
    debugPrintfEXT("test\n");
}