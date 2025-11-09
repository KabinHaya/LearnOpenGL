#version 330 core
layout (location = 0) in vec3 aPos;          // 顶点坐标
layout (location = 1) in vec3 aNormal;       // 法向量
layout (location = 2) in vec2 aTexCoords;    // 纹理坐标

out vec2 TexCoords;      // 传出材质坐标
out vec3 Normal;         // 传出法向量
out vec3 FragPos;        // 传出片段位置

uniform mat4 model;         // 模型矩阵
uniform mat4 view;          // 视图矩阵
uniform mat4 projection;    // 投影矩阵

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0f));    
    // 解决不等比缩放，使用法线矩阵（model 的逆转置）
    // 不推荐在着色器上进行矩阵求逆，对GPU开销过大
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoords = aTexCoords;
    // 注意乘法要从右向左读
    gl_Position = projection * view * vec4(FragPos, 1.0f);
}