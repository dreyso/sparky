#include "../../header/components/texture_component.h"
#include "../../header/components/mechanical_component.h"
#include "../../header/texture.h"
#include "../../header/entity.h"
#include "../../header/polygon.h"
#include "../../header/vec.h"


#include <SDL.h>

#include <stdexcept>

// If the mesh has not been moved, then its position should indicate where the texture should be drawn to align
// with the mesh
TextureComponent::TextureComponent(Entity* owner, const SDL_FRect& camera, Texture&& texture) 
	: Component{ owner }, mTexture{ std::move(texture) }, 
	mOffsetFromPos{ -1 * GET_MESH(mOwner).getPos() }, mCamera{ &camera }{}

TextureComponent::TextureComponent(Entity* owner, const SDL_FRect& camera, SDL_Renderer& defaultRenderer, std::string pathOrText, TTF_Font* defaultFont, SDL_Color* textColor)
	: Component{ owner }, mTexture{ &defaultRenderer, pathOrText, defaultFont, textColor },  
	mOffsetFromPos{ -1 * GET_MESH(mOwner).getPos() }, mCamera{ &camera }{}

void TextureComponent::draw()
{
	// Get collision mesh from mechanical component
	auto& collisionMesh = GET_MESH(mOwner);
	auto& entityPos = collisionMesh.getPos();

	// Render the entity relative to the camera
	int drawPosX = TO_INT(entityPos.getX() + mOffsetFromPos.getX() -  mCamera->x);
	int drawPosY = TO_INT(entityPos.getY() + mOffsetFromPos.getY() - mCamera->y);

	// Center of rotation (relative to texture position)
	SDL_Point center{ TO_INT(-mOffsetFromPos.getX()),  TO_INT(-mOffsetFromPos.getY()) };

	// Draw
	mTexture.draw(drawPosX, drawPosY, nullptr, static_cast<double>(collisionMesh.getRotAngle()), &center);
}