#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D brdfTexture;

void main()
{
    vec2 v = texture(brdfTexture, TexCoords).rg;
    // 把两个分量可视化为 RGB，便于检查是否为全 0
    FragColor = vec4(v, v.x, 1.0);
}