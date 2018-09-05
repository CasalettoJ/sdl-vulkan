#ifndef GAME_H
#define GAME_H

#include <SDL2/SDL.h>
#include <vulkan/vulkan.h>

#include "renderer/renderer.h"

class Game
{
  public:
    Game();
    ~Game();
    void Run();

  private:
    Renderer _renderer;

    bool handleEvent(SDL_Event e);
    void update();
    void render();
};

#endif
