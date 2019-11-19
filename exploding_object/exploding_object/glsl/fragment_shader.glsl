#version 330 core
out vec4 frag_color;

in vec2 TexCoord;

uniform sampler2D texture_diffuse_1;

void main()
{
    frag_color = texture(texture_diffuse_1, TexCoord);
}