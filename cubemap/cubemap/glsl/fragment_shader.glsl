#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D cube_texture;

void main()
{    
    FragColor = texture(cube_texture, TexCoords);
}