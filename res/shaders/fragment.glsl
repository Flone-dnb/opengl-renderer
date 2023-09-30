#version 460 core

in vec2 fragmentUv;

layout(binding = 0) uniform sampler2D diffuseTexture;

out vec4 color;

void main()
{
    color = texture(diffuseTexture, fragmentUv);
    //color = vec4(1.0F, 0.0F, 0.0F, 1.0F);
} 
