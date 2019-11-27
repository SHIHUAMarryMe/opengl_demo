#version 330 core
out vec4 frag_color;

in vec2 TexCoords;

uniform sampler2D texture_diffuse_1;

void main()
{
    FragColor = texture(texture_diffuse1, TexCoords);
}