#version 410

layout (location = 0) in vec3 position;

// model - view - projection
uniform mat4 mvp; 

// Untransformed, local-space position
out vec3 fragPos;

void main()
{
    fragPos = position;

    // Hack for putting the skybox a the far clipping plane
    gl_Position = (mvp * vec4(position, 1.0)).xyww;
}