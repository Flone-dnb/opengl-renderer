#version 460 core 

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 textureCoordinates;

uniform mat4 viewProjectionMatrix;

void main()
{
    // Set position.
    gl_Position = viewProjectionMatrix * vec4(position, 1.0F);
    gl_Position = gl_Position.xyww; // passing `xyww` so that after perspective division the depth will be 1.0
                                        // and skybox will only be rendered for pixels where no other object was drawn

    // Set output parameters.
    textureCoordinates = position.xyz;
} 
