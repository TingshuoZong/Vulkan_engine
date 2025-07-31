#pragma once

#include "shared.inl"

#include <tiny_gltf.h>
#include <stb_image.h>

struct ParsedPrimitive {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::size_t vertexCount;
    std::size_t indexCount;
};

class GLTF_Loader {
public:
    GLTF_Loader() {}

    ParsedPrimitive getModelData(int meshNo) { return modelData[meshNo]; }

    void OpenFile(const std::string& path);
    void LoadModel();

    std::vector<ParsedPrimitive> modelData;
    std::optional<tinygltf::Image> albedo;

    std::string path;
private:
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;


};