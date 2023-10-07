#include "base.glsl"

in vec2 fragmentUv;
in vec3 fragmentNormal;
in vec3 fragmentPosition;

#ifdef USE_DIFFUSE_TEXTURE
layout(binding = 0) uniform sampler2D diffuseTexture;
#endif

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 cameraPositionInWorldSpace;

out vec4 color;

void main()
{
    // Normals may be unnormalized after the rasterization (when they are interpolated).
    vec3 fragmentNormalUnit = normalize(fragmentNormal);
    
    // Define base color.
    color = vec4(1.0F, 1.0F, 1.0F, 1.0F);

    // Apply texture color.
#ifdef USE_DIFFUSE_TEXTURE
    color *= texture(diffuseTexture, fragmentUv);
#endif

    // Prepare ambient color and specular reflection intencity.
    vec3 ambientColor = vec3(0.1F, 0.1F, 0.1F);
    float specularStrength = 0.5F;

    // Calculate diffuse color.
    vec3 fragmentToLightDirectionUnit = normalize(lightPosition - fragmentPosition);
    float cosFragmentToLight = max(dot(fragmentNormalUnit, fragmentToLightDirectionUnit), 0.0F);
    vec3 diffuseColor = cosFragmentToLight * lightColor;

    // Calculate specular color.
    vec3 fragmentLightReflectionDirectionUnit = reflect(-fragmentToLightDirectionUnit, fragmentNormalUnit);
    vec3 fragmentToCameraDirectionUnit = normalize(cameraPositionInWorldSpace - fragmentPosition);
    int shininess = 32;
    float specularFactor = pow(max(dot(fragmentToCameraDirectionUnit, fragmentLightReflectionDirectionUnit), 0.0), shininess);
    vec3 specularColor = specularStrength * specularFactor * lightColor;

    // Apply calculated light.
    color.xyz *= ambientColor + diffuseColor + specularColor;
} 
