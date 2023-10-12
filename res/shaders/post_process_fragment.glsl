#version 460 core

in vec2 fragmentUv;

layout(binding = 0) uniform sampler2D screenTexture;
uniform float gamma;

out vec4 color;

void main()
{
    // Set base color.
    color = texture(screenTexture, fragmentUv);

    // Apply gamma correction.
    color.rgb = pow(color.rgb, vec3(1.0F/gamma));
}