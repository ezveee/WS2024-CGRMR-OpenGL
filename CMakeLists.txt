cmake_minimum_required(VERSION 3.20)
project(WS2024_CGRMR_OpenGL)

set(CMAKE_CXX_STANDARD 20)

# Add  executable
add_executable(WS2024_CGRMR_OpenGL src/main.cpp)

add_definitions(-DGLEW_STATIC)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/libs)
include_directories(${CMAKE_SOURCE_DIR}/libs/glew/include)
include_directories(${CMAKE_SOURCE_DIR}/libs/freeglut/include)

# Link directories
link_directories(${CMAKE_SOURCE_DIR}/libs/glew/lib)
link_directories(${CMAKE_SOURCE_DIR}/libs/freeglut/lib)

# Link libraries
target_link_libraries(WS2024_CGRMR_OpenGL
        ${CMAKE_SOURCE_DIR}/libs/glew/lib/libglew32.a
        ${CMAKE_SOURCE_DIR}/libs/freeglut/lib/libfreeglut.dll.a
        opengl32
)

# Copy runtime DLLs after build
add_custom_command(TARGET WS2024_CGRMR_OpenGL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/libs/glew/bin/glew32.dll
        $<TARGET_FILE_DIR:WS2024_CGRMR_OpenGL>
)

add_custom_command(TARGET WS2024_CGRMR_OpenGL POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/libs/freeglut/bin/libfreeglut.dll
        $<TARGET_FILE_DIR:WS2024_CGRMR_OpenGL>
)
