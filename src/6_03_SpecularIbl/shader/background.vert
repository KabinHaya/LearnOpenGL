#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

out vec3 WorldPos;

uniform mat4 view;
uniform mat4 projection;

void main() 
{
    WorldPos = aPos;
    
    mat4 rotView = mat4(mat3(view));
    vec4 clipPos = projection * rotView * vec4(WorldPos, 1.0f);

    gl_Position = clipPos.xyww;
}