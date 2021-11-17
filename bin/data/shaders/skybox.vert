#version 410

layout (location = 0) in vec3 position;

uniform mat4 mvp; 

out vec3 fragPos;

void main()
{
    fragPos = position;
    gl_Position = mvp * vec4(position, 1.0);
}