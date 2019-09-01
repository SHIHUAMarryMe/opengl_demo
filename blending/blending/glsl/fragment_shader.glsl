#version 330 core
out vec4 frag_color;

in vec2 TexCoords;

uniform sampler2D texture_1;

void main()
{             
    vec4 tex_color = texture(texture_1, TexCoords);
    
    if(tex_color.a < 0.1)
        discard;

    frag_color = tex_color;
}