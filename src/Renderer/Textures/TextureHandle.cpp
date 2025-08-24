#include "TextureHandle.h"

#include <utility>

TextureHandle::TextureHandle(daxa::Device& device)
    : width(0), height(0), channels(0), device(device) {
}

void TextureHandle::cleanup() const {
    device.destroy_image(image);
    device.destroy_buffer(texture_staging_buffer);
}

void TextureHandle::stream_texture_from_memory(const std::string& fileName, const std::string &debug_name, BulkTextureUploadManager& manager) {
    this->name = fileName + debug_name;
    std::string path = TEXTURE_PATH + fileName;

    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!pixels) { std::cerr << "stb_image failed\n"; }

    uint32_t size_bytes = width * height * 4;

    load_textures_into_buffers(pixels, size_bytes, manager);

    stbi_image_free(pixels);
}

void TextureHandle::stream_texture_from_data(const tinygltf::Image& gltf_image, std::string debug_name, BulkTextureUploadManager& manager) {
    this->name = std::move(debug_name);

    width = gltf_image.width;
    height = gltf_image.height;
    channels = gltf_image.component;

    const auto* pixels = reinterpret_cast<const stbi_uc*>(gltf_image.image.data());
    size_t size_bytes = width * height * 4; // force RGBA8

    load_textures_into_buffers(pixels, size_bytes, manager);
}

inline void TextureHandle::load_textures_into_buffers(const stbi_uc* pixels, uint32_t size_bytes, BulkTextureUploadManager& manager) {
    image = device.create_image({
        .format = daxa::Format::R8G8B8A8_UNORM,
        .size = {static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1},
        .usage = daxa::ImageUsageFlagBits::TRANSFER_DST | daxa::ImageUsageFlagBits::SHADER_SAMPLED,
        .name = name + " texture image",
    });

    task_texture_image = daxa::TaskImage({
        .initial_images = {.images = std::span{&image, 1}},
        .name = name + " texture task image"
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

    UploadData uploadData {
        .task_texture_staging = task_texture_staging,
        .task_texture_image = task_texture_image,
        .image = image,
        .width = static_cast<uint32_t>(width),
        .height = static_cast<uint32_t>(height),
    };
    manager.submitUpload(uploadData);
    //manager.submitUpload2(this);
}

std::vector<daxa::ImageViewId> BulkTextureUploadManager::bulkUploadTextures(daxa::TaskGraph &taskGraph, const std::string& name) {

    for (auto& upload : uploads) {
        taskGraph.use_persistent_buffer(upload.task_texture_staging);
        taskGraph.use_persistent_image(upload.task_texture_image);
    }

    std::vector<daxa::TaskAttachmentInfo> result;

    for (auto& upload : uploads) {
        result.emplace_back(daxa::TaskAttachmentInfo{daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_READ, upload.task_texture_staging.view())});
        result.emplace_back(daxa::TaskAttachmentInfo{daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, upload.task_texture_image.view())});
    }

    taskGraph.add_task({
        .attachments = result,
        .task = [&](const daxa::TaskInterface &ti) {
            for (auto& upload : uploads) {
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
                    .image_id = ti.get(upload.task_texture_image).ids[0],
                }),
                ti.recorder.copy_buffer_to_image({
                    .buffer = ti.get(upload.task_texture_staging).ids[0],
                    .image = ti.get(upload.task_texture_image).ids[0],
                    .image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
                    .image_extent = {static_cast<uint32_t>(upload.width), static_cast<uint32_t>(upload.height), 1}
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
                    .image_id = ti.get(upload.task_texture_image).ids[0],
                });
            }
        },
        .name = name + " upload"
    });

    for (auto& upload : uploads) {
        views.push_back(upload.image.default_view());
    }

    return std::move(views);
}

//std::vector<daxa::ImageViewId> BulkTextureUploadManager::bulkUploadTextures(daxa::TaskGraph& taskGraph, const std::string& name) {
//
//    for (auto upload : uploads) {
//        taskGraph.use_persistent_buffer(upload->task_texture_staging);
//        taskGraph.use_persistent_image(upload->task_texture_image);
//    }
//
//    std::vector<daxa::TaskAttachmentInfo> result;
//
//    for (auto upload : uploads) {
//        result.emplace_back(daxa::TaskAttachmentInfo{ daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_READ, upload->task_texture_staging.view()) });
//        result.emplace_back(daxa::TaskAttachmentInfo{ daxa::inl_attachment(daxa::TaskImageAccess::TRANSFER_WRITE, upload->task_texture_image.view()) });
//    }
//
//    taskGraph.add_task({
//        .attachments = result,
//        .task = [&](const daxa::TaskInterface& ti) {
//            for (auto upload : uploads) {
//                ti.recorder.pipeline_barrier_image_transition({
//                    .src_access = daxa::AccessConsts::NONE,
//                    .dst_access = daxa::AccessConsts::TRANSFER_WRITE,
//                    .src_layout = daxa::ImageLayout::UNDEFINED,
//                    .dst_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
//                    .image_slice = {
//                        .base_mip_level = 0,
//                        .level_count = 1,
//                        .base_array_layer = 0,
//                        .layer_count = 1
//                    },
//                    .image_id = ti.get(upload->task_texture_image).ids[0],
//                }),
//                ti.recorder.copy_buffer_to_image({
//                    .buffer = ti.get(upload->task_texture_staging).ids[0],
//                    .image = ti.get(upload->task_texture_image).ids[0],
//                    .image_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
//                    .image_extent = {static_cast<uint32_t>(upload->width), static_cast<uint32_t>(upload->height), 1}
//                }),
//                ti.recorder.pipeline_barrier_image_transition({
//                    .src_access = daxa::AccessConsts::TRANSFER_WRITE,
//                    .dst_access = daxa::AccessConsts::FRAGMENT_SHADER_READ,
//                    .src_layout = daxa::ImageLayout::TRANSFER_DST_OPTIMAL,
//                    .dst_layout = daxa::ImageLayout::READ_ONLY_OPTIMAL,
//                    .image_slice = {
//                        .base_mip_level = 0,
//                        .level_count = 1,
//                        .base_array_layer = 0,
//                        .layer_count = 1
//                    },
//                    .image_id = ti.get(upload->task_texture_image).ids[0],
//                });
//            }
//        },
//        .name = name + " upload"
//        });
//
//    for (auto upload : uploads) {
//        views.push_back(upload->image.default_view());
//    }
//
//    return std::move(views);
//}