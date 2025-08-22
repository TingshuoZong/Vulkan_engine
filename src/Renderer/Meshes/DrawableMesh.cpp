#include "DrawableMesh.h"
/// @file Currently unused due to uploading mesh being done in @ref DrawGroup

// void DrawableMesh::upload_mesh_data_task(
//     daxa::TaskGraph& tg,
//     const std::vector<Vertex>& vertex_data,
//     const std::vector<uint32_t>& index_data
// ) {
//     tg.use_persistent_buffer(task_vertex_buffer);
//     tg.use_persistent_buffer(task_index_buffer);
//
//     tg.add_task({
//         .attachments = {
//             daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, this->task_vertex_buffer),
//             daxa::inl_attachment(daxa::TaskBufferAccess::TRANSFER_WRITE, this->task_index_buffer),
//         },
//         .task = [=, this](daxa::TaskInterface ti) {
//             auto vertex_staging = ti.device.create_buffer({
//                            .size = vertex_data.size() * sizeof(Vertex),
//                            .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
//                            .name = this->name + ">" + name + "vertex staging buffer",
//             });
//             ti.recorder.destroy_buffer_deferred(vertex_staging);
//             auto* vertex_ptr = ti.device.buffer_host_address_as<Vertex>(vertex_staging).value();
//             memcpy(vertex_ptr, vertex_data.data(), vertex_data.size() * sizeof(Vertex));
//             ti.recorder.copy_buffer_to_buffer({
//                 .src_buffer = vertex_staging,
//                 .dst_buffer = ti.get(this->task_vertex_buffer).ids[0],
//                 .size = vertex_data.size() * sizeof(Vertex),
//             });
//
//             auto index_staging = ti.device.create_buffer({
//                 .size = index_data.size() * sizeof(uint32_t),
//                 .allocate_info = daxa::MemoryFlagBits::HOST_ACCESS_RANDOM,
//                 .name = this->name + ">" + name + " index staging buffer",
//                 });
//             ti.recorder.destroy_buffer_deferred(index_staging);
//             auto* index_ptr = ti.device.buffer_host_address_as<uint32_t>(index_staging).value();
//             std::memcpy(index_ptr, index_data.data(), index_data.size() * sizeof(uint32_t));
//             ti.recorder.copy_buffer_to_buffer({
//                 .src_buffer = index_staging,
//                 .dst_buffer = ti.get(this->task_index_buffer).ids[0],
//                 .size = index_data.size() * sizeof(uint32_t),
//             });
//         },
//         .name = this->name + ">" + name + " upload mesh data",
//         });
// }
