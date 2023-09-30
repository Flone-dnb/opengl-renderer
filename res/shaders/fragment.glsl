#version 460 core

out vec4 color;

in vec2 fragmentUv;

uniform sampler2D diffuseTexture;

void main()
{
    color = texture(diffuseTexture, fragmentUv);
} 
