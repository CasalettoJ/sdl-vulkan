#ifndef GAME_CONSTANTS_H
#define GAME_CONSTANTS_H

#include <glm/glm.hpp>
#include <vector>

#include "renderer/vertex.h"

const std::vector<Vertex::Vertex> TRIANGLE_VERTICES = {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
};


#endif