/*
	与灯光物体有关
*/
#version 330 core
out vec4 FragColor;

uniform vec3 lightColor;

void main()
{
	FragColor = vec4(lightColor, 1.0f);
}