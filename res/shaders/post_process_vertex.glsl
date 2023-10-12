#version 460 core

layout (location = 0) in vec3 position;
layout (location = 2) in vec2 uv;

out vec2 fragmentUv;

void main()
{
    // Position are specified in normalized device coordinates here.
    gl_Position = vec4(position, 1.0F); 

     // Set output parameters.
    fragmentUv = uv;
}  