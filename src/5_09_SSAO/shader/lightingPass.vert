#version 330 core
layout (location = 0) in vec3 aPos;          // 顶点坐标
layout (location = 1) in vec3 aNormal;       // 法向量
layout (location = 2) in vec2 aTexCoords;    // 纹理坐标

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    
    gl_Position = vec4(aPos, 1.0);
}