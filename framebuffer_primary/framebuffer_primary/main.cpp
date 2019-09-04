#define GLM_FORCE_CXX14
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <atomic>
#include <iostream>

#include "shader.hpp"
#include "stb_image/stb_image.h"

static  const int WIDTH{ 800 };
static  const int HEIGHT{ 600 };

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


	GLuint vertex_shader_id{ shader::create("C:\\Users\\shihua\\source\\repos\\opengl_demo\\framebuffer_primary\\framebuffer_primary\\glsl\\vertex_shader.glsl", shader_type::vertex_shader) };
	GLuint fragment_shader_id{ shader::create("C:\\Users\\shihua\\source\\repos\\opengl_demo\\framebuffer_primary\\framebuffer_primary\\glsl\\fragment_shader.glsl", shader_type::fragment_shader) };

	GLuint program_id{ glCreateProgram() };
	glAttachShader(program_id, vertex_shader_id);
	glAttachShader(program_id, fragment_shader_id);
	glLinkProgram(program_id);
	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);
	shader::checkout_shader_state(program_id, shader_type::program);


	GLuint quad_vertex_shader_id{ shader::create("C:\\Users\\shihua\\source\\repos\\opengl_demo\\framebuffer_primary\\framebuffer_primary\\glsl\\quad_framebuffer_vertex_shader.glsl", shader_type::vertex_shader) };
	GLuint quad_fragment_shader_id{ shader::create("C:\\Users\\shihua\\source\\repos\\opengl_demo\\framebuffer_primary\\framebuffer_primary\\glsl\\quad_framebuffer_fragment_shader.glsl", shader_type::fragment_shader) };

	GLuint quad_program_id{ glCreateProgram() };
	glAttachShader(quad_program_id, quad_vertex_shader_id);
	glAttachShader(quad_program_id, quad_fragment_shader_id);
	glLinkProgram(quad_program_id);
	glDeleteShader(quad_vertex_shader_id);
	glDeleteShader(quad_fragment_shader_id);
	shader::checkout_shader_state(quad_program_id, shader_type::program);



	// set up vertex data (and buffer(s)) and configure vertex attributes
// ------------------------------------------------------------------
	const float cubes_vertices[]{
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

	const float floor_vertices[]{
		// positions          // texture Coords 
		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f,  5.0f,  0.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,

		 5.0f, -0.5f,  5.0f,  2.0f, 0.0f,
		-5.0f, -0.5f, -5.0f,  0.0f, 2.0f,
		 5.0f, -0.5f, -5.0f,  2.0f, 2.0f
	};


	const float quad_vertices[]{ // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};


	GLuint cubes_VAO{};
	GLuint cubes_VBO{};

	glGenVertexArrays(1, &cubes_VAO);
	glGenBuffers(1, &cubes_VBO);

	glBindVertexArray(cubes_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, cubes_VBO);

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


	// transparent VAO
	GLuint quad_VAO{}, quad_VBO{};
	glGenVertexArrays(1, &quad_VAO);
	glGenBuffers(1, &quad_VBO);

	glBindVertexArray(quad_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);

	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	offset = 0;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(offset));

	offset = 2 * sizeof(float);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(offset));

	// notice that.
	glBindVertexArray(0);


	GLuint wall_texture_id{ load_texture("C:\\Users\\shihua\\source\\repos\\opengl_demo\\framebuffer_primary\\framebuffer_primary\\image\\container.jpg") };
	GLuint floor_texture_id{ load_texture("C:\\Users\\shihua\\source\\repos\\opengl_demo\\framebuffer_primary\\framebuffer_primary\\image\\metal.png") };


	glUseProgram(program_id);
	shader::set_int(program_id, "texture_1", 0);


	glUseProgram(quad_program_id);
	shader::set_int(quad_fragment_shader_id, "screen_texture", 0);

	// generate framebuffer.
	GLuint framebuffer_id{};
	glGenFramebuffers(1, &framebuffer_id);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);

	GLuint color_texture_buffer_id{};
	glGenTextures(1, &color_texture_buffer_id);
	glBindTexture(GL_TEXTURE_2D, color_texture_buffer_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_texture_buffer_id, 0);

	GLuint render_buffer_id{};
	glGenRenderbuffers(1, &render_buffer_id);
	glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_id);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WIDTH, HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer_id); // now actually attach it

		// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0); // set our defined framebuffer as default.


	while (!glfwWindowShouldClose(window))
	{
		double current_time{ glfwGetTime() };
		delta_time = current_time - last_frame;
		last_frame = current_time;

		process_input(window);

		// bind to framebuffer and draw scene as we normally would to color texture 
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
		glEnable(GL_DEPTH_TEST); // enable depth testing;
													// disable depth testing before render screen-space quad.

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glUseProgram(program_id);
		glm::mat4 model{ 1.0f };
		glm::mat4 view{ glm::lookAt(camera_pos, camera_pos + camera_front, camera_up) };
		glm::mat4 projection{ glm::perspective(glm::radians(field_of_view), WIDTH * 1.0f / HEIGHT * 1.0f, 0.1f, 100.0f) };
		shader::set_mat4(program_id, "view", view);
		shader::set_mat4(program_id, "projection", projection);

		// cubes
		glBindVertexArray(cubes_VAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wall_texture_id);
		model = glm::translate(model, glm::vec3(-1.0f, 0.0f, -1.0f));
		shader::set_mat4(program_id, "model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		model = glm::mat4{ 1.0f };
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
		shader::set_mat4(program_id, "model", model);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// floor
		glBindVertexArray(floor_VAO);
		glBindTexture(GL_TEXTURE_2D, floor_texture_id);
		shader::set_mat4(program_id, "model", model);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glBindVertexArray(0);

		// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST); // disable depth test so screen-space quad isn't discarded due to depth test.
		// clear all relevant buffers
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // set clear color to white (not really necessery actually, since we won't be able to see behind the quad anyways)
		glClear(GL_COLOR_BUFFER_BIT);


		glUseProgram(quad_program_id);
		glBindVertexArray(quad_VAO);
		glBindTexture(GL_TEXTURE_2D, color_texture_buffer_id);	// use the color attachment texture as the texture of the quad plane
		glDrawArrays(GL_TRIANGLES, 0, 6);


		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glDeleteVertexArrays(1, &cubes_VAO);
	glDeleteVertexArrays(1, &floor_VAO);
	glDeleteVertexArrays(1, &quad_VAO);
	glDeleteBuffers(1, &cubes_VBO);
	glDeleteBuffers(1, &floor_VBO);
	glDeleteBuffers(1, &quad_VBO);

	glfwTerminate();
	return 0;
}