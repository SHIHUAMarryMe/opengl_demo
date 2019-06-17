
#include <iostream>

extern "C"
{
#include "glad/glad.h"
#include <GLFW/glfw3.h>
}

static constexpr int WIDTH{ 800 };
static constexpr int HEIGHT{ 600 };

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    std::cout << "call back." << std::endl;
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window)
{
    if(glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        std::cout << "you pressed ENTER!" << std::endl;
        glfwSetWindowShouldClose(window, true);
    }
}


int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window{ glfwCreateWindow(800, 600, "Learn OpenGL", nullptr, nullptr) };

    if(!window)
    {
        std::cout << "failed to create a window!" << std::endl;
        glfwTerminate();

        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //glad: load all opengl functions.
    if(!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "failed to load functions!" << std::endl;

        return -1;
    }

    while(!glfwWindowShouldClose(window))
    {
        process_input(window);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate(); //release resource.

    return 0;
}
