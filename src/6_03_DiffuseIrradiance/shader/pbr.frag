#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 WorldPos;
    vec3 Normal;
    vec2 TexCoords;
} fs_in;

struct Material {
    vec3 albedo;
    samplerCube irradianceMap;
    float metallic;
    float roughness;
    float ao;
};

struct PointLight {
    vec3 position;
    vec3 color;
};

uniform vec3 camPos;

uniform Material material;
uniform PointLight pointLights[4];

const float PI = 3.14159265359;

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

void main()
{
    vec3 N = fs_in.Normal;
    vec3 V = normalize(camPos - fs_in.WorldPos);

    vec3 F0 = vec3(0.04f);
    F0 = mix(F0, material.albedo, material.metallic);

    vec3 Lo = vec3(0.0f);
    for (int i = 0; i < 4; ++i)
    {
        // 计算每个灯光的辐照率
        vec3 L            = normalize(pointLights[i].position - fs_in.WorldPos);
        vec3 H            = normalize(V + L);
        float distance    = length(pointLights[i].position - fs_in.WorldPos);
        float attenuation = 1.0f / (distance * distance);
        vec3 radiance     = pointLights[i].color * attenuation;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, material.roughness);
        float G   = GeometrySmith(N, V, L, material.roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0f), F0);

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f) + 0.0001; // 加上0.0001防止除以零
        vec3 specular     = numerator / denominator;

        vec3 kS = F;
        vec3 kD = vec3(1.0f) - kS;

        kD *= 1.0f - material.metallic;

        float NdotL = max(dot(N, L), 0.0f);

        Lo += (kD * material.albedo / PI + specular) * radiance * NdotL;
    }

    vec3 kS = fresnelSchlick(max(dot(N, V), 0.0f), F0);
    vec3 kD = 1.0f - kS;
    kD *= 1.0f - material.metallic;
    vec3 irradiance = texture(material.irradianceMap, N).rgb;
    vec3 diffuse = irradiance * material.albedo;
    vec3 ambient = (kD * diffuse) * material.ao;
    // vec3 ambient = vec3(0.002f);


    vec3 color = ambient + Lo;
    
    // HDR
    color = color / (color + vec3(1.0f));
    // gamma校正
    color = pow(color, vec3(1.0f/2.2f));

    FragColor = vec4(color, 1.0f);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max(dot(N, H), 0.0f);
    float NdotH2 = NdotH * NdotH;

    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0f) + 1.0f);
    denom        = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughtness)
{
    float r     = (roughtness + 1.0f);
    float k     = (r * r) / 8.0f;

    float nom   = NdotV;
    float denom = NdotV * (1.0f - k) + k;
    
    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}