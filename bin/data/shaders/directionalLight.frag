#version 410

// The direction of the sun
uniform vec3 lightDir;

// The color of the sun
uniform vec3 lightColor;

// The ambient light color.
uniform vec3 ambientColor;

// The color of the robot
uniform vec3 meshColor;

// 1/gamma; used for gamma correction
uniform float gammaInv;

// The distance at which the robot should start to fade away
uniform float startFade;

// The distance at which the robot should be completely invisible
uniform float endFade;

// Input surface normal
in vec3 fragNormal;

// Input position in camera space (use for calculating fade-out).
in vec3 fragCamSpacePos;

// Output color
out vec4 outColor;

void main()
{
    // Re-normalize the normal vector after rasterization.
    vec3 normal = normalize(fragNormal);

    // Calculate diffuse lighting.
    float lightAmount = max(0.0, dot(normal, lightDir));
    vec3 fragLight = lightColor * lightAmount + ambientColor;
    
    // Calculate fade-out.
    float alpha = clamp((endFade - length(fragCamSpacePos)) / (endFade - startFade), 0.0, 1.0);

    // Apply lighting and gamma correction.
    outColor = vec4(pow(meshColor * fragLight, vec3(gammaInv)), alpha);
}
