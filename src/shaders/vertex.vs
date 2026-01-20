#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoords;
/* Instance data*/
layout (location = 2) in mat4 iModel;

uniform mat4 viewProj = mat4(1);
uniform mat4 lightViewProj = mat4(1);

out vec2 vTexCoord;
out vec3 FragPos;
out vec4 FragPosLightSpace;

void main()
{
    vTexCoord = texCoords;
    FragPos = vec3(iModel * vec4(aPos, 1.0));
    FragPosLightSpace = lightViewProj * vec4(FragPos, 1.0);
    
    gl_Position = viewProj * vec4(FragPos, 1.0);
}
