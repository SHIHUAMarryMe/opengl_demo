#version 330 core

layout (location = 0) in vec3 the_positon;
layout (location = 2) in vec2 the_tex_coords;

out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


void main()
{
    TexCoords = the_tex_coords;
    gl_Position = projection * view * model * vec4(the_positon, 1.0f);
}