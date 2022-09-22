#include "../../header/components/texture_component.h"
#include "../../header/components/mechanical_component.h"
#include "../../header/texture.h"
#include "../../header/entity.h"

#include <SDL.h>

#include <stdexcept>


TextureComponent::TextureComponent(Entity* owner, const SDL_FRect* camera, Texture&& texture) : Component{ owner }, mTexture{ std::move(texture) }
{
	initCamera(camera);

	// Count this object
	++mObjectCount;
}

TextureComponent::TextureComponent(Entity* owner, const SDL_FRect* camera, SDL_Renderer* defaultRenderer, std::string pathOrText, TTF_Font* defaultFont, SDL_Color* textColor)
	: Component{ owner }, mTexture{ defaultRenderer, pathOrText, defaultFont, textColor }
{
	initCamera(camera);

	// Count this object
	++mObjectCount;
}

TextureComponent::~TextureComponent()
{
	if (mObjectCount == 1)	// If this is the last object
	{
		mCamera = nullptr;	// Remove static pointer to camera
	}

	// Remove this object from the count
	--mObjectCount;
}

void TextureComponent::draw()
{
	// Get collision box from mechanical component
	const SDL_FRect& collisionBox = mOwner->getComponent<MechanicalComponent>().getCollisionBox();
	
	float textureWidthOffset = (static_cast<float>(mTexture.getWidth()) - collisionBox.w) / 2.f;
	float textureHeightOffset = (static_cast<float>(mTexture.getHeight()) - collisionBox.h) / 2.f;

	// Render the entity relative to the camera
	int screenPosX = static_cast<int>(roundf((collisionBox.x - mCamera->x) - textureWidthOffset));
	int screenPosY = static_cast<int>(roundf((collisionBox.y - mCamera->y) - textureHeightOffset));
	mTexture.draw(screenPosX, screenPosY, nullptr, mOwner->getComponent<MechanicalComponent>().getRotationAngle());
}

void TextureComponent::initCamera(const SDL_FRect* camera)
{

	if (mObjectCount == 0)	// First object of this class type
	{
		if (camera == nullptr)	// Camera required
			throw(std::runtime_error{ "Error: Attempted to create texture component without a camera\n" });
		else
			mCamera = camera;	// Set the camera
	}
	else	// Not first object
	{
		if (mCamera != camera)	// Passed in camera must match currently set camera
			throw(std::runtime_error{ "Error: Attempted to change texture component class' camera\n" });
	}
}