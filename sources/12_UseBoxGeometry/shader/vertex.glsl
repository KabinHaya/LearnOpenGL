#version 330 core
layout (location = 0) in vec3 Position;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoords;

out vec3 outPosition;
out vec2 outTexCoord;

uniform float factor;

mat4 rotateX(float angle)
{
    return mat4(
        1.0f,       0.0f,        0.0f,  0.0f, 
        0.0f, cos(angle), -sin(angle),  0.0f, 
        0.0f, sin(angle),  cos(angle),  0.0f, 
        0.0f,       0.0f,        0.0f,  1.0f
    );
}

mat4 rotateY(float angle)
{
    return mat4(
         cos(angle),  0.0f,   sin(angle),  0.0f, 
               0.0f,  1.0f,         0.0f,  0.0f, 
        -sin(angle),  0.0f,   cos(angle),  0.0f, 
               0.0f,  0.0f,         0.0f,  1.0f
    );
}

mat4 rotateZ(float angle) 
{
    return mat4(
        cos(angle), -sin(angle),  0.0f,  0.0f,
        sin(angle),  cos(angle),  0.0f,  0.0f,
              0.0f,        0.0f,  1.0f,  0.0f,
              0.0f,        0.0f,  0.0f,  1.0f
        );
}

mat4 rotateXYZ(float angle) 
{
    return rotateZ(angle) * rotateY(angle) * rotateX(angle);
}

void main()
{
    gl_Position = rotateXYZ(factor) * vec4(Position, 1.0f);
    // gl_Position = vec4(Position, 1.0f);
    // gl_PointSize = 10.0f;
    outTexCoord = TexCoords;
}