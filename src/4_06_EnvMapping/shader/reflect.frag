#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube cubeTex;
uniform vec3 objectColor;

void main()
{             
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    FragColor = vec4(texture(cubeTex, R).rgb + objectColor, 1.0);
}