#ifndef __MODEL_HPP__
#define __MODEL_HPP__

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"
#include "stb_image/stb_image.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <list>


class model_loader final
{
private:
	std::map <std::shared_ptr<mesh>, std::list<texture>> mesh_with_textures_;
	std::vector<std::pair<std::basic_string<char>, texture>> loaded_texture_{};

	std::basic_string<char> texture_file_dir_{};
public:
	void load_model(const std::basic_string<char>& model_file)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(model_file, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

		if (!scene)
		{
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}

		// check for errors
		if (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
			return;
		}
	}

private:
	std::shared_ptr<mesh> process_mesh(const aiMesh* const ai_mesh, const aiScene* const ai_scene)
	{
		std::vector<vertex> vertices{};
		std::vector<std::size_t> indices{};
		std::vector<texture> textures{};

		for (std::size_t index = 0; index < ai_mesh->mNumVertices; ++index)
		{
			vertex temp_vertex{};
			glm::vec3 position{};
			glm::vec3 normal{};

			// positions
			position.x = ai_mesh->mVertices[i].x;
			position.y = ai_mesh->mVertices[i].y;
			position.z = ai_mesh->mVertices[i].z;

			// normals
			normal.x = ai_mesh->mNormals[i].x;
			normal.y = ai_mesh->mNormals[i].y;
			normal.z = ai_mesh->mNormals[i].z;

			temp_vertex.position_ = position;
			temp_vertex.normal_ = normal;


			if (ai_mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				glm::vec2 texcoord{};
				// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
				// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
				texcoord.x = ai_mesh->mTextureCoords[0][i].x;
				texcoord.y = ai_mesh->mTextureCoords[0][i].y;
				temp_vertex.texcoord_ = texcoord;
			}
			else
			{
				temp_vertex.texcoord_ = glm::vec2{ 0, 0 };
			}

			// tangent
			glm::vec3 tangent{};
			tangent.x = ai_mesh->mTangents[i].x;
			tangent.y = ai_mesh->mTangents[i].y;
			tangent.z = ai_mesh->mTangents[i].z;
			temp_vertex.tangent_ = tangent;

			// bitangent
			glm::vec3 bitangent{};
			bitangent.x = ai_mesh->mBitangents[i].x;
			bitangent.y = ai_mesh->mBitangents[i].y;
			bitangent.z = ai_mesh->mBitangents[i].z;
			temp_vertex.bitangent_ = bitangent;

			vertices.push_back(temp_vertex);
		}

		// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
		for (std::size_t index = 0; index < mesh->mNumFaces; ++index)
		{
			aiFace face{ ai_mesh->mFaces[index] };

			for (std::size_t EBO_index = 0; EBO_index < face.mNumIndices; ++EBO_index)
			{
				indices.push_back(face.mIndices[EBO_index]);
			}
		}

		// process materials
		aiMaterial* material = ai_scene->mMaterials[ai_mesh->mMaterialIndex];

		// 1. diffuse maps
		std::vector<texture> diffuse_texture{ load_material_texture(material, aiTextureType_DIFFUSE, texture_type::diffuse_type) };
		textures.insert(textures.end(), diffuse_texture.begin(), diffuse_texture.end());

		// 2. specular maps
		std::vector<texture> specular_texture{ load_material_texture(material, aiTextureType_SPECULAR, texture_type::specular_type) };
		textures.insert(textures.end(), specular_texture.begin(), specular_texture.end());

		// 3. normal maps
		std::vector<texture> normal_texture{ load_material_texture(material, aiTextureType_HEIGHT,  texture_type::normal_type) };
		textures.insert(textures.end(), normal_texture.begin(), normal_texture.end());

		// 4. height maps
		std::vector<texture> height_texture{ load_material_texture(material, aiTextureType_AMBIENT, texture_type::height_type) };
		textures.insert(textures.end(), height_texture.begin(), height_texture.end());

		std::shared_ptr<mesh> shared_mesh{ std::make_shared<mesh>() };
		shared_mesh->add_vertices(vertices);
		shared_mesh->add_indices(indices);
		shared_mesh->add_textures(textures);

	}


	std::vector<texture> load_material_texture(aiMaterial * const ai_material, aiTextureType type, texture_type the_type)
	{
		std::vector<texture> textures{};
		for (std::size_t index = 0; index < ai_material->GetTextureCount(type); ++index)
		{
			aiString texture_file_name{};
			ai_material->GetTexture(type, index, &texture_file_name);

			// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
			bool skip{ false };
			for (std::size_t load_texture_index = 0; load_texture_index < loaded_texture_.size(); ++load_texture_index)
			{
				if (std::strcmp(loaded_texture_[load_texture_index].first.c_str(), texture_file_name.C_Str()) == 0)
				{
					textures.push_back(loaded_texture_[load_texture_index].second);
					skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
					break;
				}
			}

			if (!skip)
			{   // if texture hasn't been loaded already, load it
				texture temp_texture{};
				temp_texture.id_ = model_loader::load_texture_from_file(texture_file_name.C_Str(), texture_file_dir_);
				temp_texture.type_ = the_type;
				textures.push_back(temp_texture);
				loaded_texture_.emplace_back(std::basic_string<char>{texture_file_name.C_Str()}, temp_texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
			}
		}
		return textures;
	}

	// gamma unused temporarily.
	static unsigned int load_texture_from_file(const std::basic_string<char>& file_name, const std::basic_string<char>& file_path, bool gamma = false)
	{
		std::basic_string<char> file_path_name{ file_path + '/' + file_name };

		unsigned int texture_id{};
		glGenTextures(1, &texture_id);

		int width{}, height{}, nrComponents{};
		unsigned char *data{ stbi_load(file_path_name.c_str(), &width, &height, &nrComponents, 0) };

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
			std::cout << "Texture failed to load at path: " << file_path << std::endl;
			stbi_image_free(data);
		}

		return texture_id;
	}


};


#endif // !__MODEL_HPP__
