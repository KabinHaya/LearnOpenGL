#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform float nearPlane;
uniform float farPlane;

float LinearizeDepth(float depth)
{
	float z = depth * 2.0f - 1.0f;
	return (2.0f * nearPlane * farPlane) / (farPlane + nearPlane - z * (farPlane - nearPlane));
}

void main()
{
	float depthValue = texture(depthMap, TexCoords).r;
	// FragColor = vec4(vec3(LinearizeDepth(depthValue) / farPlane), 1.0); // 透视
    FragColor = vec4(vec3(depthValue), 1.0); // 正交
}