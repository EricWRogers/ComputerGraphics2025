#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include "TinyGLTF.h"

// Shader sources
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

// Function prototypes
GLuint LoadShader(const char* source, GLenum type);
void CheckShaderLinking(GLuint shaderProgram);
void LoadModel(tinygltf::Model &model, const std::string &filename);
void InitializeBuffers(const tinygltf::Model &model, std::vector<GLuint> &vbos, std::vector<GLuint> &vaos);
void DrawModel(const tinygltf::Model &model, const std::vector<GLuint> &vaos);
void UpdateBoneTransforms(const tinygltf::Model &model, std::vector<glm::mat4> &boneTransforms, float animationTime, int animationIndex);

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("GLTF Animation",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          800, 600,
                                          SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr << "Failed to create SDL window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_GLContext glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        std::cerr << "Failed to create OpenGL context: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Enable vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        std::cerr << "Warning: Unable to set Vsync! SDL Error: " << SDL_GetError() << std::endl;
    }

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    GLuint vertexShader = LoadShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = LoadShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    CheckShaderLinking(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    tinygltf::Model model;
    LoadModel(model, "assets/CesiumMan.gltf");

    std::vector<GLuint> vbos;
    std::vector<GLuint> vaos(model.meshes.size());

    InitializeBuffers(model, vbos, vaos);

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    std::vector<glm::mat4> boneTransforms(100, glm::mat4(1.0f));

    bool quit = false;
    SDL_Event event;
    float animationTime = 0.0f;
    int animationIndex = 0;  // Change this to the index of the animation you want to play

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLuint bonesLoc = glGetUniformLocation(shaderProgram, "bones");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

        UpdateBoneTransforms(model, boneTransforms, animationTime, animationIndex);
        glUniformMatrix4fv(bonesLoc, boneTransforms.size(), GL_FALSE, glm::value_ptr(boneTransforms[0]));

        DrawModel(model, vaos);

        SDL_GL_SwapWindow(window);

        animationTime += 0.016f; // Advance the animation time by 1/60th of a second

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL error: " << err << std::endl;
        }
    }

    // Cleanup
    for (GLuint vbo : vbos) {
        glDeleteBuffers(1, &vbo);
    }
    for (GLuint vao : vaos) {
        glDeleteVertexArrays(1, &vao);
    }

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

GLuint LoadShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

void CheckShaderLinking(GLuint shaderProgram) {
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
}

void LoadModel(tinygltf::Model &model, const std::string &filename) {
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ret;
    if (filename.substr(filename.find_last_of(".") + 1) == "glb") {
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);  // Load GLB (binary)
    } else {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);  // Load GLTF (ASCII)
    }

    if (!ret) {
        std::cerr << "Failed to load GLTF/GLB: " << err << std::endl;
        if (!warn.empty()) {
            std::cerr << "Warning: " << warn << std::endl;
        }
        exit(EXIT_FAILURE);
    }
    if (!warn.empty()) {
        std::cerr << "Warning: " << warn << std::endl;
    }
}

void InitializeBuffers(const tinygltf::Model &model, std::vector<GLuint> &vbos, std::vector<GLuint> &vaos) {
    glGenBuffers(vbos.size(), vbos.data());
    glGenVertexArrays(vaos.size(), vaos.data());

    for (size_t i = 0; i < model.meshes.size(); ++i) {
        const tinygltf::Mesh &mesh = model.meshes[i];
        glBindVertexArray(vaos[i]);

        for (size_t j = 0; j < mesh.primitives.size(); ++j) {
            const tinygltf::Primitive &primitive = mesh.primitives[j];

            for (auto &attrib : primitive.attributes) {
                const tinygltf::Accessor &accessor = model.accessors[attrib.second];
                const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

                GLuint vbo;
                glGenBuffers(1, &vbo);
                vbos.push_back(vbo);

                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, buffer.data.size(), buffer.data.data(), GL_STATIC_DRAW);

                GLint size = 1;
                if (accessor.type == TINYGLTF_TYPE_SCALAR) size = 1;
                else if (accessor.type == TINYGLTF_TYPE_VEC2) size = 2;
                else if (accessor.type == TINYGLTF_TYPE_VEC3) size = 3;
                else if (accessor.type == TINYGLTF_TYPE_VEC4) size = 4;

                GLint attribLocation = -1;
                if (attrib.first == "POSITION") attribLocation = 0;
                else if (attrib.first == "NORMAL") attribLocation = 1;
                else if (attrib.first == "TEXCOORD_0") attribLocation = 2;
                else if (attrib.first == "JOINTS_0") attribLocation = 3;
                else if (attrib.first == "WEIGHTS_0") attribLocation = 4;

                if (attribLocation != -1) {
                    glEnableVertexAttribArray(attribLocation);

                    if (attrib.first == "JOINTS_0") {
                        glVertexAttribIPointer(attribLocation, size, accessor.componentType, bufferView.byteStride, (const void*)(accessor.byteOffset + bufferView.byteOffset));
                    } else {
                        glVertexAttribPointer(attribLocation, size, accessor.componentType, accessor.normalized ? GL_TRUE : GL_FALSE, bufferView.byteStride, (const void*)(accessor.byteOffset + bufferView.byteOffset));
                    }
                }
            }

            if (primitive.indices > -1) {
                const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView &bufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

                GLuint ebo;
                glGenBuffers(1, &ebo);
                vbos.push_back(ebo);

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer.data.size(), buffer.data.data(), GL_STATIC_DRAW);
            }
        }
    }

    glBindVertexArray(0);
}

