#version 460 core

in vec2 fragmentUv;

layout(binding = 0) uniform sampler2D screenTexture;
uniform float gamma;
uniform float exposure;

out vec4 color;

void main()
{
    // Set base color.
    color = vec4(texture(screenTexture, fragmentUv).rgb, 1.0F);

    // Apply exposure tone mapping.
    color.rgb = vec3(1.0F) - exp(-color.rgb * exposure);

    // Apply gamma correction.
    color.rgb = pow(color.rgb, vec3(1.0F/gamma));
}