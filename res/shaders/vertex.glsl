#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec2 fragmentUv;

uniform mat4 worldMatrix;
uniform mat4 viewProjectionMatrix;

void main()
{
    gl_Position = viewProjectionMatrix * worldMatrix * vec4(position, 1.0F);

    fragmentUv = uv;
} 
