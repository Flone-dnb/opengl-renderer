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
    float intensity;
    float distance;
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

float calculatePointLightAttenuation(float distanceToLight, float lightIntensity, float lightHalfRadius){
    float distanceToLightDivHalfRadius = distanceToLight / lightHalfRadius;
    return lightIntensity / (1 + distanceToLightDivHalfRadius * distanceToLightDivHalfRadius);
}

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

    // Calculate light attenuation.
    float fragmentDistanceToLight = length(lightSource.position - fragmentPosition);
    vec3 attenuatedLightColor =
        lightSource.color * calculatePointLightAttenuation(
            fragmentDistanceToLight, lightSource.intensity, lightSource.distance);

    // Calculate diffuse color.
    vec3 fragmentToLightDirectionUnit = normalize(lightSource.position - fragmentPosition);
    float cosFragmentToLight = max(dot(fragmentNormalUnit, fragmentToLightDirectionUnit), 0.0F);
    vec3 diffuseLight = cosFragmentToLight * fragmentDiffuseColor * attenuatedLightColor;

    // Calculate specular color.
    vec3 fragmentLightReflectionDirectionUnit = reflect(-fragmentToLightDirectionUnit, fragmentNormalUnit);
    vec3 fragmentToCameraDirectionUnit = normalize(cameraPositionInWorldSpace - fragmentPosition);
    float specularFactor =
        pow(max(dot(fragmentToCameraDirectionUnit, fragmentLightReflectionDirectionUnit), 0.0),
            material.shininess);
    vec3 specularColor = attenuatedLightColor * (specularFactor * fragmentSpecularColor);

    // Apply calculated light.
    color.xyz *= ambientColor + diffuseLight + specularColor;
} 
