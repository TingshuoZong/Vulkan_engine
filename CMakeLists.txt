cmake_minimum_required(VERSION 3.19)

include("${CMAKE_CURRENT_LIST_DIR}/cmake/deps.cmake")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(VulkanEngine VERSION 1.0)

# -------------------- Copy shaders over to be compiled in runtime --------------------
set(SHADERS_SRC_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(SHADERS_DST_DIR "${CMAKE_BINARY_DIR}/shaders")

# Make sure the destination directory exists
file(MAKE_DIRECTORY ${SHADERS_DST_DIR})

# Copy all files from source to destination
file(GLOB SHADER_FILES "${SHADERS_SRC_DIR}/*")

foreach(SHADER_FILE ${SHADER_FILES})
    get_filename_component(FILE_NAME ${SHADER_FILE} NAME)
    configure_file(${SHADER_FILE} ${SHADERS_DST_DIR}/${FILE_NAME} COPYONLY)
endforeach()

file(GLOB_RECURSE SRCS ${PROJECT_SOURCE_DIR}/src/*.cpp src/*.hpp src/*.h)

message("src files:")
foreach(file ${SRCS})
    message(STATUS ${file})
endforeach()

add_executable(${PROJECT_NAME} ${SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC 
    ${PROJECT_SOURCE_DIR}/shaders
    include
)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

find_package(daxa CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE daxa::daxa)