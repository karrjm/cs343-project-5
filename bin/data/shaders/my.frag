#version 410

in vec3 fragNormal;
in vec2 fragUV;

out vec4 outColor;

uniform vec3 lightDir;
uniform vec3 lightColor;
//uniform vec3 meshColor;

uniform sampler2D tex;

void main()
{
    vec3 normal = normalize(fragNormal);
    float nDotL = max(0, dot(normal, lightDir));

    // how much light the surface effectively receives
    vec3 irradiance = lightColor * nDotL;
    
    // gamma decoding
    vec4 meshColor = pow(texture(tex, fragUV), vec4(2.2, 2.2, 2.2, 1.0));

    if (meshColor.a < 0.1)
    {
        discard;
    }

    // how much light the surface reflects (physically)
    vec3 linearColor = meshColor.rgb * irradiance;

//    // gamma encoding
    outColor = vec4(pow(linearColor, vec3(1.0 / 2.2)), meshColor.a);

//    outColor = vec4(normal*.5+.5, 1);
}
