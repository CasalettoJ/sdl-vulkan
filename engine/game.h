#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>

class Game {
public:
    Game();
    ~Game();
    void run() 
    {
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    const int WIDTH = 800, HEIGHT = 600;
    SDL_Window *window;

    void initVulkan();
    void mainLoop();
    bool handleEvent(SDL_Event e);
    void update();
    void render();
    void cleanup();
};

#endif
