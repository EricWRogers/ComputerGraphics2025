#include "Ball.hpp"
#include "Paddle.hpp"
#include <SDL.h>

using namespace glm;

extern int leftScore;
extern int rightScore;

void Ball::Start() 
{
    name = "Ball";
    position = vec3(window->GetScreenWidth() * 0.5f, window->GetScreenHeight() * 0.5f, 0.0f);
    scale = vec3(100.0f, 100.0f, 0.0f);
}

void Ball::Update(float _dt) 
{
    if (inputManager->GetKey(SDL_SCANCODE_SPACE))
    {
        isMoving = true;
    }
    if (isMoving)
    {
        // OBJECT MOVING
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        cursorPosition = glm::vec2(mouseX, mouseY);
        cursorPosition.y = window->GetScreenHeight() - cursorPosition.y; // correct y position

        vec2 direction = cursorPosition - glm::vec2(position.x, position.y);

        dir = glm::normalize(direction);
        position += vec3(dir.x, dir.y, 0.0f) * speed * _dt;

    }

    if (dir == vec2(0.0f))
    {
        if (inputManager->GetKey(SDL_SCANCODE_SPACE))
        {
            vec2 directions[] = {vec2(1.0f, 1.0f), vec2(1.0f, -1.0f), vec2(-1.0f, 1.0f), vec2(-1.0f, -1.0f)};
            dir = directions[rand()%4];
        }
    }
    
    if (position.y > window->GetScreenHeight() - (scale.y * 0.5f)) {
        position.y = window->GetScreenHeight() - (scale.y * 0.5f);
        dir.y = abs(dir.y) * -1.0f;
    }
    if (position.y < scale.y * 0.5f) {
        position.y = scale.y * 0.5f;
        dir.y = abs(dir.y);
    }

    // detect score
    if (position.x > window->GetScreenWidth() - (scale.x * 0.5f)) {
        position = vec3(window->GetScreenWidth()*0.5f, window->GetScreenHeight()*0.5f, 0.0f);
        dir = vec2(0.0f);
        rightScore++;
    }
    if (position.x < scale.x * 0.5f) {
        position = vec3(window->GetScreenWidth()*0.5f, window->GetScreenHeight()*0.5f, 0.0f);
        dir = vec2(0.0f);
        leftScore++;

    }

    // detect if ball hits left paddle
    Paddle* leftPaddle = world->FindByName<Paddle>("LeftPaddle"); 
    if (EntityOverlap2D(*this ,*leftPaddle)) {
        dir.x = abs(dir.x);
    }

    // detect if ball hits right paddle
    Paddle* rightPaddle = world->FindByName<Paddle>("RightPaddle"); 
    if (EntityOverlap2D(*this ,*rightPaddle)) {
        dir.x = abs(dir.x) * -1.0f;
    }

    if (dir != vec2(0.0f))
        position += vec3(dir.x, dir.y, 0.0f) * speed * _dt;
}

void Ball::Draw() {mat4 transform = mat4(1.0f);
    transform = translate(transform, position);
    transform = glm::scale(transform, scale);

    // set shader variables
    shader.SetVec4("COLOR", color);
    shader.SetMat4("TRANSFORM", transform);
    shader.SetBool("useTexture", false);
    shader.SetInt("texture1", 0);
    shader.SetBool("isScrolling", false);
}

void Ball::OnDestroy() {
    
}