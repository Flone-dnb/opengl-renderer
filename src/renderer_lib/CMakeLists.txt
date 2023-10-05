cmake_minimum_required(VERSION 3.20)

project(renderer_lib)

# Define some relative paths.
set(RELATIVE_EXT_PATH "../../ext")
set(ABSOLUTE_EXT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${RELATIVE_EXT_PATH})
set(RELATIVE_CMAKE_HELPERS_PATH "../.cmake")

# Include essential stuff.
include(${RELATIVE_CMAKE_HELPERS_PATH}/essential.cmake)

# Include helper functions.
include(${RELATIVE_CMAKE_HELPERS_PATH}/utils.cmake)

# -------------------------------------------------------------------------------------------------
#                                          TARGET SOURCES
# -------------------------------------------------------------------------------------------------

# Sources.
set(PROJECT_SOURCES
    src/Application.h
    src/Application.cpp
    src/Mesh.h
    src/Mesh.cpp
    src/MeshImporter.h
    src/MeshImporter.cpp
    src/CameraProperties.h
    src/CameraProperties.cpp
    src/Camera.h
    src/Camera.cpp
    src/GLMath.hpp
    src/GLFW.hpp
    src/Globals.hpp
    src/ShaderProgramMacro.hpp
    src/MathHelpers.hpp
    src/shapes/AABB.cpp
    src/shapes/AABB.h
    src/shapes/Frustum.h
    src/shapes/Frustum.cpp
    src/shapes/Plane.h
    src/shapes/Plane.cpp
    src/ImGuiWindow.hpp
    # add your .h/.cpp files here
)

# Define target.
add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

# -------------------------------------------------------------------------------------------------
#                                         CONFIGURE TARGET
# -------------------------------------------------------------------------------------------------

# Set target folder.
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER ${PROJECT_FOLDER})

# Enable more warnings and warnings as errors.
enable_more_warnings()

# Set C++ standard.
set(PROJECT_CXX_STANDARD_VERSION 23)
set(CMAKE_CXX_STANDARD ${PROJECT_CXX_STANDARD_VERSION})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_${PROJECT_CXX_STANDARD_VERSION})
message(STATUS "${PROJECT_NAME}: using the following C++ standard: ${CMAKE_CXX_STANDARD}")

# Add includes.
target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${RELATIVE_EXT_PATH})
target_include_directories(${PROJECT_NAME} PUBLIC src)

# -------------------------------------------------------------------------------------------------
#                                           TOOLS
# -------------------------------------------------------------------------------------------------

# Enable Address Sanitizer in `Debug` builds on non-Windows OS.
if(NOT IS_RELEASE_BUILD AND NOT WIN32)
    enable_address_sanitizer()
endif()

# Enable Clang-tidy.
enable_clang_tidy(${CMAKE_CURRENT_LIST_DIR}/../../.clang-tidy)

# -------------------------------------------------------------------------------------------------
#                                       DEPENDENCIES
# -------------------------------------------------------------------------------------------------

# External: "OpenGL".
find_package(OpenGL REQUIRED)
if (OPENGL_FOUND)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenGL::GL)
else (OPENGL_FOUND)
    message(FATAL_ERROR "unable to find OpenGL")
endif()

# External: "GLSL shader includer".
message(STATUS "${PROJECT_NAME}: adding external dependency \"GLSL-Shader-Includer\"...")
set(GLSL_SHADER_INCLUDER_ENABLE_TESTS OFF CACHE BOOL "" FORCE)
set(GLSL_SHADER_INCLUDER_ENABLE_DOXYGEN OFF CACHE BOOL "" FORCE)
add_subdirectory(${RELATIVE_EXT_PATH}/GLSL-Shader-Includes ${DEPENDENCY_BUILD_DIR_NAME}/GLSL-Shader-Includes SYSTEM)
target_link_libraries(${PROJECT_NAME} PUBLIC GlslShaderIncluderLib)

# External "GLAD".
message(STATUS "${PROJECT_NAME}: adding external dependency \"GLAD\"...")
set(GLAD_TARGET_NAME GLAD)
add_library(${GLAD_TARGET_NAME} STATIC
    ${RELATIVE_EXT_PATH}/glad/src/glad.c
)
target_include_directories(${GLAD_TARGET_NAME} PUBLIC ${RELATIVE_EXT_PATH}/glad/include)
set_target_properties(${GLAD_TARGET_NAME} PROPERTIES FOLDER ${EXTERNAL_FOLDER})
add_dependencies(${PROJECT_NAME} ${GLAD_TARGET_NAME})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GLAD_TARGET_NAME})

