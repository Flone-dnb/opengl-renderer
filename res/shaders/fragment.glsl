#include "base.glsl"

in vec2 fragmentUv;

#ifdef USE_DIFFUSE_TEXTURE
layout(binding = 0) uniform sampler2D diffuseTexture;
#endif

out vec4 color;

void main()
{
    color = vec4(1.0F, 1.0F, 1.0F, 1.0F);

#ifdef USE_DIFFUSE_TEXTURE
    color *= texture(diffuseTexture, fragmentUv);
#endif
} 