void DrawModel(const tinygltf::Model &model, const std::vector<GLuint> &vaos) {
    for (size_t i = 0; i < model.meshes.size(); ++i) {
        glBindVertexArray(vaos[i]);

        for (size_t j = 0; j < model.meshes[i].primitives.size(); ++j) {
            const tinygltf::Primitive &primitive = model.meshes[i].primitives[j];

            // Draw the primitive
            glDrawElements(primitive.mode, model.accessors[primitive.indices].count, model.accessors[primitive.indices].componentType, (const void*)model.accessors[primitive.indices].byteOffset);
        }
    }

    glBindVertexArray(0);
}

void UpdateBoneTransforms(const tinygltf::Model &model, std::vector<glm::mat4> &boneTransforms, float animationTime, int animationIndex) {
    if (model.skins.empty() || model.animations.empty()) return;  // No skins or animations available

    const tinygltf::Skin &skin = model.skins[0];  // Assume a single skin for simplicity

    // Get inverse bind matrices
    const tinygltf::Accessor &accessor = model.accessors[skin.inverseBindMatrices];
    const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
    const float *data = reinterpret_cast<const float *>(&model.buffers[bufferView.buffer].data[bufferView.byteOffset + accessor.byteOffset]);

    std::vector<glm::mat4> inverseBindMatrices(accessor.count);
    for (size_t i = 0; i < accessor.count; ++i) {
        inverseBindMatrices[i] = glm::make_mat4(data + i * 16);
    }

    const tinygltf::Animation &animation = model.animations[animationIndex];

    // Apply animation to the bones
    for (const auto &channel : animation.channels) {
        const tinygltf::AnimationSampler &sampler = animation.samplers[channel.sampler];
        const tinygltf::Accessor &inputAccessor = model.accessors[sampler.input];
        const tinygltf::Accessor &outputAccessor = model.accessors[sampler.output];

        const float *inputBuffer = reinterpret_cast<const float *>(&model.buffers[model.bufferViews[inputAccessor.bufferView].buffer].data[inputAccessor.byteOffset + model.bufferViews[inputAccessor.bufferView].byteOffset]);
        const float *outputBuffer = reinterpret_cast<const float *>(&model.buffers[model.bufferViews[outputAccessor.bufferView].buffer].data[outputAccessor.byteOffset + model.bufferViews[outputAccessor.bufferView].byteOffset]);

        if (inputAccessor.count == 0 || outputAccessor.count == 0) {
            std::cerr << "Error: Empty animation input or output buffer" << std::endl;
            continue;
        }

        float startTime = inputBuffer[0];
        float endTime = inputBuffer[inputAccessor.count - 1];
        float time = fmod(animationTime, endTime - startTime);

        size_t prevIndex = 0;
        size_t nextIndex = 0;

        // Find keyframe indices
        for (size_t i = 0; i < inputAccessor.count - 1; ++i) {
            float t0 = inputBuffer[i];
            float t1 = inputBuffer[i + 1];
            if (time >= t0 && time <= t1) {
                prevIndex = i;
                nextIndex = i + 1;
                break;
            }
        }

        // Check for invalid time range
        if (prevIndex >= inputAccessor.count || nextIndex >= inputAccessor.count) {
            std::cerr << "Error: Invalid keyframe indices" << std::endl;
            continue;
        }

        // Interpolation
        float t0 = inputBuffer[prevIndex];
        float t1 = inputBuffer[nextIndex];
        float alpha = (t1 - t0 == 0) ? 0 : (time - t0) / (t1 - t0);

        if (channel.target_path == "translation") {
            glm::vec3 v0 = glm::make_vec3(outputBuffer + prevIndex * 3);
            glm::vec3 v1 = glm::make_vec3(outputBuffer + nextIndex * 3);
            glm::vec3 translation = glm::mix(v0, v1, alpha);
            boneTransforms[channel.target_node] = glm::translate(glm::mat4(1.0f), translation);

            // Debug output
            std::cout << "Bone " << channel.target_node << " translation:\n" << glm::to_string(translation) << std::endl;
        } else if (channel.target_path == "rotation") {
            glm::quat q0 = glm::make_quat(outputBuffer + prevIndex * 4);
            glm::quat q1 = glm::make_quat(outputBuffer + nextIndex * 4);
            glm::quat rotation = glm::slerp(q0, q1, alpha);
            boneTransforms[channel.target_node] *= glm::mat4_cast(rotation);

            // Debug output
            std::cout << "Bone " << channel.target_node << " rotation:\n" << glm::to_string(rotation) << std::endl;
        } else if (channel.target_path == "scale") {
            glm::vec3 s0 = glm::make_vec3(outputBuffer + prevIndex * 3);
            glm::vec3 s1 = glm::make_vec3(outputBuffer + nextIndex * 3);
            glm::vec3 scale = glm::mix(s0, s1, alpha);
            boneTransforms[channel.target_node] = glm::scale(glm::mat4(1.0f), scale);

            // Debug output
            std::cout << "Bone " << channel.target_node << " scale:\n" << glm::to_string(scale) << std::endl;
        }
    }

    // Combine with inverse bind matrices
    for (size_t i = 0; i < skin.joints.size(); ++i) {
        if (i >= boneTransforms.size() || i >= inverseBindMatrices.size()) {
            std::cerr << "Error: Index out of range for boneTransforms or inverseBindMatrices" << std::endl;
            continue;
        }
        boneTransforms[i] = boneTransforms[i] * inverseBindMatrices[i];
        // Debug output
        std::cout << "Bone " << i << " final transform:\n" << glm::to_string(boneTransforms[i]) << std::endl;
    }
}
