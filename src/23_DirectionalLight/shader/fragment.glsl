#version 330 core
out vec4 FragColor;             // 片段颜色

// 定义材质结构体
struct Material
{
    sampler2D diffuse;  // 漫反射贴图
    sampler2D specular; // 镜面光贴图
    float shininess;    // 镜面光的高光指数
    sampler2D emission; // 放射光贴图
};

struct Light
{
    // vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
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
    vec3 emissionTexture = vec3(texture(material.emission, outTexCoord));

    // 环境光
    vec3 ambient = light.ambient * diffuseTexture;

    // 漫反射
    vec3 norm = normalize(outNormal);
    // vec3 lightDir = normalize(light.position - outFragPos); // 光的方向，从片段到光源
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse =  light.diffuse * (diff * diffuseTexture);

    // 镜面反射
    vec3 viewDir = normalize(viewPos - outFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specular = light.specular * (spec * specularTexture);

    // 放射光
    vec3 emission = 0.0f * emissionTexture;

    vec3 result = ambient + diffuse + specular + emission;

    FragColor = vec4(result, 1.0f);
}