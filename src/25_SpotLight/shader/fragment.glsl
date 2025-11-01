#version 330 core
out vec4 FragColor;             // 片段颜色

// 定义材质结构体
struct Material
{
    sampler2D diffuse;  // 漫反射贴图
    sampler2D specular; // 镜面光贴图
    float shininess;    // 高光指数
};

struct Light
{
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

in vec2 outTexCoord;            // 纹理坐标
in vec3 outNormal;              // 法向量
in vec3 outFragPos;             // 片段位置

uniform vec3 viewPos;           // 摄像机位置
uniform Material material;
uniform Light light;

void main()
{
    vec3 diffuseTexture = vec3(texture(material.diffuse, outTexCoord));
    vec3 specularTexture = vec3(texture(material.specular, outTexCoord));    
    vec3 lightDir = normalize(light.position - outFragPos); // 光的方向，从片段到光源

    // 环境光
    vec3 ambient = light.ambient * diffuseTexture;

    // 漫反射
    vec3 norm = normalize(outNormal);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse =  light.diffuse * (diff * diffuseTexture);

    // 镜面反射
    vec3 viewDir = normalize(viewPos - outFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specular = light.specular * (spec * specularTexture);    

    // 聚光（柔和的边缘）
    float theta     = dot(lightDir, normalize(-light.direction));
    float epsilon   = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0f, 1.0f);
    diffuse *= intensity;
    diffuse *= intensity;

    // 计算光的衰减
    float distance = length(light.position - outFragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance +
                light.quadratic * (distance * distance));

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0f);
    
}