#pragma once
#include "texture.h"

#include <SDL.h>


class MapTexture
{
public:
    MapTexture() = delete;
    MapTexture(SDL_Renderer* defaultRenderer, const char* pathToSVG);
    ~MapTexture() = default;

    int getTextureWidth() const;
    int getTextureHeight() const;

    // Render the texture present in the camera
    void draw(const SDL_FRect& camera);

private:
    // SVG file loaded as an SDL texture
    Texture mMapTexture;
};