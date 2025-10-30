#version 330 core
layout (location = 0) in vec3 Position;     // 顶点坐标
layout (location = 1) in vec3 Normal;       // 法向量
layout (location = 2) in vec2 TexCoords;    // 纹理坐标

out vec2 outTexCoord;       // 传出材质坐标
out vec3 outNormal;         // 传出法向量
out vec3 outFragPos;        // 传出片段位置

uniform mat4 model;         // 模型矩阵
uniform mat4 view;          // 视图矩阵
uniform mat4 projection;    // 投影矩阵

void main()
{
    // 注意乘法要从右向左读
    gl_Position = projection * view * model * vec4(Position, 1.0f);
    outFragPos = vec3(model * vec4(Position, 1.0f));    
    // 解决不等比缩放，使用法线矩阵（model 的逆转置）
    // 不推荐在着色器上进行矩阵求逆，对GPU开销过大
    outNormal = mat3(transpose(inverse(model))) * Normal;
    outTexCoord = TexCoords;
}