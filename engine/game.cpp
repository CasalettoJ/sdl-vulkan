#include <SDL2/SDL.h>
#include <SDL2/SDL_Vulkan.h>
#include <vulkan/vulkan.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <string>
#include <vector>

#include "game.h"
#include "renderer/renderer.h"

Game::Game()
{
}

Game::~Game()
{
}

void Game::Run()
{
    std::cout << "Running Game" << std::endl;
    bool quit = false;
    SDL_Event e;
    while (true)
    {
        while (SDL_PollEvent(&e))
        {
            bool shouldQuit = !handleEvent(e);
            quit = quit ? quit : shouldQuit;
        }
        if (quit)
        {
            break;
        }
        update();
        render();
    }
}

bool Game::handleEvent(SDL_Event e)
{
    switch (e.type)
    {
    case SDL_QUIT:
        return false;
    case SDL_WINDOWEVENT:
        switch (e.window.event)
        {
            case SDL_WINDOWEVENT_RESIZED:
                _renderer.RecreateSwapchain();
                break;
        }
        break;
    }
    return true;
}

void Game::update()
{
    // Update Logic Here
}

void Game::render()
{
    // Render Logic Here
    _renderer.DrawFrame();
}
