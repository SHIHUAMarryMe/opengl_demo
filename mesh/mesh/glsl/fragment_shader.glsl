#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D diffuse_texture_1;

void main()
{    
    FragColor = texture(diffuse_texture_1, TexCoords);
}