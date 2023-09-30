#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 uv;

out vec2 fragmentUv;

void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0F);

    fragmentUv = uv;
} 
