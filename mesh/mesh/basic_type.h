#ifndef  __BASIC_TYPE_H__
#define __BASIC_TYPE_H__

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <string>
#include <vector>
#include <cstddef>

enum class texture_type
{
	ambient_type,
	diffuse_type,
	specular_type
};

struct vertex
{
	glm::vec3 position_;
	glm::vec3 normal_;
	glm::vec2 texcoord_;

	glm::vec3 tangent_;
	glm::vec3 bitangent_;
};


struct texture
{
	std::size_t id_;
	texture_type type_;

	std::basic_string<char> path_;
};



class mesh final
{
public:
	mesh() = default;
	mesh(const mesh&) = delete;
	mesh& operator=(const mesh&) = delete;

	inline void add_vertices(const std::vector<vertex>& vertices)noexcept
	{
		this->vertices_ = vertices;
	}

	inline const std::vector<vertex>& get_vertices(const std::vector<vertex>& vertices)const noexcept
	{
		return this->vertices_;
	}

	inline void add_indices(const std::vector<std::size_t>& indices)noexcept
	{
		this->indices_ = indices;
	}

	inline const std::vector<std::size_t>& get_indices()const noexcept
	{
		return this->indices_;
	}

	inline void add_textures(const std::vector<texture>& textures)noexcept
	{
		this->textures_ = textures;
	}

	const const std::vector<texture> get_textures()const noexcept
	{
		return this->textures_;
	}

	const std::size_t& get_VAO()const noexcept
	{
		return this->VAO_;
	}

	const std::size_t& get_VBO()const noexcept
	{
		return this->VBO_;
	}

	const std::size_t& get_EBO()const noexcept
	{
		return this->EBO_;
	}

	void bind_texture(std::size_t program_id)
	{
		std::size_t number_of_textures{ this->textures_.size() };

		for (std::size_t index = 0; index < number_of_textures; ++index)
		{
			const texture& ref_texture = this->textures_[index];

			if (ref_texture.type_ == texture_type::ambient_type)
			{
				glUniform1i(glGetUniformLocation(program_id, ("ambient_texture_" + std::to_string(index)).c_str()), index);
			}

			if (ref_texture.type_ == texture_type::diffuse_type)
			{
				glUniform1i(glGetUniformLocation(program_id, ("diffuse_texture_" + std::to_string(index)).c_str()), index);
			}

			if (ref_texture.type_ == texture_type::specular_type)
			{
				glUniform1i(glGetUniformLocation(program_id, ("specular_texture_" + std::to_string(index)).c_str()), index);
			}

			glActiveTexture(GL_TEXTURE0 + index);
			glBindTexture(GL_TEXTURE_2D, ref_texture.id_);
		}
	}

	void bind_VAO_VBO_EBO()
	{
		glGenVertexArrays(1, &(this->VAO_));
		glGenBuffers(1, (this->VBO_));
		glGenBuffers(1, (this->EBO_));

		glBindVertexArray(this->VAO_);

		glBindBuffer(GL_ARRAY_BUFFER, this->VBO_);
		glBufferData(GL_ARRAY_BUFFER, this->vertices_.size() * sizeof(vertex), &vertices_[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, this->EBO_);
		glBufferData(GL_ARRAY_BUFFER, this->indices_.size() * sizeof(std::size_t), &indices_[0], GL_STATIC_DRAW);

		std::size_t offset{ 0 };
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset));

		offset = offsetof(vertex, position_);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset));

		offset = offsetof(vertex, normal_);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset));

		offset = offsetof(vertex, texcoord_);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset));

		offset = offsetof(vertex, tangent_);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset));

		//notice here:
		glBindVertexArray(0);
	}
private:


	std::vector<vertex> vertices_;
	std::vector<std::size_t> indices_;
	std::vector<texture> textures_;

	GLuint VAO_;
	GLuint VBO_;
	GLuint EBO_;
};





#endif // __BASIC_TYPE_H__

