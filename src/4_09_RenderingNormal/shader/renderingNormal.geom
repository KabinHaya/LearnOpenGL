#version 330 core
layout (triangles) in;
layout (line_strip, max_vertices = 6) out;
// 是line_strip

in VS_OUT
{
    vec3 normal;
} gs_in[];

uniform float magnitude;

uniform mat4 projection;

void GenerateLine(int index)
{
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();

    gl_Position = projection * (gl_in[index].gl_Position +
                                vec4(gs_in[index].normal, 0.0f) * magnitude);
    EmitVertex();

    EndPrimitive();
}

void main()
{
    // 当前功能为：为三角形每个顶点绘制其法向量
    // 可选项，求出该三角形的中心，然后绘制法线
    GenerateLine(0);
    GenerateLine(1);
    GenerateLine(2);
}