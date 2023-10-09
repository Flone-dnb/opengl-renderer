#version 460 core 

in vec3 textureCoordinates;

out vec4 color;

uniform samplerCube environmentMap;

void main()
{
    color = texture(environmentMap, textureCoordinates);
}