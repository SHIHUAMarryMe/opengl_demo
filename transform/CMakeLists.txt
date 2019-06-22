cmake_minimum_required(VERSION 3.10)

project(transform)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-ggdb -O2")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


message(${CMAKE_CXX_FLAGS})


include_directories("../glad/include")
include_directories("./stb_image")
include_directories("../glm")

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)


if(glfw3_FOUND)
    message("found glfw3")
endif()

if(OpenGL_FOUND)
    message("found opengl")
endif()



add_executable(${PROJECT_NAME} main.cpp glad_src/glad.c stb_image/stb_image.cpp)

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

