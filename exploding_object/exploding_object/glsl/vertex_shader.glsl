#version 330 core
layout (location = 0) in vec3 ver_position;
layout (location = 2) in vec2 ver_tex_coord;

out VS_OUT {
    vec2 texture_coord;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    vs_out.texture_coord = ver_tex_coord;
    gl_Position = mode * view * projection * vec4(ver_position, 1.0); 
}