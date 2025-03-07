#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aUV;
out vec3 color;
out vec2 uv;
uniform float TIME;
void main()
{
   gl_Position = vec4(aPos.x + sin(TIME)*0.5f, aPos.y + cos(TIME)*0.5f, aPos.z, 1.0);
   uv = aUV;
}