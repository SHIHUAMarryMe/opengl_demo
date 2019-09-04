#version 330 core
out vec4 frag_color;

in vec2 TexCoords;

uniform sampler2D screen_texture;

void main()
{
    frag_color = texture(screen_texture, TexCoords);
    float average_value = (frag_color.r + frag_color.g + frag_color.b)/3.0f;
    frag_color = vec4(average_value, average_value, average_value, 1.0f);
}