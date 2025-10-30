#version 330 core
out vec4 FragColor;             // 片段颜色
in vec2 outTexCoord;            // 纹理坐标
in vec3 outNormal;              // 法向量
in vec3 outFragPos;             // 片段位置
in vec3 outLightPos;            // 光源位置

uniform vec3 lightColor;        // 光源发出的颜色
uniform float ambientStrength;  // 环境光照强度
uniform vec3 viewPos;           // 摄像机位置

// 采样
uniform sampler2D texture1; // 默认传递为 0
uniform sampler2D texture2;

void main()
{
    // 材质颜色
    vec3 texColor = mix(texture(texture1, outTexCoord).rgb, texture(texture2, outTexCoord).rgb, 0.2);

    // 环境光
    vec3 ambient = ambientStrength * lightColor;

    // 漫反射
    vec3 norm = normalize(outNormal);
    vec3 lightDir = normalize(outLightPos - outFragPos); // 光的方向，从片段到光源
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    // 镜面反射
    vec3 viewDir = normalize(viewPos - outFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float specularStrength = 0.5f; // 镜面反射强度
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * vec3(texColor);

    FragColor = vec4(result, 1.0f);
}