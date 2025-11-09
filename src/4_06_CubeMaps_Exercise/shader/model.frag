#version 330 core
out vec4 FragColor;             // 片段颜色

// 定义材质结构体
struct Material
{
    sampler2D diffuse;
    sampler2D reflection;
    samplerCube cubeTex;
};

in vec2 outTexCoord;            // 纹理坐标
in vec3 outNormal;              // 法向量
in vec3 outFragPos;             // 片段位置

uniform vec3 viewPos;           // 摄像机位置
uniform Material material;

void main()
{
    vec3 viewDir = normalize(viewPos - outFragPos);
    vec3 normal = normalize(outNormal);

    vec3 R = reflect(-viewDir, normal);
    vec3 reflectMap = vec3(texture(material.reflection, outTexCoord));
    vec3 reflection = vec3(texture(material.cubeTex, R).rgb) * reflectMap;

    float diff = max(normalize(dot(normal, viewDir)), 0.0f);
    vec3 diffuse = diff * vec3(texture(material.diffuse, outTexCoord));

    FragColor = vec4(diffuse + reflection, 1.0f);

    // 自己去model.h将
    // std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    // 改为
    // std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_reflection");
    // 但是不清楚为什么感觉变化不大
}