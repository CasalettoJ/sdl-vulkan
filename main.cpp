#include <stdexcept>
#include <iostream>
#include <SDL2/SDL.h>
#include "game.h"

int main(int argc, const char *argv[])
{
    try
    {
        // Initialize SDL
        std::cout << "Initializing SDL2..." << std::endl;
        if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
        {
            throw std::runtime_error("Failed to initialize SDL2: " + (std::string)SDL_GetError());
        }
        Game game = Game();
        game.Run();
        SDL_Quit();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}