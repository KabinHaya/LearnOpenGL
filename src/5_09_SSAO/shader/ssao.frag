#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform vec3 samples[64];

// 参数
int kernelSize = 64;
float radius = 0.5f;
float bias = 0.025f;

// 根据屏幕尺寸除以噪声大小在屏幕上平铺纹理
const vec2 noiseScale = vec2(1280.0f / 4.0f, 720.0f / 4.0f);

uniform mat4 projection;

void main()
{
    // 获取SSAO算法的输入
    vec3 fragPos = texture(gPosition, TexCoords).xyz;
    vec3 normal = normalize(texture(gNormal, TexCoords).rgb);
    vec3 randomVec = normalize(texture(texNoise, TexCoords * noiseScale).xyz);

    // 创建 TBN
    // 从切线空间转换到视图空间
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    // 计算遮挡因子
    float occlusion = 0.0f;
    for (int i = 0; i < kernelSize; ++i)
    {
        // 获取该样本的位置
        vec3 samplePos = TBN * samples[i]; // 从切线空间到视图空间
        samplePos = fragPos + samplePos * radius;

        // 投影样本位置并且采样纹理，获取纹理上的位置
        vec4 offset = vec4(samplePos, 1.0f);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5f + 0.5f;

        // 获取该样本的深度
        float sampleDepth = texture(gPosition, offset.xy).z;

        // 范围检查 以及 累加
        // 只有当深度值在取样半径内时才会影响遮挡因子
        float rangeCheck = smoothstep(0.0f, 1.0f, radius / abs(fragPos.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + bias ? 1.0f : 0.0f) * rangeCheck;
    }
    occlusion = 1.0f - (occlusion / kernelSize);

    FragColor = occlusion;
}