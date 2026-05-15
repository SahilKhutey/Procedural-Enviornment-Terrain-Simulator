cmake_minimum_required(VERSION 3.20)
project(SimulationWorld)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization for Low-Space/MinGW
if(MINGW)
    add_compile_options(-O3 -march=native)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++")
endif()

# --- External Libraries (FetchContent) ---
include(FetchContent)

FetchContent_Declare(glfw URL https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip)
FetchContent_Declare(glad URL https://github.com/Dav1dde/glad/archive/refs/tags/v0.1.36.zip)
FetchContent_Declare(glm URL https://github.com/g-truc/glm/archive/refs/tags/0.9.9.8.zip)
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_Declare(imgui URL https://github.com/ocornut/imgui/archive/refs/tags/v1.90.8.zip)
FetchContent_Declare(fastnoiselite URL https://github.com/Auburn/FastNoiseLite/archive/refs/tags/v1.1.1.zip)

FetchContent_MakeAvailable(glfw glad glm json imgui fastnoiselite)

find_package(OpenGL REQUIRED)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC glfw OpenGL::GL)

add_library(fastnoise INTERFACE)
target_include_directories(fastnoise INTERFACE ${fastnoiselite_SOURCE_DIR}/Cpp)

# --- Sources ---
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(APPEND SOURCES "main.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

# --- Include Directories ---
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# --- Link Libraries ---
target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw 
    glad 
    glm::glm
    nlohmann_json::nlohmann_json
    imgui
    fastnoise
)
