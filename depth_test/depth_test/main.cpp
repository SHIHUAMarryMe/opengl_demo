#define GLM_FORCE_CXX17
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <atomic>
#include <iostream>

#include "shader.hpp"
#include "stb_image/stb_image.h"

static  const int WIDTH{ 1280 };
static  const int HEIGHT{ 720 };

// lighting.
static const glm::vec3 light_pos{ 1.2f, 1.0f, 2.0f };

// camera
static glm::vec3 camera_pos{ 0.0f, 0.0f, 3.0f };
static glm::vec3 camera_front{};
static glm::vec3 camera_up{ 0.0f, 1.0f, 0.0f };
static glm::vec3 camera_right{};
static glm::vec3 world_up = camera_up;
static float delta_time{};
static float last_frame{};

//mouse and scroll
static float field_of_view{ 45.0f };
static std::atomic<bool> first_click{ true };
static float lastX{ WIDTH / 2.f };
static float lastY{ HEIGHT / 2.f };
static float pitch{ 0.0f };
static float yaw{ -90.f };
static float speed{ 2.5f };
static float sensitivity{ 0.05f };

static void update_camera_vectors()
{
	// Calculate the new Front vector
	glm::vec3 front{};
	front.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
	front.y = std::sin(glm::radians(pitch));
	front.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
	camera_front = glm::normalize(front);

	// Also re-calculate the Right and Up vector
	camera_right = glm::normalize(glm::cross(camera_front, world_up)); // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	camera_up = glm::normalize(glm::cross(camera_right, camera_front));
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
static void process_input(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float camera_speed{ speed * delta_time };

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		camera_pos += (camera_front * camera_speed);
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		camera_pos -= camera_speed * camera_front;
	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		camera_pos -= camera_right * camera_speed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		camera_pos += camera_right * camera_speed;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
static void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
static void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	if (first_click.load())
	{
		lastX = xpos;
		lastY = ypos;
		first_click.store(false);
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

	update_camera_vectors();
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


static GLuint load_texture(const char * path)
{
	GLuint texture_id{};
	glGenTextures(1, &texture_id);

	int width{}, height{}, nrComponents{};
	unsigned char *data{ stbi_load(path, &width, &height, &nrComponents, 0) };

	if (data)
	{
		GLenum format{};
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, texture_id);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << __FUNCTION__ << "   " << __LINE__ << "  " << "Texture failed to load at path : " << path << std::endl;
		stbi_image_free(data);
	}

	data = nullptr;
	return texture_id;
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

	// update camera parameters.
	update_camera_vectors();

	// configure global opengl state
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xff);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);


	GLuint cube_floor_vertex_shader_id{ shader::create(R"R(C:\\Users\\87869\\source\\repos\\opengl_demo\\depth_test\\depth_test\\glsl\\stencil_testing_vertex_shader.glsl)R", shader_type::vertex_shader) };
	GLuint cube_floor_fragment_shader_id{ shader::create(R"R(C:\\Users\\87869\\source\\repos\\opengl_demo\\depth_test\\depth_test\\glsl\\stencil_testing_fragment_shader.glsl)R", shader_type::fragment_shader) };

	GLuint cube_floor_program_id{ glCreateProgram() };
	glAttachShader(cube_floor_program_id, cube_floor_vertex_shader_id);
	glAttachShader(cube_floor_program_id, cube_floor_fragment_shader_id);
	glLinkProgram(cube_floor_program_id);
	glDeleteShader(cube_floor_vertex_shader_id);
	glDeleteShader(cube_floor_fragment_shader_id);
	shader::checkout_shader_state(cube_floor_program_id, shader_type::program);

	GLuint edge_vertex_shader_id{ shader::create(R"R(C:\\Users\\87869\\source\\repos\\opengl_demo\\depth_test\\depth_test\\glsl\\edge_vertex_shader.glsl)R", shader_type::vertex_shader) };
	GLuint edge_fragment_shader_id{ shader::create(R"R(C:\\Users\\87869\\source\\repos\\opengl_demo\\depth_test\\depth_test\\glsl\\edge_fragment_shader.glsl)R", shader_type::fragment_shader) };

	GLuint edge_program_id{ glCreateProgram() };
	glAttachShader(edge_program_id, edge_vertex_shader_id);
	glAttachShader(edge_program_id, edge_fragment_shader_id);
	glLinkProgram(edge_program_id);
	glDeleteShader(edge_vertex_shader_id);
	glDeleteShader(edge_fragment_shader_id);
	shader::checkout_shader_state(edge_program_id, shader_type::program);


	const float floor_vertices[]{
		// positions          // texture Coords (note we set these higher than 1 (together with GL_REPEAT as texture wrapping mode). this will cause the floor texture to repeat)
		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,  2.0f, 2.0f
	};

	float cubes_vertices[]{
		// positions          // texture Coords
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};


	GLuint cube_VAO{};
	GLuint cube_VBO{};

	glGenVertexArrays(1, &cube_VAO);
	glGenBuffers(1, &cube_VBO);

	glBindVertexArray(cube_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cubes_vertices), cubes_vertices, GL_STATIC_DRAW);

	std::size_t offset{ 0 };
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(offset));

	offset = 3 * sizeof(float);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(offset));

	// notice that.
	glBindVertexArray(0);

	GLuint floor_VAO{};
	GLuint floor_VBO{};

	glGenVertexArrays(1, &floor_VAO);
	glGenBuffers(1, &floor_VBO);

	glBindVertexArray(floor_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, floor_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(floor_vertices), floor_vertices, GL_STATIC_DRAW);

	offset = 0;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(offset));

	offset = 3 * sizeof(float);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(offset));

	// notice that
	glBindVertexArray(0);


	GLuint wall_texture_id{ load_texture("C:\\Users\\87869\\source\\repos\\opengl_demo\\depth_test\\depth_test\\image\\marble.jpg") };
	GLuint floor_texture_id{ load_texture("C:\\Users\\87869\\source\\repos\\opengl_demo\\depth_test\\depth_test\\image\\metal.png") };

	glUseProgram(cube_floor_program_id);
	shader::set_int(cube_floor_program_id, "texture_1", 0);


	while (!glfwWindowShouldClose(window))
	{
		double current_time{ glfwGetTime() };
		delta_time = current_time - last_frame;
		last_frame = current_time;

		process_input(window);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		glUseProgram(edge_program_id);
		glm::mat4 model{ 1.0f };
		glm::mat4 view{ glm::lookAt(camera_pos, camera_pos + camera_front, camera_up) };
		glm::mat4 projection{ glm::perspective(glm::radians(field_of_view), WIDTH*1.0f / HEIGHT * 1.0f, 0.1f, 100.0f) };
		shader::set_mat4(edge_program_id, "view", view);
		shader::set_mat4(edge_program_id, "projection", projection);

		// initialize uniform in cube_floor_program.
		glUseProgram(cube_floor_program_id);
		shader::set_mat4(cube_floor_program_id, "view", view);
		shader::set_mat4(cube_floor_program_id, "projection", projection);

		// draw floor by  cube_floor_program.
		glStencilMask(0x00); // don't write the floor to the stencil buffer

		glBindVertexArray(floor_VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floor_texture_id);
		model = glm::mat4{ 1.0f };
		shader::set_mat4(cube_floor_program_id, "model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);

		// 1------------------------------------------
		// draw cubes which are normal and write stencil testing value to stencil buffer.
		glStencilFunc(GL_ALWAYS, 1, 0xff);
		glStencilMask(0xff);

		glBindVertexArray(cube_VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wall_texture_id);

		// draw first cube.
		model = glm::mat4{ 1.0f };
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		shader::set_mat4(cube_floor_program_id, "model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		//draw second cube.
		model = glm::mat4{ 1.0f };
		model = glm::translate(model, glm::vec3{ 2.0f, 0.0f, 0.0f });
		shader::set_mat4(cube_floor_program_id, "model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// 1------------------------------------------------------ -



			// 2------------------------------------------------------
		// draw slightly scaled cubes
		glStencilFunc(GL_NOTEQUAL, 1, 0xff);
		glStencilMask(0x00);
		glDisable(GL_DEPTH_TEST);
		glUseProgram(edge_program_id);
		float scale{ 1.1f };

		glBindVertexArray(cube_VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wall_texture_id);
		model = glm::mat4{ 1.0f };
		model = glm::translate(model, glm::vec3{ -1.0f, 0.0f, -1.0f });
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		shader::set_mat4(edge_program_id, "model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4{ 1.0f };
		model = glm::translate(model, glm::vec3{ 2.0f, 0.0f, 0.0f });
		model = glm::scale(model, glm::vec3(scale, scale, scale));
		shader::set_mat4(edge_program_id, "model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);
		glStencilMask(0xFF);
		glEnable(GL_DEPTH_TEST);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glDeleteVertexArrays(1, &cube_VAO);
	//glDeleteVertexArrays(1, &floor_VAO);
	glDeleteBuffers(1, &cube_VBO);
	//glDeleteBuffers(1, &floor_VBO);
	glfwTerminate();
	return 0;
}