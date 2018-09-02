#include <stdexcept>
#include <iostream>
#include "game.h"

int main(int argc, const char * argv[]) 
{
    Game game;

    try 
    {
        game.Run();
    } 
    catch (const std::exception& e) 
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}