#include "../../header/components/texture_component.h"
#include "../../header/components/mechanical_component.h"
#include "../../header/texture.h"
#include "../../header/entity.h"
#include "../../header/polygon.h"
#include "../../header/vec.h"


#include <SDL.h>

#include <stdexcept>


TextureComponent::TextureComponent(Entity* owner, const SDL_FRect& camera, Texture&& texture) : Component{ owner }, mTexture{ std::move(texture) }, mCamera{ &camera }
{
	initOffset();
}

TextureComponent::TextureComponent(Entity* owner, const SDL_FRect& camera, SDL_Renderer& defaultRenderer, std::string pathOrText, TTF_Font* defaultFont, SDL_Color* textColor)
	: Component{ owner }, mTexture{ &defaultRenderer, pathOrText, defaultFont, textColor }, mCamera{ &camera }
{
	initOffset();
}

void TextureComponent::initOffset()
{
	// Get collision box from mechanical component
	auto& collisionBox = mOwner->getComponent<MechanicalComponent>().getCollisionBox();

	// Get the rectangular hull of the polygon (will act as the dest rect of the texture)
	auto rect = collisionBox.getRectHull();

	// Store the relative location of the upper-left corner of the texture relative to entity position
	mOffsetFromPos = rect.getPos() - collisionBox.getPos();
}

void TextureComponent::draw()
{
	// Get collision box from mechanical component
	auto& collisionBox = mOwner->getComponent<MechanicalComponent>().getCollisionBox();
	auto& entityPos = collisionBox.getPos();
	auto drawPos = entityPos + mOffsetFromPos;

	// Render the entity relative to the camera
	int screenPosX = static_cast<int>(roundf(drawPos.getX() -  mCamera->x));
	int screenPosY = static_cast<int>(roundf(drawPos.getY() - mCamera->y));

	// Center of rotation
	SDL_Point center{ static_cast<int>(roundf(entityPos.getX())),  static_cast<int>(roundf(entityPos.getY())) };

	// Draw
	mTexture.draw(screenPosX, screenPosY, nullptr, collisionBox.getRotAngle(), &center);
}