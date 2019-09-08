#version 330 core
out vec4 FragColor;

in vec3 Position;
in vec3 Normal;

uniform samplerCube skybox;
uniform vec3 camera_position;

void main()
{    
    float ratio = 1.00/1.52;

    vec3 I = normalize(Position - camera_position);
    vec3 R = refract(I, normalize(Normal), ratio);

    FragColor = vec4(texture(skybox, R).rgb, 1.0f);
}