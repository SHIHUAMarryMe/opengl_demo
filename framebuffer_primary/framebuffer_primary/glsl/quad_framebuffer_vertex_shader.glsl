#version 330 core
layout (location = 0) in vec2 positon;
layout (location = 1) in vec2 texcoords;

out vec2 TexCoords;

void main()
{
    TexCoords = texcoords;
    gl_Position = vec4(positon.x, position.y, 0.0, 1.0); 
}  