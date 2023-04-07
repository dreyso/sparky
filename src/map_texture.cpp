#include "../header/map_texture.h"

#include <SDL.h>

MapTexture::MapTexture(SDL_Renderer* defaultRenderer, const char* pathToSVG) : mMapTexture{ defaultRenderer, pathToSVG } {}

int MapTexture::getTextureWidth() const
{
    return mMapTexture.getWidth();
}

int MapTexture::getTextureHeight() const
{
    return mMapTexture.getHeight();
}


void MapTexture::draw(const SDL_FRect& camera)
{
    // Determine which portion of the texture to draw
    SDL_FRect crop{ camera.x, camera.y, camera.w, camera.h };

    // Draw the map
    mMapTexture.draw(0, 0, &crop);
}