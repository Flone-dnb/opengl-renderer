cmake_minimum_required(VERSION 3.20)

project(opengl_renderer)

set(BUILD_DIRECTORY_NAME OUTPUT)

# Add executable targets.
message(STATUS "Adding executable targets...")

# Add project target.
set(PROJECT_DIRECTORY renderer)
message(STATUS "Adding target ${PROJECT_DIRECTORY}...")
add_subdirectory(src/${PROJECT_DIRECTORY} ${BUILD_DIRECTORY_NAME}/${PROJECT_DIRECTORY})
