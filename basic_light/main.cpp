#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

static constexpr const int WIDTH{800};
static constexpr const int HEIGHT{600};

static const glm::vec3 light_pos{1.2f, 1.0f, 2.0f};

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

    GLint success{};
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
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(cubeProgramId);

        glm::mat4 modelMat4{1.0f};
        GLint cubeModelLoc{glGetUniformLocation(cubeProgramId, "model")};
        glUniformMatrix4fv(cubeModelLoc, 1, GL_FALSE, &modelMat4[0][0]);

        glm::mat4 cubeViewMat4{1.0f};
        GLint cubeViewLoc{glGetUniformLocation(cubeProgramId, "view")};
        glUniformMatrix4fv(cubeViewLoc, 1, GL_FALSE, &cubeViewMat4[0][0]);
    }
}