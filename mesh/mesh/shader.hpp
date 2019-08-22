#ifndef __SHADER_H__
#define __SHADER_H__

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <iostream>
#include <sstream>
#include <fstream>
#include <cassert>

enum class shader_type
{
	vertex_shader,
	fragment_shader,
	program
};

class shader final
{
public:
	shader() = default;
	shader(const shader&) = delete;
	shader& operator=(const shader&) = delete;

	static std::size_t create(const std::basic_string<char>& glsl_file, shader_type type)
	{
		assert(glsl_file.empty());
		std::basic_ifstream<char> file_reader{ glsl_file, std::ios::in };
		assert(file_reader.is_open());

		std::basic_ostringstream<char> file_buffer_reader{};
		std::basic_filebuf<char>* file_buffer_ptr = file_reader.rdbuf();

		// read data which is in file.
		file_buffer_reader << file_buffer_ptr;

		std::size_t shader_id{};

		if (type == shader_type::vertex_shader)
		{
			shader_id = glCreateShader(GL_VERTEX_SHADER);
		}

		if (type == shader_type::fragment_shader)
		{
			shader_id = glCreateShader(GL_FRAGMENT_SHADER);
		}

		std::basic_string<char> shader_source{ file_buffer_reader.str() };
		glShaderSource(shader_id, 1, &(shader_source.c_str()), nullptr);
		glCompileShader(shader_id);

		if (!shader::checkout_shader_state(shader_id, type))
		{
			return 0;
		}

		return shader_id;
	}

	static void set_bool(std::size_t id, const std::string &name, bool value) noexcept
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), (int)value);
	}

	static void set_int(std::size_t id, const std::string &name, int value) noexcept
	{
		glUniform1i(glGetUniformLocation(id, name.c_str()), value);
	}

	static void set_float(std::size_t id, const std::string &name, float value) noexcept
	{
		glUniform1f(glGetUniformLocation(id, name.c_str()), value);
	}

	static void set_vec2(std::size_t id, const std::string &name, const glm::vec2 &value) noexcept
	{
		glUniform2fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}

	static void set_vec2(std::size_t id, const std::string &name, float x, float y) noexcept
	{
		glUniform2f(glGetUniformLocation(id, name.c_str()), x, y);
	}

	static void set_vec3(std::size_t id, const std::string &name, const glm::vec3 &value) noexcept
	{
		glUniform3fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	static void set_vec3(std::size_t id, const std::string &name, float x, float y, float z) noexcept
	{
		glUniform3f(glGetUniformLocation(id, name.c_str()), x, y, z);
	}

	static void set_vec4(std::size_t id, const std::string &name, const glm::vec4 &value) noexcept
	{
		glUniform4fv(glGetUniformLocation(id, name.c_str()), 1, &value[0]);
	}
	static void set_vec4(std::size_t id, const std::string &name, float x, float y, float z, float w)noexcept
	{
		glUniform4f(glGetUniformLocation(id, name.c_str()), x, y, z, w);
	}

	static void set_mat2(std::size_t id, const std::string &name, const glm::mat2 &mat) noexcept
	{
		glUniformMatrix2fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	static void set_mat3(std::size_t id, const std::string &name, const glm::mat3 &mat) noexcept
	{
		glUniformMatrix3fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	static void set_mat4(std::size_t id, const std::basic_string<char> &name, const glm::mat4 &mat)noexcept
	{
		glUniformMatrix4fv(glGetUniformLocation(id, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	static bool checkout_shader_state(std::size_t id, shader_type type)
	{
		GLint success{};
		GLchar error_log[1024]{};

		if (type == shader_type::vertex_shader || type == shader_type::fragment_shader)
		{
			glGetShaderiv(id, GL_COMPILE_STATUS, &success);

			if (!success)
			{
				glGetShaderInfoLog(id, 1024, nullptr, error_log);
				std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << "\n" << error_log << std::endl;
				return false;
			}
		}
		else
		{
			glGetProgramiv(id, GL_LINK_STATUS, &success);
			if (!success)
			{
				glGetProgramInfoLog(id, 1024, nullptr, error_log);
				std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << "\n" << error_log << std::endl;
				return false;
			}
		}

		return true;
	}

};



#endif // !__SHADER_H__

