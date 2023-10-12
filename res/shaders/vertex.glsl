#include "base.glsl"

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv;
layout (location = 3) in vec3 tangent;

out vec3 fragmentPosition;
out vec3 fragmentNormal;
out vec2 fragmentUv;
out mat3 tangentBitangentNormalMatrix;

uniform mat4 worldMatrix;
uniform mat3 normalMatrix;
uniform mat4 viewProjectionMatrix;

void main()
{
    // Calculate position in world space.
    vec4 positionInWorldSpace = worldMatrix * vec4(position, 1.0F);

    // Set position.
    gl_Position = viewProjectionMatrix * positionInWorldSpace;

    // Set output parameters.
    fragmentPosition = positionInWorldSpace.xyz;
    fragmentNormal = normalMatrix * normal;
    fragmentUv = uv;

    // Calculate vectors for TBN matrix.
    vec3 tangentUnit = normalize(normalMatrix * tangent);
    vec3 normalUnit = normalize(normalMatrix * normal);

    // Re-orthogonalize tangent using Gram-Schmidt process (to avoid non-orthogonal TBN matrix for better normals).
    tangentUnit = normalize(tangentUnit - dot(tangentUnit, normalUnit) * normalUnit);

    // Find bitangent as cross product.
    vec3 bitangentUnit = cross(normalUnit, tangentUnit);

    // Calculate TBN matrix.
    tangentBitangentNormalMatrix = mat3(
        tangentUnit,
        bitangentUnit,
        normalUnit
    );
} 
