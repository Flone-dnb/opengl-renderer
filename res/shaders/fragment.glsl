#include "base.glsl"

in vec2 fragmentUv;
in vec3 fragmentNormal;
in vec3 fragmentPosition;

#ifdef USE_DIFFUSE_TEXTURE
layout(binding = 0) uniform sampler2D diffuseTexture;
#endif

uniform vec3 lightPosition;
uniform vec3 lightColor;

out vec4 color;

void main()
{
    // Define base color.
    color = vec4(1.0F, 1.0F, 1.0F, 1.0F);

    // Apply texture color.
#ifdef USE_DIFFUSE_TEXTURE
    color *= texture(diffuseTexture, fragmentUv);
#endif

    // Prepare ambient color.
    vec3 ambientColor = vec3(0.1F, 0.1F, 0.1F);

    // Normals may be unnormalized after the rasterization (when they are interpolated).
    vec3 normal = normalize(fragmentNormal);

    // Calculate direction (in world space) to light source from our fragment.
    vec3 toLightDirection = normalize(lightPosition - fragmentPosition);

    // Calculate diffuse color.
    float cosFragmentToLight = max(dot(normal, toLightDirection), 0.0F);
    vec3 diffuseColor = cosFragmentToLight * lightColor;

    // Apply ambient + diffuse color.
    color.xyz *= ambientColor + diffuseColor;
} 
