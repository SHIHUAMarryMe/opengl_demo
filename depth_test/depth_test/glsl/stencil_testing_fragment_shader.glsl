#version 330 core
out vec4 frag_color;

in vec2 TexCoords;

uniform sampler2D texture_wall;

void main()
{
    frag_color = texture(texture1, TexCoords);
}