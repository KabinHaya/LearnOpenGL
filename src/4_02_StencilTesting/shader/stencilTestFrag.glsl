#version 330 core
out vec4 FragColor;             // 片段颜色

in vec2 outTexCoord;       // 传出材质坐标
in vec3 outNormal;         // 传出法向量
in vec3 outFragPos;        // 传出片段位置

void main()
{
	FragColor = vec4(0.04, 0.28, 0.26, 1.0);
}