# External: "tinygltf"
message(STATUS "${PROJECT_NAME}: adding external dependency \"tinygltf\"...")
set(TINYGLTF_HEADER_ONLY ON CACHE BOOL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE BOOL "" FORCE)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE BOOL "" FORCE)
add_subdirectory(${RELATIVE_EXT_PATH}/tinygltf ${DEPENDENCY_BUILD_DIR_NAME}/tinygltf SYSTEM)
target_link_libraries(${PROJECT_NAME} PUBLIC tinygltf)
set_target_properties(tinygltf PROPERTIES FOLDER ${EXTERNAL_FOLDER})
add_dependencies(${PROJECT_NAME} tinygltf)

# External: "xxHash"
message(STATUS "${PROJECT_NAME}: adding external dependency \"xxHash\"...")
set(XXHASH_TARGET_NAME xxHash)
add_library(${XXHASH_TARGET_NAME} STATIC
    ${RELATIVE_EXT_PATH}/xxHash/xxhash.c
    ${RELATIVE_EXT_PATH}/xxHash/xxhash.h)
set_target_properties(${XXHASH_TARGET_NAME} PROPERTIES FOLDER ${EXTERNAL_FOLDER})
add_dependencies(${PROJECT_NAME} ${XXHASH_TARGET_NAME})
target_link_libraries(${PROJECT_NAME} PUBLIC ${XXHASH_TARGET_NAME})

# External: "GLM".
message(STATUS "${PROJECT_NAME}: adding external dependency \"GLM\"...")
add_subdirectory(${RELATIVE_EXT_PATH}/glm ${DEPENDENCY_BUILD_DIR_NAME}/glm SYSTEM)
target_link_libraries(${PROJECT_NAME} PUBLIC glm)
set_target_properties(glm PROPERTIES FOLDER ${EXTERNAL_FOLDER})
set_target_properties(uninstall PROPERTIES FOLDER ${EXTERNAL_FOLDER})

# External: "GLFW".
message(STATUS "${PROJECT_NAME}: adding external dependency \"GLFW\"...")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${RELATIVE_EXT_PATH}/glfw ${DEPENDENCY_BUILD_DIR_NAME}/glfw SYSTEM)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
set_target_properties(glfw PROPERTIES FOLDER ${EXTERNAL_FOLDER})
set_target_properties(update_mappings PROPERTIES FOLDER ${EXTERNAL_FOLDER})

# External: "ImGui"
message(STATUS "${PROJECT_NAME}: adding external dependency \"ImGui\"...")
set(IMGUI_TARGET_NAME ImGui)
file(GLOB IMGUI_CPP ${RELATIVE_EXT_PATH}/imgui/*.cpp)
file(GLOB IMGUI_HEADERS ${RELATIVE_EXT_PATH}/imgui/*.h)
add_library(${IMGUI_TARGET_NAME} STATIC
    ${IMGUI_CPP}
    ${IMGUI_HEADERS}
    ${RELATIVE_EXT_PATH}/imgui/backends/imgui_impl_glfw.cpp
    ${RELATIVE_EXT_PATH}/imgui/backends/imgui_impl_opengl3.cpp
    ${RELATIVE_EXT_PATH}/imgui/backends/imgui_impl_glfw.h
    ${RELATIVE_EXT_PATH}/imgui/backends/imgui_impl_opengl3.h
)
target_include_directories(${IMGUI_TARGET_NAME} SYSTEM PUBLIC ${RELATIVE_EXT_PATH}/imgui)
target_include_directories(${IMGUI_TARGET_NAME} SYSTEM PUBLIC ${RELATIVE_EXT_PATH}/imgui/backends)
set_target_properties(${IMGUI_TARGET_NAME} PROPERTIES FOLDER ${EXTERNAL_FOLDER})
add_dependencies(${PROJECT_NAME} ${IMGUI_TARGET_NAME})
target_link_libraries(${PROJECT_NAME} PUBLIC ${IMGUI_TARGET_NAME})

# External: portable-file-dialogs
message(STATUS "${PROJECT_NAME}: adding external dependency \"portable-file-dialogs\"...")
add_subdirectory(${RELATIVE_EXT_PATH}/portable-file-dialogs ${DEPENDENCY_BUILD_DIR_NAME}/portable-file-dialogs SYSTEM)
target_link_libraries(${PROJECT_NAME} PUBLIC portable_file_dialogs)
set_target_properties(portable_file_dialogs PROPERTIES FOLDER ${EXTERNAL_FOLDER})
