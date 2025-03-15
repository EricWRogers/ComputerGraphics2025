#pragma once

#include "Entity.hpp"

#include "World.hpp"

class Background : public Entity {
public:
    void Start();
    void Update(float _dt);
    void Draw();
    void OnDestroy();

    float speed = 100.0f;
    glm::vec2 dir = glm::vec2(0.0f, 0.0f);
};