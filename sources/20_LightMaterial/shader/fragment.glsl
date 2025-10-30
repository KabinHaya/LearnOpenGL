#version 330 core
out vec4 FragColor;             // 片段颜色
in vec2 outTexCoord;            // 纹理坐标
in vec3 outNormal;              // 法向量
in vec3 outFragPos;             // 片段位置

uniform vec3 lightColor;        // 光源发出的颜色
uniform vec3 viewPos;           // 摄像机位置

// 采样
uniform sampler2D texture1; // 默认传递为 0
uniform sampler2D texture2;

// 定义材质结构体
struct Material
{
    vec3 ambient;       // 环境光颜色
    vec3 diffuse;       // 漫反射颜色
    vec3 specular;      // 高光颜色
    float shininess;    // 高光指数
};

struct Light
{
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

uniform Material material;
uniform Light light;

void main()
{
    // 材质颜色
    // vec3 objColor = mix(texture(texture1, outTexCoord).rgb, texture(texture2, outTexCoord).rgb, 0.2);
    vec4 objColor = vec4(1.0f, 0.5f, 0.31f, 1.0f);

    // 环境光
    vec3 ambient = light.ambient * material.ambient;

    // 漫反射
    vec3 norm = normalize(outNormal);
    vec3 lightDir = normalize(light.position - outFragPos); // 光的方向，从片段到光源
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse =  light.diffuse * (diff * material.diffuse);

    // 镜面反射
    vec3 viewDir = normalize(viewPos - outFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), material.shininess);
    vec3 specular = light.specular * (spec * material.specular);

    vec3 result = (ambient + diffuse + specular) * vec3(objColor);

    FragColor = vec4(result, 1.0f);
}