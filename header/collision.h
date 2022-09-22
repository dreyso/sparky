#pragma once
#include "vec.h"

#include <SDL.h>

#include <vector>


void convert(const SDL_FRect& a, SDL_Rect& b);
void convert(const SDL_Rect& a, SDL_FRect& b);

// Box collision detector
bool checkCollision(const SDL_Rect& a, const SDL_Rect& b);
bool checkCollision(const SDL_FRect& a, const SDL_Rect& b);
bool checkCollision(const SDL_FRect& a, const SDL_FRect& b);

// Finds the minimum vector needed to resolve all collisions on a axis aligned system
void buildCollisionReport(const SDL_FRect& entity, const std::vector<SDL_Rect>& collidedTiles, Vec& adjustPos);


