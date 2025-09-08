cmake_minimum_required(VERSION 3.10)
project(PersonalEconomyTracker)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB IMGUI_SRC
    include/external/imgui/*.cpp
    include/external/imgui/*.h
    include/external/imgui/backends/imgui_impl_glfw.cpp
    include/external/imgui/backends/imgui_impl_opengl3.cpp
)


# GLAD source
set(GLAD_SRC src/glad.c)

add_subdirectory(include/external/glfw)

# Include directories
include_directories(
    src
    src/gui
    src/model
    src/data
    src/utils
)


# Source files
set(SOURCES
    src/main.cpp
    src/app.cpp
    src/gui/gui.cpp
    src/model/transaction.cpp
    src/data/storage.cpp
    src/utils/plot.cpp
)

file(GLOB IMPLOT_SOURCES 
    include/external/implot/*.cpp
    include/external/implot/backends/*.cpp
)

# Add executable
add_executable(${PROJECT_NAME} 
    ${SOURCES}
    ${GLAD_SRC}
    ${IMGUI_SRC}
    ${IMPLOT_SOURCES}
)

# set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE OFF)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    include
    include/glad
    include/KHR
    include/external/imgui
    include/external/imgui/backends
    include/external/imgui/examples/libs/glfw/include
    include/external/json/single_include
    include/external/implot
)

# Find and link libraries
# find_package(OpenGL REQUIRED)
# find_package(GLM REQUIRED)
# find_package(glfw3 REQUIRED)
# find_package(GLAD REQUIRED)
# find_package(imgui REQUIRED)

target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw
    OpenGL32
    gdi32
    user32
    shell32
)