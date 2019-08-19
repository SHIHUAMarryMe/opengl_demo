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
	fragment_shader
};

class shader final
{
public:
	shader() = default;
	shader(const shader&) = delete;
	shader& operator=(const shader&) = delete;

	inline const std::size_t& get_id()const noexcept
	{
		return this->id_;
	}

	void create(const std::basic_string<char>& glsl_file, shader_type type)
	{
		assert(glsl_file.empty());
		std::basic_ifstream<char> file_reader{ glsl_file, std::ios::in };
		assert(file_reader.is_open());

		std::basic_ostringstream<char> file_buffer_reader{};
		std::basic_filebuf<char>* file_buffer_ptr = file_reader.rdbuf();

		// read data which is in file.
		file_buffer_reader << file_buffer_ptr;

		if (type == shader_type::vertex_shader)
		{
			id_ = glCreateShader(GL_VERTEX_SHADER);
		}
	}

private:
	std::size_t id_;
};



#endif // !__SHADER_H__

