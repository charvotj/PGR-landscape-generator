#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoords;
/* Instance data*/
layout (location = 2) in mat4 iModel;

uniform mat4 lightViewProj = mat4(1);

out vec2 vTexCoord;

void main()
{
    vTexCoord = texCoords;
    gl_Position = lightViewProj * iModel * vec4(aPos, 1.0);
}
