
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "shader.hpp"

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif //__APPLE__


		// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);


	GLuint vertex_shader_id{ shader::create("C:\\Users\\y\\Documents\\Visual Studio 2017\\Projects\\opengl_demo\\geometry_shader\\geometry_shader\\glsl\\vertex_shader.glsl", shader_type::vertex_shader) };
	GLuint geometry_shader_id{ shader::create("C:\\Users\\y\\Documents\\Visual Studio 2017\\Projects\\opengl_demo\\geometry_shader\\geometry_shader\\glsl\\geometry_shader.glsl", shader_type::geometry_shader) };
	GLuint fragment_shader_id{ shader::create("C:\\Users\\y\\Documents\\Visual Studio 2017\\Projects\\opengl_demo\\geometry_shader\\geometry_shader\\glsl\\fragment_shader.glsl", shader_type::fragment_shader) };

	GLuint program_id{ glCreateProgram() };
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, geometry_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);
	glDeleteShader(geometry_shader_id);
	shader::checkout_shader_state(program_id, shader_type::program);

	const float points[]{
	-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
	 0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
	 0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
	-0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
	};

	GLuint VAO{};
	GLuint VBO{};

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof points, points, GL_STATIC_DRAW);


	GLuint offset{ 0 };
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(offset));

	offset = 2 * sizeof(float);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(offset));

	glBindVertexArray(0);

	while (!glfwWindowShouldClose(window))
	{
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(program_id);


		glBindVertexArray(VAO);
		glDrawArrays(GL_POINTS, 0, 4);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}
