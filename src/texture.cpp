#include "../header/texture.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <string>
#include <cmath>
#include <stdexcept>

#define TO_INT(floatVal) (static_cast<int>(round(floatVal)))


Texture::Texture(SDL_Renderer* defaultRenderer, std::string pathOrText, TTF_Font* defaultFont, SDL_Color* textColor)
{
	mDefaultRenderer = defaultRenderer;
	mDefaultFont = defaultFont;

	// If font is provided, text texture is assumed
	if (defaultFont == nullptr)
	{
		if (loadTexture(pathOrText) == false)
			exit(-1);
	}
	else
	{
		if(loadTextTexture(pathOrText, *textColor) == false)
			exit(-1);
	}

	// Get texture dimensions
	if (SDL_QueryTexture(mTexture.get(), nullptr, nullptr, &mWidth, &mHeight) != 0)
	{ 
		fprintf(stderr, "%s", SDL_GetError());
		exit(-1);
	}
}

Texture::Texture(Texture&& texture) noexcept
	: mWidth{ texture.mWidth }, mHeight{ texture.mHeight }, mTexture{ std::move(texture.mTexture) }, mDefaultRenderer{ texture.mDefaultRenderer }, mDefaultFont{ texture.mDefaultFont }
{
	// Reset passed in texture
	texture.mWidth = 0;
	texture.mHeight = 0;
	texture.mDefaultRenderer = nullptr;
	texture.mDefaultFont = nullptr;
}

// Load texture from file
bool Texture::loadTexture(std::string path)
{
	// Create new texture
	mTexture.reset(IMG_LoadTexture(mDefaultRenderer, path.c_str()));
	if (mTexture == nullptr)
	{ 
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	return true;
}

// Loads texture from text
bool Texture::loadTextTexture(std::string textureText, SDL_Color textColor)
{
	// Create surface
	SDL_Surface* textSurface = TTF_RenderText_Blended(mDefaultFont, textureText.c_str(), textColor);
	if (textSurface == nullptr)
	{
		fprintf(stderr, "%s", TTF_GetError());
		return false;
	}

	// Create texture from surface
	mTexture.reset(SDL_CreateTextureFromSurface(mDefaultRenderer, textSurface));

	// Free surface
	SDL_FreeSurface(textSurface);

	if (mTexture == nullptr)
	{
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	return true;
}

// Set color modulation
bool Texture::setColorModulation(Uint8 red, Uint8 green, Uint8 blue)
{
	if (SDL_SetTextureColorMod(mTexture.get(), red, green, blue) != 0)
	{
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	return true;
}

// Set texture blend mode
bool Texture::setBlendMode(SDL_BlendMode blending)
{
	if (SDL_SetTextureBlendMode(mTexture.get(), blending) != 0)
	{
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	return true;
}

// Set transparency
bool Texture::setTransparency(Uint8 alpha)
{
	if (SDL_SetTextureAlphaMod(mTexture.get(), alpha) != 0)
	{
		fprintf(stderr, "%s", SDL_GetError());
		return false;
	}

	return true;
}

// Draws texture at given point
void Texture::draw(float x, float y, const SDL_FRect* crop, double angle, SDL_FPoint* center, SDL_RendererFlip flip)
{
	// Set drawing area
	SDL_Rect intCrop;
	SDL_FRect renderArea{ x, y, static_cast<float>(mWidth), static_cast<float>(mHeight) };

	// If cropping texture, set the render area's dimensions to the crop's
	if (crop != nullptr)
	{
		intCrop = SDL_Rect{ TO_INT(crop->x), TO_INT(crop->y), TO_INT(crop->w), TO_INT(crop->h) };

		// -- Tailor the crop -----------------------------------------

		/*
		* SDL crops textures without any extra space. I.e, if a
		* crop includes an entire texture and some space around it,
		* it will only grab the texture, no extra space. This will
		* then be rendered to some space, and if the space has different
		* dimensions from the cropped texture, the image will be distorted.
		* Hence, the crop should be "tailored" and the rendering area shifted
		* to simulate including the space in the crop.
		*/

		SDL_FRect textureDimensions{ 0.f, 0.f, static_cast<float>(mWidth), static_cast<float>(mHeight) };
		SDL_FRect tailoredCrop{ 0.f, 0.f, 0.f, 0.f };
		
		if (!SDL_IntersectFRect(&textureDimensions, crop, &tailoredCrop))
		{
			fprintf(stderr, "Cropped area does not intersect with texture to be drawn\n");
			return;
		}

		renderArea.w = tailoredCrop.w;
		renderArea.h = tailoredCrop.h;

		// Shift render area to include space in the crop area
		if (crop->x < 0)
			renderArea.x -= crop->x;
		if (crop->y < 0)
			renderArea.y -= crop->y;
	}
	
	// Draw texture
	if (SDL_RenderCopyExF(mDefaultRenderer, mTexture.get(), crop == nullptr ? nullptr : &intCrop, &renderArea, angle, center, flip) != 0)
		throw(std::runtime_error{ SDL_GetError() });
}
// Get texture width
int Texture::getWidth() const{	return mWidth;	}

// Get texture height
int Texture::getHeight() const{	return mHeight;	}