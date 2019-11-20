#version 330 core
layout (location = 0) in vec3 ver_position;
layout (location = 2) in vec2 ver_tex_coord;
layout (location = 3) in mat4 instance_matrix;

out vec2 TexCoords;
uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = ver_tex_coord;
    gl_Position = projection * view * instance_matrix * vec4(ver_position, 1.0); 
}