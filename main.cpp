#include <stdexcept>
#include <iostream>
#include <SDL2/SDL.h>

#include "engine/game.h"
#include "main.h"

int main(int argc, const char *argv[])
{
    try
    {
        init();
        Game game = Game();
        game.Run();
        cleanup();

        return EXIT_SUCCESS;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        cleanup();
        return EXIT_FAILURE;
    }
}

void init()
{
    // Initialize SDL
    std::cout << "Initializing SDL2..." << std::endl;
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
    {
        throw std::runtime_error("Failed to initialize SDL2: " + (std::string)SDL_GetError());
    }
}

void cleanup()
{
    std::cout << "Quitting SDL..." << std::endl;
    SDL_Quit();
}