#include "base.glsl"

in vec2 fragmentUv;
in vec3 fragmentNormal;
in vec3 fragmentPosition;
in mat3 tangentBitangentNormalMatrix;

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

#ifdef USE_NORMAL_TEXTURE
layout(binding = 1) uniform sampler2D normalTexture;
#endif

#ifdef USE_METALLIC_ROUGHNESS_TEXTURE
layout(binding = 2) uniform sampler2D metallicRoughnessTexture;
#endif

#ifdef USE_EMISSION_TEXTURE
layout(binding = 3) uniform sampler2D emissionTexture;
#endif

layout(binding = 4) uniform samplerCube environmentMap;

#define LIGHT_COUNT 2

uniform vec3 cameraPositionInWorldSpace;
uniform float ambientLightIntensity;
uniform float environmentIntensity;
uniform Material material;
uniform LightSource vLightSources[LIGHT_COUNT];

out vec4 color;

float calculatePointLightAttenuation(float distanceToLight, float lightIntensity, float lightHalfRadius){
    float distanceToLightDivHalfRadius = distanceToLight / lightHalfRadius;
    return lightIntensity / (1 + distanceToLightDivHalfRadius * distanceToLightDivHalfRadius);
}

vec3 calculateColorFromPointLight(LightSource lightSource, vec3 fragmentNormalUnit, vec3 fragmentDiffuseColor, vec3 fragmentSpecularColor){
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
    vec3 fragmentToCameraDirectionUnit = normalize(cameraPositionInWorldSpace - fragmentPosition);
    vec3 fragmentLightHalfwayDirectionUnit = normalize(fragmentToLightDirectionUnit + fragmentToCameraDirectionUnit);
    float specularFactor =
        pow(max(dot(fragmentNormalUnit, fragmentLightHalfwayDirectionUnit), 0.0),
            material.shininess);
    vec3 specularColor = attenuatedLightColor * (specularFactor * fragmentSpecularColor);

    return diffuseLight + specularColor + ambientLightIntensity * fragmentDiffuseColor;
}

void main()
{
    // Calculate normal.
#ifdef USE_NORMAL_TEXTURE
    vec3 fragmentNormalUnit = texture(normalTexture, fragmentUv).rgb; // read normal in range [0; 1]
    fragmentNormalUnit = fragmentNormalUnit * 2.0F - 1.0F; // convert to range [-1; 1]
    fragmentNormalUnit = normalize(tangentBitangentNormalMatrix * fragmentNormalUnit); // transform normal to world space
#else
    // Normals may be unnormalized after the rasterization (when they are interpolated).
    vec3 fragmentNormalUnit = normalize(fragmentNormal);
#endif
    
    // Define base (unlit) color.
    color = vec4(0.0F, 0.0F, 0.0F, 1.0F);

    // Prepare diffuse color.
    vec3 fragmentDiffuseColor = material.diffuseColor;
#ifdef USE_DIFFUSE_TEXTURE
    fragmentDiffuseColor *= vec3(texture(diffuseTexture, fragmentUv));
#endif

#ifdef USE_EMISSION_TEXTURE
    // Use emission texture as a color texture for now...
    fragmentDiffuseColor += vec3(texture(emissionTexture, fragmentUv));
#endif

    // Prepare specular color.
    vec3 fragmentSpecularColor = material.specularColor;
#ifdef USE_METALLIC_ROUGHNESS_TEXTURE
    vec3 fragmentMetallRoughness = texture(metallicRoughnessTexture, fragmentUv).rgb;
    fragmentSpecularColor *= vec3(1.0F - fragmentMetallRoughness.g);
#endif

    // Calculate total light received.
    for (int i = 0; i < LIGHT_COUNT; i++){
        color.xyz += calculateColorFromPointLight(vLightSources[i], fragmentNormalUnit, fragmentDiffuseColor, fragmentSpecularColor);
    }

    // Calculate environment reflection light.
    vec3 cameraToFragmentDirectionUnit = normalize(fragmentPosition - cameraPositionInWorldSpace);
    vec3 fragmentReflectionFromEyeDirectionUnit = reflect(cameraToFragmentDirectionUnit, fragmentNormalUnit);
    vec3 environmentColor = texture(environmentMap, fragmentReflectionFromEyeDirectionUnit).rgb;

#ifdef USE_METALLIC_ROUGHNESS_TEXTURE
    color.xyz += environmentColor * fragmentMetallRoughness.b * environmentIntensity;
#else
    color.xyz += environmentColor * environmentIntensity;
#endif
} 
