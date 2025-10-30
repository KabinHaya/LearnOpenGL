#version 330 core
out vec4 FragColor;
in vec3 ourColor;
in vec2 texCoord;

// 采样
uniform sampler2D texture1; // 默认传递为 0
uniform sampler2D texture2;

uniform float mixValue;

void main()
{

    FragColor = mix(texture(texture1, texCoord), texture(texture2, texCoord), abs(sin(mixValue * 0.5f)));
}