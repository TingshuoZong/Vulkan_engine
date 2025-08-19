#pragma once

#include "shared.inl"

#include <tiny_gltf.h>
#include <stb_image.h>

/**
 * @brief Holds each glTF primitive (eqivilant to a mesh in this engine) 
 * 
 * Each primitive has a @c std::vector of incdicies which are @c uint32_t and verticies which are @ref meshRemderer::Vertex
 * They also hold the @c vertexCount and @c indexCount which are @c std::size_t
 * ParsedPrimitives will also hold texture data in terms of a @c std::optional<tinygltf::Image> right now only albedos are supported
 * The @c tinygltf::Image are the raw data but they need to be put through @ref TextureManager::stream_texture_from_data to be actually loaded into the GPU
 * Models can either be loaded with model_loader -> TextureHandle or model_loader -> TextureManager (-> TextureHandle)
 * 
 */
struct ParsedPrimitive {
    std::vector<meshRenderer::Vertex> vertices;
    std::vector<uint32_t> indices;
    std::size_t vertexCount;
    std::size_t indexCount;

    std::optional<tinygltf::Image> albedo;
};

/**
 * @brief Holds glTF meshes (equivlant to multiple meshes in this engine though they will typically be registered to the same @ref DrawGroup)
 * 
 * Holds a @c std::vector of @ref ParsedPrimitive
 * 
 */
struct ParsedMesh {
    std::vector<ParsedPrimitive> primitives;
};

/**
 * @brief Handles loading of glTF files
 * 
 * Each instance of @c GLTF_Loader will have one file open for which you can tell it to load the model
 * Model data except for textures will be cached until the GLTF_Loader is destroyed
 * @note Be sure to call @ref GLTF_Loader::purgeImages after the image file has been loaded into a daxa imageView
 * You would typically call @c OpenFile then @c LoadModel before getting your data using @getModelData and once you're done calling @purgeImages but keeping vertex and index data cached
 * The @c tinygltf::Image are the raw data but they need to be put through @ref TextureManager::stream_texture_from_data to be actually loaded into the GPU
 * 
 */
class GLTF_Loader {
public:
    GLTF_Loader() {}

    /// @brief Returns a @ref ParsedPrimitive storing the data for primitive requested
    /// @param meshNo The number of the mesh you want in terms of the indicies inside of the glTF file
    /// @param primitiveNo The primitive of the mesh you want in terms of the indicies inside of the glTF file
    /// @return A @ref ParsedPrimitive by value of the primitive requested
    ParsedPrimitive getModelData(int meshNo, int primitiveNo) { return modelData[meshNo].primitives[primitiveNo]; }

    /// @brief Opens the file resource based on the @c path
    /// @param path The path to the file
    void OpenFile(const std::string& path);
    /// @brief Loads and parses the glTF model data
    void LoadModel();

    /// @brief Removes the @c tinygltf::Image for all the textures in the @ref ParsedPrimitives that exist
    void purgeImages();

    std::vector<ParsedMesh> modelData;

    std::string path;
private:
    tinygltf::TinyGLTF loader;
    tinygltf::Model model;


};