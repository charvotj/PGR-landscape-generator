#version 450 core

in vec2 vTexCoord;

uniform bool useTexture;
layout(binding=0)uniform sampler2D tex0;

void main()
{   
    if (useTexture)
    {
        vec4 objectColor = texture(tex0, vTexCoord);
        // Ignore transparent pixels in depth buffer - important when callculating shadows of semi-transparent billboards
        if(objectColor.a < 0.1)
            discard;
    }

    /* This line happens implicitly*/
    // gl_FragDepth = gl_FragCoord.z;
}  