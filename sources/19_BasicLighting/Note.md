## 冯氏光照源代码

在世界空间中计算冯氏光照

==vertex.glsl==

```glsl
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
```

==fragment.glsl==

```glsl
#version 330 core
out vec4 FragColor;             // 片段颜色
in vec2 outTexCoord;            // 纹理坐标
in vec3 outNormal;              // 法向量
in vec3 outFragPos;             // 片段位置

uniform vec3 lightColor;        // 光源发出的颜色
uniform vec3 lightPos;          // 光源所在位置
uniform float ambientStrength;  // 环境光照强度
uniform vec3 viewPos;           // 摄像机位置

// 采样
uniform sampler2D texture1; // 默认传递为 0
uniform sampler2D texture2;

void main()
{
    // 材质颜色
    vec3 texColor = mix(texture(texture1, outTexCoord).rgb, texture(texture2, outTexCoord).rgb, 0.2);
    
    // 环境光
    vec3 ambient = ambientStrength * lightColor;

    // 漫反射
    vec3 norm = normalize(outNormal);
    vec3 lightDir = normalize(lightPos - outFragPos); // 光的方向，从片段到光源
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    // 镜面反射
    vec3 viewDir = normalize(viewPos - outFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float specularStrength = 0.5f; // 镜面反射强度
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * vec3(texColor);

    FragColor = vec4(result, 1.0f);
}
```

## 练习

### 在观察空间（而不是世界空间）中计算冯氏光照

修改过后

==vertex.glsl==

```glsl
#version 330 core
layout (location = 0) in vec3 Position;     // 顶点坐标
layout (location = 1) in vec3 Normal;       // 法向量
layout (location = 2) in vec2 TexCoords;    // 纹理坐标

out vec2 outTexCoord;       // 传出材质坐标
out vec3 outNormal;         // 传出法向量
out vec3 outFragPos;        // 传出片段位置
out vec3 outLightPos;       // 传出光源位置

uniform vec3 lightPos;      // 光源所在位置

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
    outLightPos = vec3(view * vec4(lightPos, 1.0f)); // 将光源位置从世界坐标转换为观察坐标
}
```

==fragment.glsl==

```glsl
#version 330 core
out vec4 FragColor;             // 片段颜色
in vec2 outTexCoord;            // 纹理坐标
in vec3 outNormal;              // 法向量
in vec3 outFragPos;             // 片段位置
in vec3 outLightPos;            // 光源位置

uniform vec3 lightColor;        // 光源发出的颜色
uniform float ambientStrength;  // 环境光照强度
uniform vec3 viewPos;           // 摄像机位置

// 采样
uniform sampler2D texture1; // 默认传递为 0
uniform sampler2D texture2;

void main()
{
    // 材质颜色
    vec3 texColor = mix(texture(texture1, outTexCoord).rgb, texture(texture2, outTexCoord).rgb, 0.2);

    // 环境光
    vec3 ambient = ambientStrength * lightColor;

    // 漫反射
    vec3 norm = normalize(outNormal);
    vec3 lightDir = normalize(outLightPos - outFragPos); // 光的方向，从片段到光源
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    // 镜面反射
    vec3 viewDir = normalize(viewPos - outFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float specularStrength = 0.5f; // 镜面反射强度
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * vec3(texColor);

    FragColor = vec4(result, 1.0f);
}
```

- 最大区别是光源位置相比之前先进行了一次变换，从世界坐标变换到了观察坐标
- 修改地方就是传入光源位置的地方从==fragment.glsl==变为了==vertex.glsl==

### 尝试实现一个Gouraud着色（而不是冯氏着色）

==vertex.glsl==

```glsl
#version 330 core
layout (location = 0) in vec3 Position;     // 顶点坐标
layout (location = 1) in vec3 Normal;       // 法向量
layout (location = 2) in vec2 TexCoords;    // 纹理坐标

out vec2 outTexCoord;       // 传出材质坐标
out vec3 outLightingColor;  // 光照颜色

uniform vec3 lightPos;          // 光源所在位置
uniform vec3 lightColor;        // 光源发出的颜色
uniform vec3 viewPos;           // 摄像机位置
uniform float ambientStrength;  // 环境光照强度

uniform mat4 model;         // 模型矩阵
uniform mat4 view;          // 视图矩阵
uniform mat4 projection;    // 投影矩阵

void main()
{
    outTexCoord = TexCoords;
    // 注意乘法要从右向左读
    gl_Position = projection * view * model * vec4(Position, 1.0f);
    
    // gouraud 着色
    // ------------------------------------------------------------
    vec3 position = vec3(model * vec4(Position, 1.0f));
    vec3 normal = mat3(transpose(inverse(model))) * Normal;

    // 环境光
    vec3 ambient = ambientStrength * lightColor;

    // 漫反射
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(lightPos - position);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    // 镜面反射
    float specularStrength = 1.0f;
    vec3 viewDir = normalize(viewPos - position);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;      

    outLightingColor = ambient + diffuse + specular;
}
```

==fragment.glsl==

```glsl
#version 330 core
out vec4 FragColor;             // 片段颜色

in vec2 outTexCoord;            // 纹理坐标
in vec3 outLightingColor;       // 点的颜色

// 采样
uniform sampler2D texture1; // 默认传递为 0
uniform sampler2D texture2;

void main()
{
    // 材质颜色
    vec3 texColor = mix(texture(texture1, outTexCoord).rgb, texture(texture2, outTexCoord).rgb, 0.2f);

    FragColor = vec4(outLightingColor * texColor, 1.0f);
}
```

