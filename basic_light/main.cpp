#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

static constexpr const int WIDTH{800};
static constexpr const int HEIGHT{600};

// lighting.
static const glm::vec3 light_pos{1.2f, 1.0f, 2.0f};

// camera
static glm::vec3 cameraPos{0.0f, 0.0f, 3.0f};
static glm::vec3 cameraFront{0.0f, 0.0f, -1.0f};
static glm::vec3 cameraUp{0.0f, 1.0f, 0.0f};
static float delta_time{};
static float last_frame{};

//mouse and scroll
static float field_of_view{45.0f};
static bool firstMouse{true};
static float lastX{WIDTH / 2.f};
static float lastY{HEIGHT / 2.f};
static float pitch{};
static float yaw{-90.f};

static constexpr const char *cubeVertexShaderSource{
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"

    "out vec3 FragPos;\n"
    "out vec3 Normal;\n"

    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"

    "void main()\n"
    "{\n"
    "    FragPos = vec3(model * vec4(aPos, 1.0));\n"
    "    Normal = aNormal;\n"
    "    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
    "}"};
static constexpr const char *cubeFragmentShaderSource{
    "#version 330 core\n"
    "out vec4 FragColor;\n"

    "in vec3 Normal;\n"
    "in vec3 FragPos;\n"

    "uniform vec3 lightPos;\n"
    "uniform vec3 lightColor;\n"
    "uniform vec3 objectColor;\n"

    "void main()\n"
    "{\n"
    // ambient
    "    float ambientStrength = 0.1;\n"
    "    vec3 ambient = ambientStrength * lightColor;\n"

    // diffuse
    "    vec3 norm = normalize(Normal);\n"
    "    vec3 lightDir = normalize(lightPos - FragPos);\n"
    "    float diff = max(dot(norm, lightDir), 0.0);\n"
    "    vec3 diffuse = diff * lightColor;\n"

    "    vec3 result = (ambient + diffuse) * objectColor;\n"
    "    FragColor = vec4(result, 1.0);\n"
    "}"};

static constexpr const char *lampVertexShaderSource{
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "void main()\n"
    "{\n"
    "	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}"};
static constexpr const char *lampFragmentShaderSource{
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "    FragColor = vec4(1.0);\n" // set alle 4 vector values to 1.0
    "}"};

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
static void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float camera_speed{5.0f * delta_time};

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPos += (cameraFront * camera_speed);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPos -= camera_speed * cameraFront;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * camera_speed;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * camera_speed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
static void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front{};
    front.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    front.y = std::sin(glm::radians(pitch));
    front.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
