#version 410

// The water color.
uniform vec3 meshColor;

// The distance at which the water starts to disappear / blend into distant water and terrain.
uniform float startFade;

// THe distance at which the water is completely invisible.
uniform float endFade;

// Input position in camera space (use for calculating level-of-detail transition fade).
in vec3 fragCamSpacePos;

// Output color
out vec4 outColor;

void main()
{
    // Level-of-detail transition fade.
    float alpha = clamp((endFade - length(fragCamSpacePos)) / (endFade - startFade), 0.0, 1.0);
    
    // Use constant color for the water.
    outColor = vec4(meshColor, alpha);
}
