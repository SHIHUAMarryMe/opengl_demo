#ifndef  __BASIC_TYPE_H__
#define __BASIC_TYPE_H__

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <list>
#include <string>
#include <vector>
#include <cstddef>

enum class texture_type
{
	ambient_type,
	diffuse_type,
	specular_type,
	height_type
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
};



class mesh final
{
private:
	std::vector<vertex> vertices_;
	std::vector<GLuint> indices_;
	std::list<texture> textures_;

	GLuint VAO_;
	GLuint VBO_;
	GLuint EBO_;

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

	inline void add_indices(const std::vector<GLuint>& indices)noexcept
	{
		this->indices_ = indices;
	}

	inline const std::vector<GLuint>& get_indices()const noexcept
	{
		return this->indices_;
	}

	inline void add_textures(const std::list<texture>& textures)noexcept
	{
		this->textures_ = textures;
	}

	const const std::list<texture> get_textures()const noexcept
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
		auto texture_itr_beg{ this->textures_.cbegin() };

		std::size_t diffuse_no{ 1 };
		std::size_t specular_no{ 1 };
		std::size_t ambient_no{ 1 };
		std::size_t height_no{ 1 };

		for (std::size_t index = 0; index < number_of_textures; ++index)
		{
			const texture& ref_texture = *texture_itr_beg;

			if (ref_texture.type_ == texture_type::ambient_type)
			{
				glUniform1i(glGetUniformLocation(program_id, ("ambient_texture_" + std::to_string(ambient_no++)).c_str()), index);
			}

			if (ref_texture.type_ == texture_type::diffuse_type)
			{
				glUniform1i(glGetUniformLocation(program_id, ("diffuse_texture_" + std::to_string(diffuse_no++)).c_str()), index);
			}

			if (ref_texture.type_ == texture_type::specular_type)
			{
				glUniform1i(glGetUniformLocation(program_id, ("specular_texture_" + std::to_string(specular_no++)).c_str()), index);
			}

			if (ref_texture.type_ == texture_type::height_type)
			{
				glUniform1i(glGetUniformLocation(program_id, ("height_texture_" + std::to_string(height_no++)).c_str()), index);

			}

			glActiveTexture(GL_TEXTURE0 + index);
			glBindTexture(GL_TEXTURE_2D, ref_texture.id_);

			++texture_itr_beg;
		}

		// draw mesh
		glBindVertexArray(VAO_);
		glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);

		// always good practice to set everything back to defaults once configured.
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);

	}

	void bind_VAO_VBO_EBO()
	{
		glGenVertexArrays(1, &(VAO_));
		glGenBuffers(1, &VBO_);
		glGenBuffers(1, &EBO_);

		glBindVertexArray(VAO_);

		glBindBuffer(GL_ARRAY_BUFFER, VBO_);
		glBufferData(GL_ARRAY_BUFFER, vertices_.size() * sizeof(vertex), &vertices_[0], GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(GLuint), &indices_[0], GL_STATIC_DRAW);

		std::size_t offset{ 0 };
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset));

		offset = offsetof(vertex, normal_);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset));

		offset = offsetof(vertex, texcoord_);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset));

		offset = offsetof(vertex, tangent_);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset));

		offset = offsetof(vertex, bitangent_);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vertex), reinterpret_cast<void*>(offset));

		//notice here:
		glBindVertexArray(0);
	}
};





#endif // __BASIC_TYPE_H__

