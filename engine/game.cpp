#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <functional>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <string>

#include "game.h"

Game::Game()
{
    // Initialize SDL
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        throw "Failed to initialize SDL2: " + (std::string)SDL_GetError();
    }

    // Create SDL Window with Vulkan
    window = SDL_CreateWindow("SDL2 - Vulkan", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);
    if (window == nullptr)
    {
        throw "Failed to create SDL Window: " + (std::string)SDL_GetError();
    }

    // Initialize Vulkan
}

Game::~Game()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::initVulkan() {}

void Game::mainLoop()
{
    std::cout << "Running Game" << std::endl;
    bool _quit = false;
    SDL_Event e;
    while (true)
    {
        while (SDL_PollEvent(&e))
        {
            bool shouldQuit = !handleEvent(e);
            _quit = _quit ? _quit : shouldQuit;
        }
        if (_quit)
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
    }
    return true;
}

void Game::update() {
    // Update Logic Here
}

void Game::render() {}

void Game::cleanup() {}