static void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    if (field_of_view >= 1.0f && field_of_view <= 45.0f)
    {
        field_of_view -= yoffset;
    }

    if (field_of_view <= 1.f)
    {
        field_of_view = 1.0f;
    }

    if (field_of_view >= 45.0f)
    {
        field_of_view = 45.0f;
    }
    std::cout << xoffset << "----------->" << yoffset << "   " << field_of_view << std::endl;
}

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // cube shaders.
    GLuint cubeVertexShader{};
    GLuint cubeFragmentShader{};

    cubeVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(cubeVertexShader, 1, &cubeVertexShaderSource, nullptr);
    glCompileShader(cubeVertexShader);

    GLint success{};
    glGetShaderiv(cubeVertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024]{};
        glGetShaderInfoLog(cubeVertexShader, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: "
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }

    cubeFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(cubeFragmentShader, 1, &cubeFragmentShaderSource, nullptr);
    glCompileShader(cubeFragmentShader);

    success = 0;
    glGetShaderiv(cubeFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024]{};
        glGetShaderInfoLog(cubeFragmentShader, 1024, nullptr, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: "
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }

    GLint cubeProgramId{glCreateProgram()};
    glAttachShader(cubeProgramId, cubeVertexShader);
    glAttachShader(cubeProgramId, cubeFragmentShader);
    glLinkProgram(cubeProgramId);

    success = 0;
    glGetProgramiv(cubeProgramId, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024]{};
        glGetProgramInfoLog(cubeFragmentShader, 1024, nullptr, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: "
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }

    // lamp shaders.
    GLuint lampVertexShader{};
    GLuint lampFragmentShader{};

    lampVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(lampVertexShader, 1, &lampVertexShaderSource, nullptr);
    glCompileShader(lampVertexShader);

    success = 0;
    glGetShaderiv(lampVertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024]{};
        glGetShaderInfoLog(lampVertexShader, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: "
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }

    lampFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(lampFragmentShader, 1, &lampFragmentShaderSource, nullptr);
    glCompileShader(lampFragmentShader);

    success = 0;
    glGetShaderiv(lampFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024]{};
        glGetShaderInfoLog(lampFragmentShader, 1024, nullptr, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: "
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }

    GLint lampProgramId{glCreateProgram()};
    glAttachShader(lampProgramId, lampVertexShader);
    glAttachShader(lampProgramId, lampFragmentShader);
    glLinkProgram(lampProgramId);

    success = 0;
    glGetProgramiv(lampProgramId, GL_LINK_STATUS, &success);
    if (!success)
    {
        GLchar infoLog[1024]{};
        glGetProgramInfoLog(lampProgramId, 1024, nullptr, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: "
                  << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    const float vertices[]{
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,

        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,

        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,

        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f};

    GLuint VBO{};
    GLuint cubeVAO{};
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    GLint offset{0};
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(offset));
    glEnableVertexAttribArray(0);

    offset = 3 * sizeof(float);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(offset));

    GLuint lampVAO{};
    glGenVertexArrays(1, &lampVAO);

    offset = 0;
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), reinterpret_cast<void *>(offset));
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = glfwGetTime();
        delta_time = currentFrame - last_frame;
        last_frame = currentFrame;

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(cubeProgramId);

        glm::vec3 object_color{1.0f, 0.5f, 0.31f};
        GLint objectColorLoc{glGetUniformLocation(cubeProgramId, "objectColor")};
        glUniform3fv(objectColorLoc, 1, &object_color[0]);

        glm::vec3 light_color{1.0f, 1.0f, 1.0f};
        GLint lightColorLoc{glGetUniformLocation(cubeProgramId, "lightColor")};
        glUniform3fv(lightColorLoc, 1, &light_color[0]);

        GLint lightPosLoc{glGetUniformLocation(cubeProgramId, "lightPos")};
        glUniformMatrix4fv(lightPosLoc, 1, GL_FALSE, &light_pos[0]);

        // view/projection transformations
        glm::mat4 projection{glm::perspective(glm::radians(field_of_view), static_cast<float>(WIDTH / HEIGHT), 0.1f, 100.0f)};
        glUniformMatrix4fv(glGetUniformLocation(cubeProgramId, "projection"), 1, GL_FALSE, &projection[0][0]);

        // camera/view transformation
        glm::mat4 view{1.0f}; // make sure to initialize matrix to identity matrix first
        view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(cubeProgramId, "view"), 1, GL_FALSE, &view[0][0]);

        // model
        glm::mat4 model{1.0f};
        glUniformMatrix4fv(glGetUniformLocation(cubeProgramId, "model"), 1, GL_FALSE, &model[0][0]);

        // render the cube
        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // also draw the lamp object
        glUseProgram(lampProgramId);
        glUniformMatrix4fv(glGetUniformLocation(lampProgramId, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(lampProgramId, "view"), 1, GL_FALSE, &projection[0][0]);
        model = glm::mat4(1.0f);
        model = glm::translate(model, light_pos);
        model = glm::scale(model, glm::vec3(0.2f)); // a smaller lamp cube
        glUniformMatrix4fv(glGetUniformLocation(lampProgramId, "model"), 1, GL_FALSE, &projection[0][0]);

        glBindVertexArray(lampVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lampVAO);
    glDeleteBuffers(1, &VBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}