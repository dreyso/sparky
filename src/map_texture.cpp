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
    SDL_Rect crop{ static_cast<int>(roundf(camera.x)), static_cast<int>(roundf(camera.y)), static_cast<int>(roundf(camera.w)), static_cast<int>(roundf(camera.h)) };

    // Draw the map
    mMapTexture.draw(0, 0, &crop);
}