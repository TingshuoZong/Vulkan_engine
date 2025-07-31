cmake_minimum_required(VERSION 3.19)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/deps.cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(VulkanEngine VERSION 1.0)

# -------------------- Copy shaders over to be compiled in runtime --------------------
set(SHADERS_SRC_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(SHADERS_DST_DIR "${CMAKE_BINARY_DIR}/shaders")

# Make sure the destination directory exists
file(MAKE_DIRECTORY ${SHADERS_DST_DIR})

# Copy all files from source to destination
file(GLOB SHADER_FILES
    "${SHADERS_SRC_DIR}/*.inl"
    "${SHADERS_SRC_DIR}/*.cpp"
    "${SHADERS_SRC_DIR}/*.h"
    "${SHADERS_SRC_DIR}/*.hpp"
)

foreach(SHADER_FILE ${SHADER_FILES})
    # This adds the files to the VS project but doesn��t compile them
    source_group("Shaders" FILES ${SHADER_FILE})
endforeach()

foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(FILE_NAME ${SHADER_FILE} NAME)
    configure_file(${SHADER_FILE} ${SHADERS_DST_DIR}/${FILE_NAME} COPYONLY)
endforeach()

file(GLOB_RECURSE SRCS
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/*.hpp
    ${PROJECT_SOURCE_DIR}/src/*.h
)

if(MSVC)
    # Arrange groups in Visual Studio
    foreach(source_file IN LISTS SRCS)
        file(RELATIVE_PATH source_path "${PROJECT_SOURCE_DIR}/src" "${source_file}")
        get_filename_component(source_dir "${source_path}" PATH)
        string(REPLACE "/" "\\" source_group "${source_dir}")
        if(source_group STREQUAL "")
            set(source_group "Source Files")
        else()
            set(source_group "Source Files\\${source_group}")
        endif()
    
        source_group("${source_group}" FILES "${source_file}")
    endforeach()
endif()

message("src files:")
foreach(file ${SRCS})
    message(STATUS ${file})
endforeach()

add_executable(${PROJECT_NAME} ${SRCS})

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}/shaders
    ${PROJECT_SOURCE_DIR}/lib/glm
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/src
)

find_package(glfw3 CONFIG REQUIRED)

find_package(daxa CONFIG REQUIRED)

target_sources(${PROJECT_NAME} PRIVATE
    ${SHADER_FILES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw
    daxa::daxa
)