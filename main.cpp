#include <stdexcept>
#include <iostream>
#include "game.h"

int main(int argc, const char * argv[]) 
{
    Game game;

    try 
    {
        game.run();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}