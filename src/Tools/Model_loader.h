#pragma once

#include "shared.inl"

#include <tiny_gltf.h>
#include <stb_image.h>

struct ParsedPrimitive {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::size_t vertexCount;
    std::size_t indexCount;

    std::optional<tinygltf::Image> albedo;
};

struct ParsedMesh {
    std::vector<ParsedPrimitive> primitives;
};

class GLTF_Loader {
public:
    GLTF_Loader() {}

    ParsedPrimitive getModelData(int meshNo, int primitiveNo) { return modelData[meshNo].primitives[primitiveNo]; }

    void OpenFile(const std::string& path);
    void LoadModel();

    void purgeImages();

    std::vector<ParsedMesh> modelData;

    std::string path;
private:
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;


};