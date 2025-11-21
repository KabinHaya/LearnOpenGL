#version 330 core
layout (location = 0) in vec3 aPos;          // 顶点坐标
layout (location = 1) in vec3 aNormal;       // 法向量
layout (location = 2) in vec2 aTexCoords;    // 纹理坐标

out VS_OUT
{
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
} vs_out;

uniform mat4 model;         // 模型矩阵
uniform mat4 view;          // 视图矩阵
uniform mat4 projection;    // 投影矩阵

void main()
{
    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));   
    vs_out.TexCoords = aTexCoords;
        
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalize(normalMatrix * aNormal);
    
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}