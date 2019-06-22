
#include <cmath>
#include <iostream>

#include <glad/glad.h> // must include this before glfw(#error OpenGL header already included, remove this include, glad already provides it);
#include <GLFW/glfw3.h>

#define GLM_FORCE_CXX14
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image/stb_image.h"

static constexpr int WIDTH{800};
static constexpr int HEIGHT{600};

static constexpr const char *vertexShaderSource{
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 texCoord;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "	texCoord = vec2(aTexCoord.x, aTexCoord.y);\n"
    "}"};

static constexpr const char *fragmentShaderSource{
    "#version 330 core\n"
    "out vec4 fragmentColor;\n"
    "in vec2 texCoord;\n"
    "uniform sampler2D texture1;\n"
    "uniform sampler2D texture2;\n"
    "void main()\n"
    "{\n"
    "	fragmentColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), 0.2);\n"
    "}"};

static void processInput(GLFWwindow *window)
{
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}

static void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height)
{
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // glfw window creation
  // --------------------
  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
  if (window == nullptr)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // build and compile our shader program
  // ------------------------------------
  // vertex shader
  int vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
  glCompileShader(vertexShader);

  // check for shader compile errors
  int success{};
  char infoLog[512]{};
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }
  // fragment shader
  int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
  glCompileShader(fragmentShader);
  // check for shader compile errors
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << infoLog << std::endl;
  }

  // link shaders
  int shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);

  // check for linking errors
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success)
  {
    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << infoLog << std::endl;
  }

  // after linking program, you can delete shaders.
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  const float vertices[]{
      -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, 0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
      0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

      -0.5f, 0.5f, 0.5f, 1.0f, 0.0f, -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

      0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
      0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

      -0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
      0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
      -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

      -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
      0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
      -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, -0.5f, 0.5f, -0.5f, 0.0f, 1.0f};

  // world space positions of our cubes
  const glm::vec3 cubePositions[]{
      glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(2.0f, 5.0f, -15.0f),
      glm::vec3(-1.5f, -2.2f, -2.5f), glm::vec3(-3.8f, -2.0f, -12.3f),
      glm::vec3(2.4f, -0.4f, -3.5f), glm::vec3(-1.7f, 3.0f, -7.5f),
      glm::vec3(1.3f, -2.0f, -2.5f), glm::vec3(1.5f, 2.0f, -2.5f),
      glm::vec3(1.5f, 0.2f, -1.5f), glm::vec3(-1.3f, 1.0f, -1.5f)};

  GLuint VAO{}; // vertex array object.
  GLuint VBO{}; // vertex buffer object.

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // tell shader how to parse vertices.
  // position attribute.
  GLint offset{};
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        reinterpret_cast<void *>(offset));
  glEnableVertexAttribArray(0);

  // texture coordinate.
  offset = 3 * sizeof(float);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        reinterpret_cast<void *>(offset));
  glEnableVertexAttribArray(1);

  // load and create a texture
  GLuint texture{};
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have
                                         // effect on this texture object

  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // load image, create texture and generate mipmaps
  GLint width{}, height{}, nrChannels{};
  unsigned char *data{stbi_load("/home/shihua/projects/learn_opengl/transform/image/container.jpg", &width, &height, &nrChannels, 0)};

  if (data)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
  data = nullptr;

  GLuint texture2{};
  glGenTextures(1, &texture2);
  glBindTexture(GL_TEXTURE_2D, texture2);

  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // tell stb_image.h to flip loaded texture's on the y-axis.
  stbi_set_flip_vertically_on_load(true);

  // load image, create texture and generate mipmaps
  data = stbi_load("/home/shihua/projects/learn_opengl/transform/image/awesomeface.png", &width, &height, &nrChannels, 0);

  if (data)
  {
    // note that the awesomeface.png has transparency and thus an alpha channel,
    // so make sure to tell OpenGL the data type is of GL_RGBA
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
  data = nullptr;

  // tell opengl for each sampler2D to which texture unit it belongs to (only
  // has to be done once)
  glUseProgram(shaderProgram);

  GLint texture1Id{glGetUniformLocation(shaderProgram, "texture1")};
  glUniform1i(texture1Id, 0);

  GLint texture2Id{glGetUniformLocation(shaderProgram, "texture2")};
  glUniform1i(texture2Id, 1);

  // set the uniform var projection.
  glm::mat4 projectionMat4{glm::perspective(glm::radians(45.0f),
                                            static_cast<float>(WIDTH / HEIGHT), 0.1f, 100.0f)};
  GLint projectionLocation{glGetUniformLocation(shaderProgram, "projection")};
  glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projectionMat4[0][0]);

  // render loop
  while (!glfwWindowShouldClose(window))
  {
    // input
    // -----
    processInput(window);

    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // clear last time depth-testing.

    // bind textures to specify uniform.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    // render container
    glUseProgram(shaderProgram);

    // camera/view
    glm::mat4 view{1.0f};
    GLfloat radius{45.0f};
    GLfloat cameraX{static_cast<GLfloat>(std::sin(glfwGetTime())) * radius};
    GLfloat cameraZ{static_cast<GLfloat>(std::cos(glfwGetTime())) * radius};

    view = glm::lookAt(glm::vec3{cameraX, 0.0f, cameraZ}, glm::vec3{.0f, .0f, .0f}, glm::vec3{.0f, 1.0f, .0f});

    GLint viewLocation{glGetUniformLocation(shaderProgram, "view")};
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

    glBindVertexArray(VAO);

    std::size_t cubesN{sizeof(cubePositions) / sizeof(glm::vec3)};

    for (std::size_t index = 0; index < cubesN; ++index)
    {
      glm::mat4 model{1.0f};
      model = glm::translate(model, cubePositions[index]);

      // translate local position to world position.
      GLfloat angle{20.0f * index};
      model = glm::rotate(model, glm::radians(angle), glm::vec3{1.0f, 0.3f, 0.5f});

      // initialize model(matrix4) in glsl.
      GLint modelLocation{glGetUniformLocation(shaderProgram, "model")};
      glUniformMatrix4fv(modelLocation, 1, GL_FALSE, &model[0][0]);

      glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved
    // etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // optional: de-allocate all resources once they've outlived their purpose:
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);

  // glfw: terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}