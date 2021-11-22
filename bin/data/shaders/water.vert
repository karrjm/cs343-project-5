#version 410

layout (location = 0) in vec3 pos;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

// Needs both model-view-projection and model-view; model-view is used for level-of-detail transition fade.
uniform mat4 mvp;
uniform mat4 modelView;
uniform mat3 normalMatrix;

// Pass-through normal and UV.
out vec3 fragNormal;
out vec2 fragUV;

// For level-of-detail transition fade
out vec3 fragCamSpacePos;

void main()
{
    // Standard model-view-projection transformation.
    gl_Position = mvp * vec4(pos, 1.0);
    
    // Pass-through normal and UV.
    fragNormal = normalize(normalMatrix * normal);
    fragUV = vec2(uv[0], 1 - uv[1]);

    // For level-of-detail transition fade
    fragCamSpacePos = (modelView * vec4(pos, 1.0)).xyz;
}
