#version 450 core

in vec2 vTexCoord;
in vec3 FragPos;
in vec4 FragPosLightSpace;

uniform bool useTexture;
layout(binding=0)uniform sampler2D tex0;
layout(binding=1)uniform sampler2D shadowMap;
uniform vec3 diffuseColor;

out vec4 fragColor;

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    float bias = 0.001;

    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    int halfSize = 1;
    for(int x = -halfSize; x <= halfSize; ++x)
    {
        for(int y = -halfSize; y <= halfSize; ++y)
        {
            // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;        
        }    
    }
    int fullSize = 2*halfSize+1;
    shadow /= (fullSize*fullSize);

    return shadow;
}

void main()
{
    vec4 objectColor;
    if (useTexture)
    {
        objectColor = texture(tex0, vTexCoord);
        // Discard transparent fragments - don't write even into depth buffer - important for billboards
        if(objectColor.a < 0.1)
            discard;
    }
    else
        objectColor = vec4(diffuseColor, 1.0);

    // -----------------------------
    // Global illumination
    // ----------------------------- 
    vec3 skyColor = vec3(0.73, 0.63, 0.71); // Calculated from skybox texture
    vec3 groundColor = vec3(0.2, 0.1, 0.1); // Brown
    float giIntensity = 0.6;

    vec3 normal = normalize(cross(dFdx(FragPos), dFdy(FragPos)));
    float hemiMix = normal.y * 0.5 + 0.5;
    vec3 ambient = mix(groundColor, skyColor, hemiMix);
    vec4 finalColor = objectColor * vec4(ambient * giIntensity, 1.0);
    // Gamma correction
    finalColor = pow(finalColor, vec4(1.0/2.2));

    // -----------------------------
    // Shadow mapping
    // ----------------------------- 
    float shadow = ShadowCalculation(FragPosLightSpace);
    fragColor = vec4(vec3(finalColor) * (1.0 - 0.6 * shadow), finalColor[3]);
}