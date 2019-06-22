cmake_minimum_required(VERSION 3.10)

project(hello_opengl)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-g -O2")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories("../glad/include")


find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)


add_executable(${PROJECT_NAME} main.cpp glad_src/glad.c)

target_link_libraries(${PROJECT_NAME}
    # Boost::system
    glfw3 #if do not link glfwxxx.a will: undefined reference to symbol 'pthread_key_delete@@GLIBC_2.2.5'
    pthread
    m
    GL
    GLU
    X11
    Xxf86vm
    Xrandr
    Xi
    dl
)

