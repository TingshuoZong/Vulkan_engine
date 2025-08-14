#include "Model_loader.h"

void GLTF_Loader::OpenFile(const std::string& path) {
    std::string err, warn;

    bool success = loader.LoadASCIIFromFile(&model, &err, &warn, path);
    // For .glb: loader.LoadBinaryFromFile(&model, &err, &warn, ...);

    if (!warn.empty()) std::cout << "Warn: " << warn << std::endl;
    if (!err.empty()) std::cerr << "Err: " << err << std::endl;

    if (!success) {
        std::cerr << "Failed to load glTF model\n";
        return;
    }

    this->path = path;
}


void GLTF_Loader::LoadModel() {
    for (const auto& mesh : model.meshes) {
        ParsedMesh parsedMesh;

        int texture_index = -1;


        for (const auto& primitive : mesh.primitives) {
            ParsedPrimitive parsedPirimitive;

            // Check if model contains multiple textures
            if (primitive.material >= 0) {
                const auto& material = model.materials[primitive.material];
                if (material.values.contains("baseColorTexture")) {
                    texture_index = material.values.at("baseColorTexture").TextureIndex();
                    const auto& image = model.images[model.textures[texture_index].source];
                    parsedPirimitive.albedo = image;
                }
            }

            // === POSITION ===
            const auto& posAccessor = model.accessors[primitive.attributes.at("POSITION")];
            const auto& posView = model.bufferViews[posAccessor.bufferView];
            const auto& posBuffer = model.buffers[posView.buffer];
            const float* positions = reinterpret_cast<const float*>(
                &posBuffer.data[posView.byteOffset + posAccessor.byteOffset]);

            // === UV ===
            const float* uvs = nullptr;
            size_t uvCount = 0;
            if (primitive.attributes.contains("TEXCOORD_0")) {
                const auto& uvAccessor = model.accessors[primitive.attributes.at("TEXCOORD_0")];
                const auto& uvView = model.bufferViews[uvAccessor.bufferView];
                const auto& uvBuffer = model.buffers[uvView.buffer];
                uvs = reinterpret_cast<const float*>(
                    &uvBuffer.data[uvView.byteOffset + uvAccessor.byteOffset]);
                uvCount = uvAccessor.count;
            }

            size_t vertexCount = posAccessor.count;
            for (size_t i = 0; i < vertexCount; ++i) {
                Vertex v;
                v.position = {
                    positions[i * 3 + 0],
                    positions[i * 3 + 1],
                    positions[i * 3 + 2],
                };

                if (uvs && i < uvCount) {
                    v.uv = {
                        uvs[i * 2 + 0],
                        uvs[i * 2 + 1]
                    };
                } else {
                    v.uv = {0.0f, 0.0f};
                }

                parsedPirimitive.vertices.push_back(v);
            }

            // === INDICES ===
            size_t indexCount = 0;
            if (primitive.indices >= 0) {
                const auto& idxAccessor = model.accessors[primitive.indices];
                const auto& idxView = model.bufferViews[idxAccessor.bufferView];
                const auto& idxBuffer = model.buffers[idxView.buffer];
                const void* dataPtr = &idxBuffer.data[idxView.byteOffset + idxAccessor.byteOffset];

                for (size_t i = 0; i < idxAccessor.count; ++i) {
                    uint32_t idx = 0;
                    switch (idxAccessor.componentType) {
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            idx = reinterpret_cast<const uint8_t*>(dataPtr)[i]; break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            idx = reinterpret_cast<const uint16_t*>(dataPtr)[i]; break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            idx = reinterpret_cast<const uint32_t*>(dataPtr)[i]; break;
                        default: throw std::runtime_error("Unsupported index component type");
                    }
                    parsedPirimitive.indices.push_back(idx);
                }
                indexCount = idxAccessor.count;
            }
            parsedPirimitive.vertexCount = vertexCount;
            parsedPirimitive.indexCount = indexCount;

            parsedMesh.primitives.push_back(std::move(parsedPirimitive));
        }

        modelData.push_back(parsedMesh);
    }
}
