#version 450 core


in vec3 color;
in vec2 vTexCoord;
in vec3 FragPos;

uniform bool useTexture;
uniform sampler2D tex0;
uniform vec3 diffuseColor;

out vec4 fragColor;


void main()
{
    vec3 objectColor;
    if (useTexture)
    {
        vec4 texColor = texture(tex0, vTexCoord);
        objectColor = texColor.rgb;    

    }
    else
        objectColor = diffuseColor;

    fragColor = vec4(objectColor, 1.0);
}