#version 410

uniform samplerCube cubemap;

in vec3 fragPos;
out vec4 outColor;

void main()
{
	outColor = texture(cubemap, fragPos);
}