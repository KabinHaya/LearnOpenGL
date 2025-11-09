#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube cubeTex;
uniform vec3 objectColor;

void main()
{
    float ratio = 1.0 / 1.52;
    vec3 I = normalize(Position - cameraPos);
    vec3 R = refract(I, normalize(Normal), ratio);
    FragColor = vec4(texture(cubeTex, R).rgb + objectColor, 1.0);
}