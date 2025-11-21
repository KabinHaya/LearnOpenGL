#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTexture;
uniform bool useHDR;
uniform float exposure;
uniform int toneMappingMode;

void main()
{
    const float gamma = 2.2f;
	vec3 hdrColor = texture(screenTexture, TexCoords).rgb;
    vec3 result = vec3(1.0f);
	if (useHDR)
    {        
        
        if (toneMappingMode == 0) // Reinhard色调映射
            result = hdrColor / (hdrColor + vec3(1.0));
        else if (toneMappingMode == 1) // 曝光色调映射
            result = vec3(1.0) - exp(-hdrColor * exposure);
    }
    else
    {
        result = hdrColor;
    }

    // Gamma 校正
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}