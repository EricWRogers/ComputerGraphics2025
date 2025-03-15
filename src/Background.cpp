#include "Background.hpp"

using namespace glm;


void Background::Start() {
    name = "Background";
    position = vec3(window->GetScreenWidth() * 0.5f, window->GetScreenHeight() * 0.5f, -1.0f);
    scale = vec3(window->GetScreenWidth(), window->GetScreenHeight(), 1.0f);
}

void Background::Update(float _dt) {


}

void Background::Draw(){
    mat4 transform = mat4(1.0f);
    transform = translate(transform, position);
    transform = glm::scale(transform, scale);

    // set shader variables
    shader.SetVec4("COLOR", color);
    shader.SetMat4("TRANSFORM", transform);
    shader.SetBool("useTexture", true);
    shader.SetBool("isScrolling", true);
    shader.SetInt("texture2", 1);
    
}

void Background::OnDestroy() {
    std::cout << "Background Destroyed" << std::endl;
}