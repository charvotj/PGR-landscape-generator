#version 450 core

layout (location = 0) in vec3 aPos;

uniform mat4 view = mat4(1);
uniform mat4 proj = mat4(1);

out vec3 vTexCoord;

void main()
{
    vTexCoord = aPos;
    // Take only rotation from view
    vec4 pos = proj * mat4(mat3(view)) * vec4(aPos, 1.0);
    gl_Position = pos.xyww; // w-w trick → skybox is infinitely far away
}
