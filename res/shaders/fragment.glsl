#version 460 core

in vec2 fragmentUv;

layout(binding = 0) uniform sampler2D diffuseTexture;

out vec4 color;

void main()
{
    color = texture(diffuseTexture, fragmentUv);
} 
