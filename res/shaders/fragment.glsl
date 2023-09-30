#version 460 core

out vec4 color;

in vec2 fragmentUv;

layout(binding = 0) uniform sampler2D diffuseTexture;

void main()
{
    color = texture(diffuseTexture, fragmentUv);
} 
