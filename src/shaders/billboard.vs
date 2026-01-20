#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 texCoords;

uniform mat4 viewProj = mat4(1);

out vec2 vTexCoord;
out vec3 FragPos;

void main()
{
    vTexCoord = texCoords;
    FragPos = vec3(vec4(aPos, 1.0));

    gl_Position = viewProj * vec4(FragPos, 1.0);
}
