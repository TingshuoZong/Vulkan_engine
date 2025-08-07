#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tiny_gltf.h>	// This include is necessary to get TINYGLTF and STB_IMAGE to work

#include <Core/Engine.h>

int main (int argc, char* argv[]) {
	return init();	// TODO: refactor so main does things but Engine starts things like Renderer
}