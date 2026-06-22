cmake_minimum_required(VERSION 4.1)
project(Renderer C)

set(CMAKE_C_STANDARD 11)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

add_executable(Renderer main.c
        AspenRenderer/deps/glad/src/glad.c
        AspenRenderer/deps/glad/include/glad/glad.h
        AspenRenderer/deps/glad/include/KHR/khrplatform.h
        AspenRenderer/deps/stb_image.h
        AspenRenderer/Renderer.h
        ExampleAssets/Shaders/nebula.frag
        AspenRenderer/deps/stb_truetype.h
)

target_include_directories(Renderer PRIVATE
        AspenRenderer/deps/glad/include
)

target_link_libraries(Renderer PRIVATE glfw OpenGL::GL m)

file(COPY
        "ExampleAssets/Shaders/nebula.frag"
        "ExampleAssets/Shaders/grayscale.frag"
        "ExampleAssets/Font/SpringMinimalist.ttf"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
)

file(COPY "ExampleAssets/Textures/" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/Textures")

file(COPY
        "ExampleAssets/Textures/container.jpg"
        "ExampleAssets/Textures/uniformclouds.jpg"
        DESTINATION "${CMAKE_CURRENT_BINARY_DIR}"
)