#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform samplerCube environmentMap;

void main()
{
	vec3 envColor = texture(environmentMap, WorldPos).rgb;

	// HDR 和 gamma校正
	envColor = envColor / (envColor + vec3(1.0f));
	envColor = pow(envColor, vec3(1.0f / 2.2f));

	FragColor = vec4(envColor, 1.0f);
}