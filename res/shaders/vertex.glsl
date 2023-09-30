#version 460 core

out vec2 fragmentUv;

layout (location = 0) in vec3 position;
layout (location = 0) in vec2 uv;

void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0F);

    fragmentUv = uv;
} 
