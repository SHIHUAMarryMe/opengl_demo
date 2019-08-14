#define GLM_FORCE_CXX14
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "stb_image/stb_image.h"

static constexpr const int WIDTH{ 800 };
static constexpr const int HEIGHT{ 600 };

// lighting.
static const glm::vec3 light_pos{ 1.2f, 1.0f, 2.0f };

// camera
static glm::vec3 cameraPos{ 0.0f, 0.0f, 3.0f };
static glm::vec3 cameraFront{};
static glm::vec3 cameraUp{ 0.0f, 1.0f, 0.0f };
static glm::vec3 cameraRight{};
static glm::vec3 worldUp = cameraUp;
static float delta_time{};
static float last_frame{};

//mouse and scroll
static float field_of_view{ 45.0f };
static bool firstMouse{ true };
static float lastX{ WIDTH / 2.f };
static float lastY{ HEIGHT / 2.f };
static float pitch{ 0.0f };
static float yaw{ -90.f };
static float speed{ 2.5f };
static float sensitivity{ 0.05f };

static void updateCameraVectors()
{
	// Calculate the new Front vector
	glm::vec3 front{};
	front.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
	front.y = std::sin(glm::radians(pitch));
	front.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	// Also re-calculate the Right and Up vector
	cameraRight = glm::normalize(glm::cross(cameraFront, worldUp)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}

static constexpr const char *cubeVertexShaderSource{
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"layout (location = 1) in vec3 aNormal;\n"
	"layout (location = 2) in vec2 aTexCoords;\n"

	"out vec3 FragPos;\n"
	"out vec3 Normal;\n"
	"out vec2 TexCoords;\n"

	"uniform mat4 model;\n"
	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"

	"void main()\n"
	"{\n"
	"    FragPos = vec3(model * vec4(aPos, 1.0));\n"
	"    TexCoords = aTexCoords;\n"

	// must use Normal Matrix, it can keep NU Scale right.
	"    Normal = mat3(transpose(inverse(model))) * aNormal;\n" //transpose(inverse(model)) create Normal Matrix.

	"    gl_Position = projection * view * vec4(FragPos, 1.0);\n"
	"}" };
static constexpr const char *cubeFragmentShaderSource{
	"#version 330 core\n"
	"out vec4 FragColor;\n"

	"struct Material\n"
	"{\n"
	"	sampler2D diffuse_;\n"
	"	sampler2D specular_;\n"
	"	float shininess_;\n"
	"};\n"

"struct DirLight {\n"
"	vec3 direction_;\n"
"	vec3 ambient_;\n"
"	vec3 diffuse_;\n"
"	vec3 specular_;\n"
"};\n"

"struct PointLight\n"
"{\n"
"	vec3 position_;\n"

"	float constant_;\n"
"	float linear_;\n"
"	float quadratic_;\n"

"	vec3 ambient_;\n"
"	vec3 diffuse_;\n"
"	vec3 specular_;\n"
"};\n"


"struct Material\n"
"{\n"
"    sampler2D diffuse_;\n" //ambient color is the same with diffuse color, ususally.
"    sampler2D specular_;\n"
"    float shininess_;\n"
"};\n"

"struct SpotLight"
"{\n"
"vec3 position_;\n"   // bes same with camera position.
"vec3 direction_;\n" // be same with camera direction.
"float cutoff_;\n"
"float outer_cutoff_;\n"

"float constant_;\n"
"float linear_;\n"
"float quadratic_;\n"

"vec3 ambient_;\n"
"vec3 diffuse_;\n"
"vec3 specular_;\n"
"};\n"

"#define NR_POINT_LIGHTS 4\n"

"in vec3 Normal;\n"
"in vec3 FragPos;\n"
"in vec2 TexCoords;\n"

"uniform Material material;\n"
"uniform DirLight dirLight;\n" // cllimated light
"uniform PointLight pointLights[NR_POINT_LIGHTS];\n" // point lights
"uniform SpotLight spotLight;\n" // flash light

"uniform vec3 cameraPos;\n" //camera pos.

"vec3 CalcDirLight(DirLight dir_light, vec3 normal, vec3 viewer_dir);\n"
"vec3 CalcPointLight(PointLight point_light, vec3 normal, vec3 frag_pos, vec3 viewer_dir);\n"
"vec3 CalcSpotLight(SpotLight spot_light, vec3 normal, vec3 frag_pos, vec3 viewer_dir);\n"

"void main()\n"
"{\n"

// properties
"vec3 norm = normalize(Normal);\n"
"vec3 viewDir = normalize(cameraPos - FragPos);\n"

// == =====================================================
// Our lighting is set up in 3 phases: directional, point lights and an optional flashlight
// For each phase, a calculate function is defined that calculates the corresponding color
// per lamp. In the main() function we take all the calculated colors and sum them up for
// this fragment's final color.
// == =====================================================
// phase 1: directional lighting
"vec3 result = CalcDirLight(dirLight, norm, viewDir);\n"

// phase 2: point lights
"for (int i = 0; i < NR_POINT_LIGHTS; i++)\n"
"	result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);\n"

// phase 3: spot light
"result += CalcSpotLight(spotLight, norm, FragPos, viewDir);\n"

"FragColor = vec4(result, 1.0);\n"

"}\n"

"vec3 CalcDirLight(DirLight dir_light, vec3 normal, vec3 viewer_dir)"
"{\n"

	"vec3 light_direction = normalize(-dir_light.direction_);\n"

	// diffuse
	"float diffuse_value = max(dot(light_direction, normal), 0.0f);\n"

	// specular
	"vec3 reflect_direction = reflect(-light_direction, normal);\n"
	"float specular_value = pow(max(dot(viewer_dir, reflect_direction), 0.0f), material.shininess_);\n"

	"vec3 ambient = dir_light.ambient_ * vec3(texture(material.diffuse_, TexCoords));\n"
	"vec3 diffuse = dir_light.diffuse_ * diffuse_value * vec3(texture(material.diffuse_, TexCoords));\n"
	"vec3 specular = dir_light.specular_ * specular_value * vec3(texture(material.specular_, TexCoords));\n"

	"return (ambient + diffuse + specular);\n"
"}\n"

"vec3 CalcPointLight(PointLight point_light, vec3 normal, vec3 frag_pos, vec3 viewer_dir)\n"
"{\n"
	"vec3 light_direction = normalize(point_light.position_ - frag_pos);\n"

	// diffuse
	"float diffuse_value = max(dot(light_direction, normal), 0.0f);\n"

	// specular
	"vec3 reflect_direction = reflect(-light_direction, normal);\n"
	"float specular_value = pow(max(dot(viewer_dir, reflect_direction), 0.0), material.shininess_);\n"

	// attenuation
	"float distance = length(point_light.position_ - frag_pos);\n"
	"float attenuation_value = 1.0f / (point_light.constant_ + point_light.linear_ * distance + point_light.quadratic_ * (distance * distance));\n"

	"vec3 ambient = point_light.ambient_ * vec3(texture(material.diffuse_, TexCoords));\n"
	"vec3 diffuse = point_light.diffuse_ * diffuse_value * vec3(texture(material.diffuse_, TexCoords));\n"
	"vec3 specular = point_light.specular_ * specular_value * vec3(texture(material.specular_, TexCoords));\n"

	"ambient *= attenuation_value;\n"
	"diffuse *= attenuation_value;\n"
	"specular *= attenuation_value;\n"
	"return (ambient + diffuse + specular);\n"
"}\n"

// calculates the color when using a spot light.
"vec3 CalcSpotLight(SpotLight spot_light, vec3 normal, vec3 frag_pos, vec3 viewer_dir)\n"
"{\n"
	"vec3 light_direction = normalize(spot_light.position_ - frag_pos);\n"

	// diffuse shading
	"float diff = max(dot(normal, light_direction), 0.0);\n"

	// specular shading
	"vec3 reflect_direction = reflect(-light_direction, normal);\n"
	"float spec = pow(max(dot(viewer_dir, reflect_direction), 0.0), material.shininess_);\n"

	// attenuation
	"float distance = length(spot_light.position_ - frag_pos);\n"
	"float attenuation = 1.0f / (spot_light.constant_ + spot_light.linear_ * distance + spot_light.quadratic_ * (distance * distance));\n"

	// spotlight intensity
	"float theta = dot(light_direction, normalize(-spot_light.direction_));\n"
	"float epsilon = spot_light.cutoff_ - spot_light.outer_cutoff_;\n"
	"float intensity = clamp((theta - spot_light.outer_cutoff_) / epsilon, 0.0, 1.0);\n"

	// combine results
	"vec3 ambient = spot_light.ambient_ * vec3(texture(material.diffuse_, TexCoords));\n"
	"vec3 diffuse = spot_light.diffuse_ * diff * vec3(texture(material.diffuse_, TexCoords));\n"
	"vec3 specular = spot_light.specular_ * spec * vec3(texture(material.specular_, TexCoords));\n"

	"ambient *= attenuation * intensity;\n"
	"diffuse *= attenuation * intensity;\n"
	"specular *= attenuation * intensity;\n"

	"return (ambient + diffuse + specular);\n"
"}\n"
};

static constexpr const char *lampVertexShaderSource{
	"#version 330 core\n"
	"layout (location = 0) in vec3 aPos;\n"
	"uniform mat4 model;\n"
	"uniform mat4 view;\n"
	"uniform mat4 projection;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
	"}" };

static constexpr const char *lampFragmentShaderSource{
	"#version 330 core\n"
	"out vec4 FragColor;\n"
	"void main()\n"
	"{\n"
	"    FragColor = vec4(1.0);\n" // set alle 4 vector values to 1.0
	"}" };

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
static void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float camera_speed{ speed * delta_time };

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
		cameraPos -= cameraRight * camera_speed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += cameraRight * camera_speed;
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

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	updateCameraVectors();
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
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "LearnOpenGL", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

	// notice that: count camera front/up/right vectors.
	updateCameraVectors();

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
		glGetShaderInfoLog(cubeVertexShader, 1024, nullptr, infoLog);
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

	GLuint cubeProgramId{ glCreateProgram() };
	glAttachShader(cubeProgramId, cubeVertexShader);
	glAttachShader(cubeProgramId, cubeFragmentShader);
	glLinkProgram(cubeProgramId);

	success = 0;
	glGetProgramiv(cubeProgramId, GL_LINK_STATUS, &success);
	if (!success)
	{
		GLchar infoLog[1024]{};
		glGetProgramInfoLog(cubeProgramId, 1024, nullptr, infoLog);
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
		glGetShaderInfoLog(lampVertexShader, 1024, nullptr, infoLog);
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

	GLuint lampProgramId{ glCreateProgram() };
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
		// positions          // normals           // texture coords
-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	// positions all containers
	const glm::vec3 cubePositions[]{
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(2.0f, 5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f, 3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f, 2.0f, -2.5f),
		glm::vec3(1.5f, 0.2f, -1.5f),
		glm::vec3(-1.3f, 1.0f, -1.5f) };

	// positions of the point lights
	const glm::vec3 pointLightPositions[]{
		glm::vec3(0.7f,  0.2f,  2.0f),
		glm::vec3(2.3f, -3.3f, -4.0f),
		glm::vec3(-4.0f,  2.0f, -12.0f),
		glm::vec3(0.0f,  0.0f, -3.0f)
	};

	GLuint VBO{};
	GLuint cubeVAO{};
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLint offset{ 0 };
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(offset));
	glEnableVertexAttribArray(0);

	offset = 3 * sizeof(float);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(offset));
	glEnableVertexAttribArray(1);

	offset = 6 * sizeof(float);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(offset));
	glEnableVertexAttribArray(2);

	GLuint lampVAO{};
	glGenVertexArrays(1, &lampVAO);

	glBindVertexArray(lampVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	offset = 0;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), reinterpret_cast<void *>(offset));
	glEnableVertexAttribArray(0);

	GLuint textureID{};
	glGenTextures(1, &textureID);

	int width{}, height{}, nrComponents{};
	unsigned char *data{ stbi_load("C:\\Users\\shihua\\source\\repos\\opengl_demo\\lightcaster-flashlight\\lightcaster-flashlight\\image\\container2.png",
								  &width, &height, &nrComponents, 0) };

	if (data)
	{
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set texture wrapper.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// set texture filter.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "the path of image is wrong." << std::endl;
		stbi_image_free(data);
	}

	data = nullptr;

	GLuint textureID2{};
	glGenTextures(1, &textureID2);
	width = 0;
	height = 0;
	nrComponents = 0;

	data = stbi_load("C:\\Users\\shihua\\source\\repos\\opengl_demo\\lightcaster-flashlight\\lightcaster-flashlight\\image\\container2_specular.png",
		&width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID2);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set texture wrapper.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// set texture filter.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cerr << "the path of image is wrong." << std::endl;
		stbi_image_free(data);
	}

	data = nullptr;

	glUseProgram(cubeProgramId);
	GLint texture1Id{ glGetUniformLocation(cubeProgramId, "material.diffuse_") };
	glUniform1i(texture1Id, 0);
	GLint texture2Id{ glGetUniformLocation(cubeProgramId, "material.specular_") };
	glUniform1i(texture2Id, 1);

	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		float currentFrame = glfwGetTime();
		delta_time = currentFrame - last_frame;
		last_frame = currentFrame;

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// input
		// -----
		processInput(window);

		glUseProgram(cubeProgramId);

		// bind textures to specify uniform.
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureID);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureID2);

		GLint viewPosLoc{ glGetUniformLocation(cubeProgramId, "cameraPos") };
		glUniform3fv(viewPosLoc, 1, &cameraPos[0]);

		// directional light
		GLint dirLightDirectionLoc{ glGetUniformLocation(cubeProgramId, "dirLight.direction_") };
		glUniform3f(dirLightDirectionLoc, -0.2f, -1.0f, -0.3f);

		GLint dirLightAmbientLoc{ glGetUniformLocation(cubeProgramId, "dirLight.ambient_") };
		glUniform3f(dirLightAmbientLoc, 0.05f, 0.05f, 0.05f);

		GLint dirLightDiffuseLoc{ glGetUniformLocation(cubeProgramId, "dirLight.diffuse_") };
		glUniform3f(dirLightDiffuseLoc, 0.4f, 0.4f, 0.4f);

		GLint dirLightSpecularLoc{ glGetUniformLocation(cubeProgramId, "dirLight.specular_") };
		glUniform3f(dirLightSpecularLoc, 0.5f, 0.5f, 0.5f);

		// point light 1
		glUniform3fv(glGetUniformLocation(cubeProgramId, "pointLights[0].position_"), 1, &pointLightPositions[0][0]);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[0].ambient_"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[0].diffuse_"), 0.8f, 0.8f, 0.8f);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[0].specular_"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[0].constant_"), 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[0].linear_"), 0.09f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[0].quadratic_"), 0.032f);
		// point light 2
		glUniform3fv(glGetUniformLocation(cubeProgramId, "pointLights[1].position_"), 1, &pointLightPositions[1][0]);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[1].ambient_"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[1].diffuse_"), 0.8f, 0.8f, 0.8f);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[1].specular_"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[1].constant_"), 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[1].linear_"), 0.09f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[1].quadratic_"), 0.032);
		// point light 3
		glUniform3fv(glGetUniformLocation(cubeProgramId, "pointLights[2].position_"), 1, &pointLightPositions[2][0]);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[2].ambient_"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[2].diffuse_"), 0.8f, 0.8f, 0.8f);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[2].specular_"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[2].constant_"), 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[2].linear_"), 0.09f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[2].quadratic_"), 0.032f);
		// point light 4
		glUniform3fv(glGetUniformLocation(cubeProgramId, "pointLights[3].position_"), 1, &pointLightPositions[1][0]);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[3].ambient_"), 0.05f, 0.05f, 0.05f);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[3].diffuse_"), 0.8f, 0.8f, 0.8f);
		glUniform3f(glGetUniformLocation(cubeProgramId, "pointLights[3].specular_"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[3].constant_"), 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[3].linear_"), 0.09f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "pointLights[3].quadratic_"), 0.32f);
		// spotLight
		glUniform3fv(glGetUniformLocation(cubeProgramId, "spotLight.position_"), 1, &cameraPos[0]);
		glUniform3fv(glGetUniformLocation(cubeProgramId, "spotLight.direction_"), 1, &cameraFront[0]);
		glUniform3f(glGetUniformLocation(cubeProgramId, "spotLight.ambient_"), 0.0f, 0.0f, 0.0f);
		glUniform3f(glGetUniformLocation(cubeProgramId, "spotLight.diffuse_"), 1.0f, 1.0f, 1.0f);
		glUniform3f(glGetUniformLocation(cubeProgramId, "spotLight.specular_"), 1.0f, 1.0f, 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "spotLight.constant_"), 1.0f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "spotLight.linear_"), 0.09f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "spotLight.quadratic_"), 0.032f);
		glUniform1f(glGetUniformLocation(cubeProgramId, "spotLight.cutoff_"), std::cos(glm::radians(12.5f)));
		glUniform1f(glGetUniformLocation(cubeProgramId, "spotLight.outer_cutoff_"), std::cos(glm::radians(15.0f)));

		GLint materialShininessLoc{ glGetUniformLocation(cubeProgramId, "material.shininess_") };
		glUniform1f(materialShininessLoc, 32.0f);

		// view/projection transformations
		glm::mat4 projection{ glm::perspective(glm::radians(field_of_view), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f) };
		glUniformMatrix4fv(glGetUniformLocation(cubeProgramId, "projection"), 1, GL_FALSE, &projection[0][0]);

		// camera/view transformation
		glm::mat4 view{ 1.0f }; // make sure to initialize matrix to identity matrix first
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glUniformMatrix4fv(glGetUniformLocation(cubeProgramId, "view"), 1, GL_FALSE, &view[0][0]);

		// model
		glm::mat4 model{ 1.0f };
		glUniformMatrix4fv(glGetUniformLocation(cubeProgramId, "model"), 1, GL_FALSE, &model[0][0]);

		// render containers
		glBindVertexArray(cubeVAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model{ 1.0f };
			model = glm::translate(model, cubePositions[i]);
			float angle{ 20.0f * i };
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			glUniformMatrix4fv(glGetUniformLocation(cubeProgramId, "model"), 1, GL_FALSE, &model[0][0]);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// lamp
		glUseProgram(lampProgramId);
		glUniformMatrix4fv(glGetUniformLocation(lampProgramId, "projection"), 1, GL_FALSE, &projection[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(lampProgramId, "view"), 1, GL_FALSE, &view[0][0]);

		// we now draw as many light bulbs as we have point lights.
		glBindVertexArray(lampVAO);
		for (std::size_t i = 0; i < 4; ++i)
		{
			model = glm::mat4{ 1.0f };
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			glUniformMatrix4fv(glGetUniformLocation(lampProgramId, "model"), 1, GL_FALSE, &model[0][0]);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

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