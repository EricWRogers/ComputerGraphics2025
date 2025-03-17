#ifdef _WIN32
#include <windows.h>
#endif

#include <iostream>

#include <SDL.h>
#include <glm/glm.hpp>
#include <GL/glew.h>

#include "Canis/Canis.hpp"
#include "Canis/IOManager.hpp"
#include "Canis/FrameRateManager.hpp"

#include "Entity.hpp"
#include "Ball.hpp"
#include "Paddle.hpp"
#include "Background.hpp"

// git restore .
// git fetch
// git pull

// score tracking variables
int leftScore = 0; 
int rightScore = 0;

// move out to external class
unsigned int vertexShader;
unsigned int VBO, VAO, EBO;

void InitModel();

#ifdef _WIN32
#define main SDL_main
extern "C" int main(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    Canis::Init();
    Canis::Window window;

    // *** CHANGES - UPDATED to dynamically change window size based on display size so it works with my 1440p
    SDL_DisplayMode displayMode;
    if (SDL_GetCurrentDisplayMode(0, &displayMode) == 0) {

        float windowSize = 640;
        float windowHeight = (windowSize * ((float)displayMode.h / 1080.0f));
        float windowWidth  = (windowSize * ((float)displayMode.w / 1920.0f));

        window.Create("Computer Graphics 2025", windowWidth, windowHeight, 0);

    }
    // *** END OF CHANGES


    Canis::InputManager inputManager;
    Canis::FrameRateManager frameRateManager;
    frameRateManager.Init(60.0f);
    float deltaTime = 0.0f;
    float fps = 0.0f;

    Canis::Shader spriteShader;
    spriteShader.Compile("assets/shaders/sprite.vs", "assets/shaders/sprite.fs");
    spriteShader.AddAttribute("aPos");
    spriteShader.AddAttribute("aUV");
    spriteShader.Link();

    InitModel();

    //Canis::GLTexture texture = Canis::LoadImageGL("assets/textures/ForcePush.png", true);
    Canis::GLTexture texture1 = Canis::LoadImageGL("assets/textures/TilePattern.png", true);

    int textureSlots = 0;

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureSlots);

    Canis::Log(std::to_string(textureSlots));

    spriteShader.SetInt("texture1", 0);
    spriteShader.SetInt("texture2", 1);

    //glActiveTexture(GL_TEXTURE0 + 0);
    //glBindTexture(GL_TEXTURE_2D, texture.id);

    glActiveTexture(GL_TEXTURE1);                    
    glBindTexture(GL_TEXTURE_2D, texture1.id);


    World world;
    world.VAO = VAO;
    world.window = &window;
    world.inputManager = &inputManager;

    {
        Background *background = world.Instantiate<Background>();
        background->shader = spriteShader;
        background->texture = texture1;
        background->shader.SetBool("useTexture", true);
        background->shader.SetBool("isScrolling", true);
    
    }
    
    {
        Ball *ball = world.Instantiate<Ball>();
        ball->shader = spriteShader;
        ball->name = "Ball";
        ball->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    {
        Paddle *paddle = world.Instantiate<Paddle>();
        paddle->shader = spriteShader;
        //paddle->texture = texture;
        paddle->name = "RightPaddle";
        paddle->position = glm::vec3(window.GetScreenWidth() - (10.0f*0.5f), window.GetScreenHeight() * 0.5f, 0.0f);
        paddle->color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
        paddle->shader.SetBool("isScrolling", false);
    }

    {
        Paddle *paddle = world.Instantiate<Paddle>();
        paddle->shader = spriteShader;
        //paddle->texture = texture;
        paddle->name = "LeftPaddle";
        paddle->position = glm::vec3(10.0f*0.5f, window.GetScreenHeight() * 0.5f, 0.0f);
        paddle->color = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // Blue
        paddle->shader.SetBool("isScrolling", false);
    }

    while (inputManager.Update(window.GetScreenWidth(), window.GetScreenHeight()))
    {
        deltaTime = frameRateManager.StartFrame();
        glClearColor( 1.0f, 1.0f, 1.0f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT);

        using namespace glm;

        mat4 projection = ortho(0.0f, (float)window.GetScreenWidth(), 0.0f, (float)window.GetScreenHeight(), 0.001f, 100.0f);
        
        mat4 view = mat4(1.0f);
        view = translate(view, vec3(0.0f, 0.0f, 0.5f));
        view = inverse(view);

        world.Update(view, projection, deltaTime);

        // update the window title bar with the current score
        window.SetWindowName("Pong - Score: Blue " + std::to_string(rightScore) + " | Red " + std::to_string(leftScore));

        if (leftScore >= 5) 
        {
            break;
        }
        if (rightScore >= 5)
        {
            break;
        }

        window.SwapBuffer();

        fps = frameRateManager.EndFrame();
    }

    return 0;
}

void InitModel()
{
    float vertices[] = {
        // position         // uv
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,  // top left
    };

    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3};

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}