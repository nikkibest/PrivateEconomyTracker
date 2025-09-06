cmake_minimum_required(VERSION 3.10)
project(DearImGuiApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ImGui source files
file(GLOB IMGUI_SRC
    include/imgui/*.cpp
    include/imgui/*.h
    include/imgui/backends/imgui_impl_glfw.cpp
    include/imgui/backends/imgui_impl_opengl3.cpp
)


# GLAD source
set(GLAD_SRC src/glad.c)

# Main application
set(APP_SRC src/main.cpp)

# Add GLFW as a subdirectory
add_subdirectory(glfw)

# Combine all sources
add_executable(${PROJECT_NAME}
    ${APP_SRC}
    ${GLAD_SRC}
    ${IMGUI_SRC}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    include/glad
    include/KHR
    include/imgui
    include/imgui/backends
    include/imgui/examples/libs/glfw/include
)

# Link system libraries
target_link_libraries(${PROJECT_NAME}
    glfw
    OpenGL32
    gdi32
    user32
    shell32
)
