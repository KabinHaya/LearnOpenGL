#version 330 core
out vec4 FragColor;             // 片段颜色

// 定义材质结构体
struct Material
{
    sampler2D diffuse;  // 漫反射贴图
    sampler2D specular; // 镜面光贴图
    float shininess;    // 高光指数
};

struct DirLight
{
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight
{
    vec3 position;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

#define NR_POINT_LIGHTS 4

in vec2 outTexCoord;            // 纹理坐标
in vec3 outNormal;              // 法向量
in vec3 outFragPos;             // 片段位置

uniform vec3 viewPos;           // 摄像机位置
uniform Material material;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;

// 函数
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
float LinearizeDepth(float depth, float near, float far);

void main()
{
    // 属性
    vec3 norm = normalize(outNormal);
    vec3 viewDir = normalize(viewPos - outFragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);
    for (int i = 0; i < NR_POINT_LIGHTS; i++)
    {
        result += CalcPointLight(pointLights[i], norm, outFragPos, viewDir);
    }
    result += CalcSpotLight(spotLight, norm, outFragPos, viewDir);

    // FragColor = vec4(result, 1.0f);

    float depth = LinearizeDepth(gl_FragCoord.z, 0.1f, 100.0f) / 100.0f; // 为了演示除以 far
    FragColor = vec4(vec3(depth), 1.0);
}

// calculates the color when using a directional light.
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 diffuseTexture = vec3(texture(material.diffuse, outTexCoord));
    vec3 specularTexture = vec3(texture(material.specular, outTexCoord));
    vec3 lightDir = normalize(-light.direction);

    // 环境光
    vec3 ambient = light.ambient * diffuseTexture;

    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseTexture;

    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularTexture;

    return (ambient + diffuse + specular);
}

// calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 diffuseTexture = vec3(texture(material.diffuse, outTexCoord));
    vec3 specularTexture = vec3(texture(material.specular, outTexCoord));
    vec3 lightDir = normalize(light.position - fragPos);

    // 环境光
    vec3 ambient = light.ambient * diffuseTexture;
    
    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseTexture;
    
    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularTexture;
    
    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
                    light.quadratic * (distance * distance));    
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
}

// calculates the color when using a spot light.
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 diffuseTexture = vec3(texture(material.diffuse, outTexCoord));
    vec3 specularTexture = vec3(texture(material.specular, outTexCoord));
    vec3 lightDir = normalize(light.position - fragPos);

    // 环境光
    vec3 ambient = light.ambient * diffuseTexture;

    // 漫反射
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * diffuseTexture;

    // 镜面反射
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * spec * specularTexture;

    // 衰减
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    // 聚光强度
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}

float LinearizeDepth(float depth, float near, float far)
{float z = depth * 2.0f - 1.0f; // 转换为 NDC
    return (2.0f * near * far) / (far + near - z * (far - near));
}