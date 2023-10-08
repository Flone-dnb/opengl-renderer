#include "base.glsl"

in vec2 fragmentUv;
in vec3 fragmentNormal;
in vec3 fragmentPosition;

struct Material {
    vec3 diffuseColor;
    vec3 specularColor;
    float shininess;
}; 

struct LightSource{
    vec3 position;
    vec3 color;
};

#ifdef USE_DIFFUSE_TEXTURE
layout(binding = 0) uniform sampler2D diffuseTexture;
#endif

#ifdef USE_METALLIC_ROUGHNESS_TEXTURE
layout(binding = 1) uniform sampler2D metallicRoughnessTexture;
#endif

uniform vec3 cameraPositionInWorldSpace;
uniform vec3 ambientColor;
uniform Material material;
uniform LightSource lightSource;

out vec4 color;

void main()
{
    // Normals may be unnormalized after the rasterization (when they are interpolated).
    vec3 fragmentNormalUnit = normalize(fragmentNormal);
    
    // Define base color.
    color = vec4(1.0F, 1.0F, 1.0F, 1.0F);

    // Prepare diffuse color.
    vec3 fragmentDiffuseColor = material.diffuseColor;
#ifdef USE_DIFFUSE_TEXTURE
    fragmentDiffuseColor *= vec3(texture(diffuseTexture, fragmentUv));
#endif

    // Prepare specular color.
    vec3 fragmentSpecularColor = material.specularColor;
#ifdef USE_METALLIC_ROUGHNESS_TEXTURE
    fragmentSpecularColor *= vec3(1.0F - texture(metallicRoughnessTexture, fragmentUv).y);
#endif

    // Calculate diffuse color.
    vec3 fragmentToLightDirectionUnit = normalize(lightSource.position - fragmentPosition);
    float cosFragmentToLight = max(dot(fragmentNormalUnit, fragmentToLightDirectionUnit), 0.0F);
    vec3 diffuseLight = cosFragmentToLight * fragmentDiffuseColor * lightSource.color;

    // Calculate specular color.
    vec3 fragmentLightReflectionDirectionUnit = reflect(-fragmentToLightDirectionUnit, fragmentNormalUnit);
    vec3 fragmentToCameraDirectionUnit = normalize(cameraPositionInWorldSpace - fragmentPosition);
    float specularFactor = pow(max(dot(fragmentToCameraDirectionUnit, fragmentLightReflectionDirectionUnit), 0.0), material.shininess);
    vec3 specularColor = lightSource.color * (specularFactor * fragmentSpecularColor);

    // Apply calculated light.
    color.xyz *= ambientColor + diffuseLight + specularColor;
} 
