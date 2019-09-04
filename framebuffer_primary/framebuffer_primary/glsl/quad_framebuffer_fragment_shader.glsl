#version 330 core
out vec4 frag_color;

in vec2 TexCoords;

uniform sampler2D screen_texture;

void main()
{
    vec3 rgb_value = texture(screen_texture, TexCoords).rgb;
    frag_color = vec4(vec3(1.0-rgb_value), 1.0);
}