#include "TextureHandle.h"

TextureHandle::TextureHandle(daxa::Device& device)
	: device(device) {}

void TextureHandle::cleanup() {
    device.destroy_image(image);
    device.destroy_buffer(texture_staging_buffer);
}

void TextureHandle::stream_texture_from_memory(std::string fileName) {
    this->name = fileName;
    std::string path = TEXTURE_PATH + fileName;

    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!pixels) { std::cerr << "stb_image failed\n"; }

    uint32_t size_bytes = width * height * 4;

    load_textures_into_buffers(pixels, size_bytes);

    stbi_image_free(pixels);
}

void TextureHandle::stream_texture_from_data(const tinygltf::Image& gltf_image, std::string debug_name) {
    this->name = debug_name;

    width = gltf_image.width;
    height = gltf_image.height;
    channels = gltf_image.component;

    const stbi_uc* pixels = reinterpret_cast<const stbi_uc*>(gltf_image.image.data());
    size_t size_bytes = width * height * 4; // force RGBA8

    load_textures_into_buffers(pixels, size_bytes);
}

inline void TextureHandle::load_textures_into_buffers(const stbi_uc* pixels, uint32_t size_bytes) {
    image = device.create_image({
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        .usage = daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        .name = name + " texture image"
    });

    task_texture_image = daxa::TaskImage({
        .initial_images = {.images = std::span{&image, 1}},
        .name = name + " texutre task image"
    });

    texture_staging_buffer = device.create_buffer({
        .size = size_bytes,
        .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_SEQUENTIAL_WRITE,
        .name = name + " texture staging buffer"
    });

    task_texture_staging = daxa::TaskBuffer({
        .initial_buffers = {.buffers = std::span{&texture_staging_buffer, 1} },
        .name = name + " staging task buffer"
    });

    {
        auto* ptr = device.buffer_host_address_as<std::byte>(task_texture_staging.get_state().buffers[0]).value();
        std::memcpy(ptr, pixels, size_bytes);
    }
}

daxa::ImageViewId TextureHandle::load_texture(daxa::TaskGraph& loop_task_graph) {
    loop_task_graph.use_persistent_buffer(task_texture_staging);
    loop_task_graph.use_persistent_image(task_texture_image);

    loop_task_graph.add_task({
        .attachments = {
            daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_READ, task_texture_staging.view()),
            daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, task_texture_image.view())
        },
        .task = [=](daxa::TaskInterface ti) {
            ti.recorder.pipeline_barrier_image_transition({
                .src_access = daxa::AccessConsts::NONE,
                .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
                .src_layout = daxa::ImageLayout::UNDEFINED,
                .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .image_slice = {
                    .base_mip_level = 0,
                    .level_count = 1,
                    .base_array_layer = 0,
                    .layer_count = 1
                },
                .image_id = ti.get(task_texture_image).ids[0],
            }),
            ti.recorder.copy_buffer_to_image({
                .buffer = ti.get(task_texture_staging).ids[0],
                .image = ti.get(task_texture_image).ids[0],
                .image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .image_extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1}
            }),
            ti.recorder.pipeline_barrier_image_transition({
                .src_access = daxa::AccessConsts::TRANSFER_WRITE,
                .dst_access = daxa::AccessConsts::FRAGMENT_SHADER_READ,
                .src_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                .dst_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
                .image_slice = {
                    .base_mip_level = 0,
                    .level_count = 1,
                    .base_array_layer = 0,
                    .layer_count = 1
                },
                .image_id = ti.get(task_texture_image).ids[0],
            });
        },
        .name = name + " upload"
        });

    texture_view = image.default_view();
    return texture_view;
}