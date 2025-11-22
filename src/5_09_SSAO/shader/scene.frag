#version 330 core
out vec4 FragColor;

// 定义材质结构体
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D ssao;
uniform float shininess;        // 高光指数

struct PointLight {
    vec3 position;

    float constant;
    float linear;
    float quadratic;
    float radius;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec2 TexCoords;

#define NR_POINT_LIGHTS 32

uniform vec3 viewPos;           // 摄像机位置
uniform PointLight pointLights[NR_POINT_LIGHTS];

// 函数
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Diffuse, float ambientOcclusion);

void main()
{
    // 属性
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float AmbientOcclusion = texture(ssao, TexCoords).r;

    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = vec3(0.0f);
    for (int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        float distance = length(pointLights[i].position - FragPos);
        if(distance < pointLights[i].radius)
            result += CalcPointLight(pointLights[i], Normal, FragPos, viewDir, Diffuse, AmbientOcclusion);
    }

    FragColor = vec4(result, 1.0f);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 Diffuse, float ambientOcclusion)
{
    vec3 lightDir = normalize(light.position - fragPos);

    // 环境光
    vec3 ambient = light.ambient * Diffuse * ambientOcclusion;
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * (diff * Diffuse);
    
    // 镜面反射
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(viewDir, halfwayDir), 0.0), shininess);
    vec3 specular = light.specular * spec;
    
    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                    light.quadratic * (distance * distance));    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}
