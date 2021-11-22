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

in vec2 fragUV;
in mat3 TBN;

uniform sampler2D diffuseTex;
uniform sampler2D normalTex;

void main()
{
    // Re-normalize the normal vector after rasterization.
    // vec3 normal = normalize(fragNormal);
    vec3 diffuseColor = pow(texture(diffuseTex, fragUV  * 0.05).rgb, vec3(2.2)); // = normalize(fragNormal);

    vec3 tsNormal = texture(normalTex, fragUV * 0.05).xyz * 2 - 1;
    vec3 wsNormal = TBN * tsNormal;

    // Calculate diffuse lighting.
    float lightAmount = max(0.0, dot(wsNormal, lightDir));
    vec3 fragLight = lightColor * lightAmount + ambientColor;
    
    // Calculate fade-out.
    float alpha = clamp((endFade - length(fragCamSpacePos)) / (endFade - startFade), 0.0, 1.0);

    // Apply lighting and gamma correction.
    outColor = vec4(pow(diffuseColor.rgb * fragLight, vec3(gammaInv)), alpha);
    
    //outColor = vec4(TBN[0] * 0.5 + 0.5, 1);
}
