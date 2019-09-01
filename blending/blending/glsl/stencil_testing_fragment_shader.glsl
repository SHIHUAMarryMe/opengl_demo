#version 330 core
out vec4 fragColor;

in vec2 TexCoords;

uniform sampler2D texture_1;

void main()
{
    fragColor = texture(texture_1, TexCoords);
}