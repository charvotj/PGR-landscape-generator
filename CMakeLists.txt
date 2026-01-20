cmake_minimum_required(VERSION 3.19)

project(PGR)

option(SDL_STATIC "" ON)
option(SDL_SHARED "" OFF)

add_subdirectory(libs/SDL-release-3.2.24)
add_subdirectory(libs/geGL-master)
add_subdirectory(libs/glm-1.0.2)

set(MY_SOURCES
    src/main.h
    src/global_defines.h
    src/main.cpp
    src/skybox.h
    src/skybox.cpp
    src/gl_basics.h
    src/gl_basics.cpp
    src/terrain.h
    src/terrain.cpp
    src/obj_load.h
    src/obj_load.cpp
    src/fps_counter.h
    src/fps_counter.cpp
    src/camera_and_control.h
    src/camera_and_control.cpp
    src/billboards.h
    src/billboards.cpp
    src/trees.h
    src/trees.cpp
    src/shadows.h
    src/shadows.cpp
)

add_executable(${PROJECT_NAME} 
    ${MY_SOURCES}
    libs/stb_image/stb_image.h
    libs/stb_image/stb_image_impl.cpp
    libs/tiny_obj_loader/tiny_obj_loader.h
    libs/tiny_obj_loader/tiny_obj_loader_impl.cpp
    )

target_include_directories(${PROJECT_NAME} PUBLIC libs/stb_image)
target_include_directories(${PROJECT_NAME} PUBLIC libs/tiny_obj_loader)

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
    RESOURCE_DIR="${CMAKE_SOURCE_DIR}/resources/"
    SHADERS_DIR="${CMAKE_SOURCE_DIR}/src/shaders/"
    )


target_link_libraries(${PROJECT_NAME} 
    PUBLIC 
    SDL3::SDL3
    geGL::geGL
    glm::glm
    imgui)



# ============================================
# Dear ImGui
# ============================================
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/libs/imgui)

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

target_link_libraries(imgui PUBLIC SDL3::SDL3)  # backend SDL3 potřebuje SDL
