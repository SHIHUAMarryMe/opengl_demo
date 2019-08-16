#ifdef  __BASIC_TYPE_H__
#define __BASIC_TYPE_H__

#include <glad/glad.h> // holds all OpenGL type declarations

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

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
	glm::vec2 texcoords_;
};


#endif // __BASIC_TYPE_H__

