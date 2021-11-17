#version 410

layout (location = 0) in vec3 position;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

out vec3 fragNormal;
out vec2 fragUV;

uniform mat3 normalMatrix;
uniform mat4 mvp; 

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
    fragNormal = normalMatrix * normal;
    fragUV = 4 * uv;
}
