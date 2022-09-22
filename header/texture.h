#pragma once
#include "util.h"    // For texture smart pointer

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>


class Texture
{
public:
	Texture() = delete;
	// Can only be created inside Game::
	Texture(SDL_Renderer* defaultRenderer, std::string pathOrText, TTF_Font* defaultFont = nullptr, SDL_Color* textColor = nullptr);
	Texture(Texture&& other) noexcept;

	~Texture() = default;

	// Load texture from file
	bool loadTexture(std::string path);

	// Loads texture from text
	bool loadTextTexture(std::string textureText, SDL_Color textColor);

	// Set color modulation
	bool setColorModulation(Uint8 red, Uint8 green, Uint8 blue);

	// Set blending
	bool setBlendMode(SDL_BlendMode blending);

	// Set transparency
	bool setTransparency(Uint8 alpha);

	// Renders texture at given point
	bool draw(int x, int y, const SDL_Rect* clip = nullptr, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);

	// Gets texture dimensions
	int getWidth() const;
	int getHeight() const;

private:
	// Texture owned by this class
	TexturePtr mTexture;

	// Texture dimensions
	int mWidth;
	int mHeight;

	// Required for loading and drawing textures
	// These objects are not owned by Texture
	SDL_Renderer* mDefaultRenderer;
	TTF_Font* mDefaultFont;
};