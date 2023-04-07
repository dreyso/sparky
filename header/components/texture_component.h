#pragma once
#include "Component.h"
#include "mechanical_component.h"
#include "../texture.h"
#include "../entity.h"
#include "../vec.h"
#include "../util.h"

#include <SDL.h>

#define GET_MESH(mOwner) (mOwner->getComponent<MechanicalComponent>().getCollisionMesh())

class TextureComponent : public Component
{
public:
	// Constructors assume that the texture starts at the origin and the collision mesh has not moved
	TextureComponent() = delete;
	TextureComponent(Entity* owner, const SDL_FRect& camera, Texture&& texture);
	TextureComponent(Entity* owner, const SDL_FRect& camera, SDL_Renderer& defaultRenderer, std::string pathOrText, TTF_Font* defaultFont = nullptr, SDL_Color* textColor = nullptr);
	virtual ~TextureComponent() = default;
	
	void draw() override;

private:

	Texture mTexture;

	// Location of the upper-left corner of the texture relative to entity position
	Vec mOffsetFromPos;

	// The camera this component will render relative to (not owned)
	const SDL_FRect* mCamera;
};

template<typename EntityType>
class SharedTextureComponent : public Component
{
public:
	// Constructors assume that the texture starts at the origin and the collision mesh has not moved
	SharedTextureComponent() = delete;

	SharedTextureComponent(Entity* owner, const SDL_FRect& camera, Texture&& texture) : Component{ owner }, 
		mOffsetFromPos{ -1 * GET_MESH(mOwner).getPos() }, mCamera{ &camera }
	{
		mTexture.reset(new Texture{ std::move(texture) });
	}

	SharedTextureComponent(Entity* owner, const SDL_FRect& camera, SDL_Renderer& defaultRenderer, std::string pathOrText, TTF_Font* defaultFont = nullptr, SDL_Color* textColor = nullptr) 
		: Component{ owner }, mOffsetFromPos{ -1 * GET_MESH(mOwner).getPos() }, mCamera{ &camera }
	{
		mTexture.reset(new Texture{ &defaultRenderer, pathOrText, defaultFont, textColor });
	}

	// Only usable for non-first objects
	SharedTextureComponent(Entity* owner, const SDL_FRect& camera) : Component{ owner }, 
	mOffsetFromPos{ -1 * GET_MESH(mOwner).getPos() }, mCamera{ &camera }
	{
		// Throw error if this is the first object of this class type
		if(mObjectCount == 0)
			throw(std::runtime_error{ "Error: Attempted to create a shared texture component without a texture\n" });
	}

	virtual ~SharedTextureComponent() 
	{
		if (mObjectCount == 1)	// If this is the last object
		{
			// Remove all static pointers
			mTexture.reset(nullptr);
		}
		
		// Remove this object from the count
		--mObjectCount;
	}

	void draw() override
	{
		// Get collision mesh from mechanical component
		auto& collisionMesh = GET_MESH(mOwner);
		auto& entityPos = collisionMesh.getPos();

		// Render the entity relative to the camera
		float drawPosX = entityPos.getX() + mOffsetFromPos.getX() - mCamera->x;
		float drawPosY = entityPos.getY() + mOffsetFromPos.getY() - mCamera->y;

		// Center of rotation (relative to texture position)
		SDL_FPoint center{ -mOffsetFromPos.getX(),  -mOffsetFromPos.getY() };

		// Draw
		mTexture->draw(drawPosX, drawPosY, nullptr, static_cast<double>(collisionMesh.getRotAngle()), &center);
	}

private:
	// Texture shared between objects
	static inline std::unique_ptr<Texture> mTexture{ nullptr };

	// Location of the upper-left corner of the texture relative to entity position
	Vec mOffsetFromPos;

	// The camera this component will render relative to (not owned)
	const SDL_FRect* mCamera;

	// Keep track of how many objects exist
	static inline int mObjectCount = 0;
};
