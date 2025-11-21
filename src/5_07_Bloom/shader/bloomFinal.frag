#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D screenTex;
uniform sampler2D bloomBlur;
uniform bool useBloom;
uniform float exposure;

void main()
{             
    const float gamma = 2.2;
    vec3 hdrColor = texture(screenTex, TexCoords).rgb;    
    vec3 bloomColor = texture(bloomBlur, TexCoords).rgb;
    if(useBloom)
        hdrColor += bloomColor; // additive blending
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));
    FragColor = vec4(result, 1.0);
}