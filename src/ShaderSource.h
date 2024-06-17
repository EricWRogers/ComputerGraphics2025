#pragma once


const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in ivec4 boneIDs;
layout(location = 4) in vec4 weights;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bones[100]; // Assume a maximum of 100 bones for simplicity

void main() {
    mat4 boneTransform = bones[boneIDs[0]] * weights[0] +
                         bones[boneIDs[1]] * weights[1] +
                         bones[boneIDs[2]] * weights[2] +
                         bones[boneIDs[3]] * weights[3];

    vec4 pos = boneTransform * vec4(position, 1.0);
    gl_Position = projection * view * model * pos;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 color;

void main() {
    color = vec4(1.0, 0.5, 0.2, 1.0);
}
)";
