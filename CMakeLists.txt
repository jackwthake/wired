cmake_minimum_required(VERSION 3.16)
project(wired C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Build SDL first ---
set(SDL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/SDL")
if(NOT EXISTS "${SDL_DIR}/CMakeLists.txt")
    message(FATAL_ERROR "SDL source not found at ${SDL_DIR}. Check the vendored dependency path.")
endif()
add_subdirectory("${SDL_DIR}")

# --- Dependencies ---
find_package(PkgConfig REQUIRED)
# pkg_check_modules(WAYLAND REQUIRED wayland-client wayland-egl)
pkg_check_modules(EGL     REQUIRED egl)
pkg_check_modules(GLES    REQUIRED glesv2)

# --- Main executable ---
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")

add_executable(${PROJECT_NAME}
    ${SOURCES}
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    $<$<CONFIG:Debug>:DEBUG>
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    "${SDL_DIR}/include"
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    SDL3::SDL3
    ${EGL_LIBRARIES}
    ${GLES_LIBRARIES}
)

# --- Copy assets to build directory ---
file(GLOB_RECURSE ASSET_FILES RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/assets/*")

add_custom_target(copy_assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "${CMAKE_CURRENT_BINARY_DIR}/assets"
    COMMENT "Copying assets to build directory"
    SOURCES ${ASSET_FILES}
)

add_dependencies(${PROJECT_NAME} copy_assets)