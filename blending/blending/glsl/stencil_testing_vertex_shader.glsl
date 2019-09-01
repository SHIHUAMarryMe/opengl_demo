#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 the_texcoords;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = the_texcoords;
    gl_Position = projection * view * model * vec4(position, 1.0f);
}