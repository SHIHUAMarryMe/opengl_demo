#define GLM_FORCE_CXX14
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
	glEnable(GL_CULL_FACE);

	GLuint vertex_shader_id{ shader::create("C:\\Users\\shihua\\source\\repos\\opengl_demo\\face_culling\\face_culling\\glsl\\vertex_shader.glsl", shader_type::vertex_shader) };
	GLuint fragment_shader_id{ shader::create("C:\\Users\\shihua\\source\\repos\\opengl_demo\\face_culling\\face_culling\\glsl\\fragment_shader.glsl", shader_type::fragment_shader) };

	GLuint program_id{ glCreateProgram() };
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);
	shader::checkout_shader_state(program_id, shader_type::program);


	const float cube_vertices[]{
		// Back face
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // Bottom-left
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		 0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right         
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		// Front face
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		 0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
		// Left face
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
		// Right face
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right         
		 0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
		 0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left     
		// Bottom face
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		 0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		 0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
		// Top face
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		 0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right     
		 0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // bottom-left        
	};




	GLuint cube_VAO{};
	GLuint cube_VBO{};

	glGenVertexArrays(1, &cube_VAO);
	glGenBuffers(1, &cube_VBO);

	glBindVertexArray(cube_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, cube_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

	std::size_t offset{ 0 };
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(offset));

	offset = 3 * sizeof(float);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(offset));

	// notice that.
	glBindVertexArray(0);


	GLuint wall_texture_id{ load_texture("C:\\Users\\shihua\\source\\repos\\opengl_demo\\face_culling\\face_culling\\image\\marble.jpg") };


	glUseProgram(program_id);
	shader::set_int(program_id, "texture_1", 0);


	while (!glfwWindowShouldClose(window))
	{
		double current_time{ glfwGetTime() };
		delta_time = current_time - last_frame;
		last_frame = current_time;

		process_input(window);


		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glCullFace(GL_FRONT); // cull front faces.

		glUseProgram(program_id);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wall_texture_id);
		glm::mat4 projection{ glm::perspective(glm::radians(field_of_view), WIDTH * 1.0f / HEIGHT * 1.0f, 0.1f, 100.0f) };
		glm::mat4 view{ glm::lookAt(camera_pos, camera_pos + camera_front, camera_up) };
		glm::mat4 model{ 1.0f };
		shader::set_mat4(program_id, "projection", projection);
		shader::set_mat4(program_id, "view", view);
		shader::set_mat4(program_id, "model", model);

		glBindVertexArray(cube_VAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);



		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glDeleteVertexArrays(1, &cube_VAO);
	glDeleteBuffers(1, &cube_VBO);

	glfwTerminate();
	return 0;
}