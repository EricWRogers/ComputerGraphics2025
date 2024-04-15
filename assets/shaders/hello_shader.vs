#version 330 core

in vec3 aPosition;
in vec3 aNormal;
in vec2 aUV;

out vec2 fragmentUV;
out vec3 fragmentPos;
out vec3 fragmentNormal;

uniform mat4 TRANSFORM;
uniform mat4 VIEW;
uniform mat4 PROJECTION;

void main()
{
    fragmentPos = vec3(TRANSFORM * vec4(aPosition, 1.0));
    fragmentNormal = aNormal;
    fragmentUV = aUV;
    gl_Position = PROJECTION * VIEW * vec4(fragmentPos, 1.0);
}