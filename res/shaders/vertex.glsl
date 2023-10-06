#include "base.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;

out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec2 fragmentUv;

uniform mat4 worldMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewProjectionMatrix;

void main()
{
    vec4 positionInWorldSpace = worldMatrix * vec4(position, 1.0F);

    // Set position.
    gl_Position = viewProjectionMatrix * positionInWorldSpace;

    // Set output parameters.
    fragmentPosition = positionInWorldSpace.xyz;
    fragmentNormal = normalMatrix * normal;
    fragmentUv = uv;
} 
