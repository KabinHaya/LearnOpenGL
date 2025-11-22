#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out VS_OUT
{
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
} vs_out;

uniform bool invertedNormals;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() 
{
	vec4 worldPos = model * vec4(aPos, 1.0);
	vs_out.FragPos = worldPos.xyz;

	vs_out.TexCoords = aTexCoords;
	// 解决不等比缩放，对法向量产生的影响
	vs_out.Normal = mat3(transpose(inverse(model))) * aNormal;
	vs_out.Normal *= invertedNormals ? -1 : 1;

	gl_Position = projection * view * worldPos;
}