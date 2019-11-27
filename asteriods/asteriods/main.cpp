#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/mesh.h>


#define GLM_FORCE_CXX14
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <atomic>
#include <random>
#include <iostream>


#include "shader.hpp"
#include "model.hpp"



static constexpr const int WIDTH{ 800 };
static constexpr const int HEIGHT{ 600 };

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
	// Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
	camera_right = glm::normalize(glm::cross(camera_front, world_up));
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
	glEnable(GL_DEPTH_TEST);

	GLuint asteriods_vertex_shader_id{ shader::create("C:\\Users\\y\\Documents\\Visual Studio 2017\\Projects\\opengl_demo\\asteriods\\asteriods\\glsl\\vertex_shader.glsl", shader_type::vertex_shader) };
	GLuint asteriods_fragment_shader_id{ shader::create("C:\\Users\\y\\Documents\\Visual Studio 2017\\Projects\\opengl_demo\\asteriods\\asteriods\\glsl\\fragment_shader.glsl", shader_type::fragment_shader) };

	GLuint asteriods_gl_program_id{ glCreateProgram() };
	glAttachShader(asteriods_gl_program_id, asteriods_vertex_shader_id);
	glAttachShader(asteriods_gl_program_id, asteriods_fragment_shader_id);
	glLinkProgram(asteriods_gl_program_id);
	shader::checkout_shader_state(asteriods_gl_program_id, shader_type::program);


	GLuint planet_vertex_shader_id{ shader::create("C:\\Users\\y\\Documents\\Visual Studio 2017\\Projects\\opengl_demo\\asteriods\\asteriods\\glsl\\vertex_shader.glsl", shader_type::vertex_shader) };
	GLuint planet_fragment_shader_id{ shader::create("C:\\Users\\y\\Documents\\Visual Studio 2017\\Projects\\opengl_demo\\asteriods\\asteriods\\glsl\\fragment_shader.glsl", shader_type::fragment_shader) };
	GLuint planet_gl_program_id{ glCreateProgram() };
	glAttachShader(planet_fragment_shader_id, planet_vertex_shader_id);
	glAttachShader(planet_fragment_shader_id, planet_fragment_shader_id);



	std::unique_ptr<model_loader> loaded_planet{ std::make_unique<model_loader>() };
	loaded_planet->load_model("C:\\Users\\y\\Documents\\Visual Studio 2017\\Projects\\opengl_demo\\asteriods\\asteriods\\model_file\\planet\\planet.obj");
	loaded_planet->load_vertices_data();

	std::unique_ptr<model_loader> loaded_rock{ std::make_unique<model_loader>() };
	loaded_rock->load_model("C:\\Users\\y\\Documents\\Visual Studio 2017\\Projects\\opengl_demo\\asteriods\\asteriods\\model_file\\planet\\rock.obj");
	loaded_rock->load_vertices_data();



	// generate a large list of semi-random model transformation matrices
	// ------------------------------------------------------------------
	std::size_t amount{ 1000 };
	std::unique_ptr<glm::mat4[]> model_matrices{ new glm::mat4[amount]{} };

	std::random_device random_device{};
	std::mt19937 generator{ random_device() };
	std::uniform_int_distribution<> distribution{ 0, static_cast<long int>(glfwGetTime()) };

	float radius{ 50.0 };
	float offset{ 2.5f };
	for (std::size_t index = 0; index < amount; ++index)
	{
		glm::mat4 model = glm::mat4(1.0f);
		// 1. translation: displace along circle with 'radius' in range [-offset, offset]
		float angle{ index * 1.0f / amount * 360.0f };
		float displacement{ (distribution(generator) % (int)(2 * offset * 100)) / 100.0f - offset };
		float x{ std::sin(angle) * radius + displacement };
		displacement = (distribution(generator) % (int)(2 * offset * 100)) / 100.0f - offset;
		float y{ displacement * 0.4f }; // keep height of asteroid field smaller compared to width of x and z
		displacement = (distribution(generator) % (int)(2 * offset * 100)) / 100.0f - offset;
		float z{ std::cos(angle) * radius + displacement };
		model = glm::translate(model, glm::vec3(x, y, z));

		// 2. scale: Scale between 0.05 and 0.25f
		float scale{ static_cast<float>((distribution(generator) % 20) / 100.0f + 0.05) };
		model = glm::scale(model, glm::vec3(scale));

		// 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
		float rotAngle = (rand() % 360);
		model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

		// 4. now add to list of matrices
		model_matrices[index] = model;
	}


	// configure instanced array
	GLuint instanced_VBO{};
	glGenBuffers(1, &instanced_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanced_VBO);
	glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), model_matrices.get(), GL_STATIC_DRAW);

	const auto& meshes_in_rock{ loaded_rock->get_meshes() };
	auto rock_meshed_itr{ meshes_in_rock.cbegin() };
	for (; rock_meshed_itr != meshes_in_rock.cend(); ++rock_meshed_itr)
	{
		std::size_t mesh_VAO{ (*rock_meshed_itr)->get_VAO() };
		glBindVertexArray(mesh_VAO);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(0));

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(sizeof(glm::mat4)));

		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(2 * sizeof(glm::mat4)));

		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), reinterpret_cast<void*>(3 * sizeof(glm::mat4)));



		glBindVertexArray(0);
	}





	while (!glfwWindowShouldClose(window))
	{
		double current_time{ glfwGetTime() };
		delta_time = current_time - last_frame;
		last_frame = current_time;


		// process keyboard events.
		process_input(window);

		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		glm::mat4 projection{ glm::perspective(glm::radians(45.0f), (WIDTH / HEIGHT)*1.0f, 1.0f, 100.0f) };
		glm::mat4 view{ 1.0f }; // make sure to initialize matrix to identity matrix first
		view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);

		glUseProgram(asteriods_gl_program_id);
		shader::set_mat4(asteriods_gl_program_id, "projection", projection);
		shader::set_mat4(asteriods_gl_program_id, "view", view);

		glUseProgram(planet_gl_program_id);
		shader::set_mat4(planet_gl_program_id, "projection", projection);
		shader::set_mat4(planet_gl_program_id, "view", view);

		// draw planet
		glm::mat4 model{ 1.0f };
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.0f, 4.0f, 4.0f));
		shader::set_mat4(planet_gl_program_id, "model", model);
		loaded_planet->draw(planet_gl_program_id);


		// draw asteriod
		glUseProgram(asteriods_gl_program_id);
		shader::set_int(asteriods_gl_program_id, "texture_diffuse_1", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, loaded_planet->get_loaded_textures()[0].second.id_);

		std::size_t no_meshed_in_asteroid{ loaded_planet->get_loaded_textures().size() };
		for (auto meshed_in_rock_itr_beg = loaded_planet->get_meshes().cbegin(); 
			meshed_in_rock_itr_beg != loaded_planet->get_meshes().cend(); ++meshed_in_rock_itr_beg)
		{
			glBindVertexArray((*meshed_in_rock_itr_beg)->get_VAO());
			glDrawElementsInstanced(GL_TRIANGLES, (*meshed_in_rock_itr_beg)->get_indices().size(), GL_UNSIGNED_INT, 0, amount);

			glBindVertexArray(0);
		}



		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}
