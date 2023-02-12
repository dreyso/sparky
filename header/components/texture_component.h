#pragma once
#include "Component.h"
#include "mechanical_component.h"
#include "../texture.h"
#include "../entity.h"
#include "../vec.h"
#include "../util.h"

#include <SDL.h>

class TextureComponent : public Component
{
public:
	TextureComponent() = delete;
	TextureComponent(Entity* owner, const SDL_FRect& camera, Texture&& texture);
	TextureComponent(Entity* owner, const SDL_FRect& camera, SDL_Renderer& defaultRenderer, std::string pathOrText, TTF_Font* defaultFont = nullptr, SDL_Color* textColor = nullptr);
	virtual ~TextureComponent() = default;
	
	void draw() override;

private:
	void initOffset();

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
	SharedTextureComponent() = delete;

	SharedTextureComponent(Entity* owner, const SDL_FRect& camera, Texture&& texture) : Component{ owner }, mCamera{ &camera }
	{
		mTexture.reset(new Texture{ std::move(texture) });
		initOffset();
	}

	SharedTextureComponent(Entity* owner, const SDL_FRect& camera, SDL_Renderer& defaultRenderer, std::string pathOrText, TTF_Font* defaultFont = nullptr, SDL_Color* textColor = nullptr) : Component{ owner }, mCamera{ &camera }
	{
		mTexture.reset(new Texture{ &defaultRenderer, pathOrText, defaultFont, textColor });
		initOffset();
	}

	// Only useable for non-first objects
	SharedTextureComponent(Entity* owner, const SDL_FRect& camera) : Component{ owner }, mCamera{ &camera }
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

	void initOffset()
	{
		// Get collision box from mechanical component
		auto& collisionBox = mOwner->getComponent<MechanicalComponent>().getCollisionBox();

		// Get the rectangular hull of the polygon (will act as the dest rect of the texture)
		auto rect = collisionBox.getRectHull();

		// Store the relative location of the upper-left corner of the texture relative to entity position
		mOffsetFromPos = rect.getPos() - collisionBox.getPos();
	}

	void draw() override
	{
		// Get collision box from mechanical component
		auto& collisionBox = mOwner->getComponent<MechanicalComponent>().getCollisionBox();
		auto& entityPos = collisionBox.getPos();
		auto drawPos = entityPos + mOffsetFromPos;

		// Render the entity relative to the camera
		int screenPosX = static_cast<int>(roundf(drawPos.getX() - mCamera->x));
		int screenPosY = static_cast<int>(roundf(drawPos.getY() - mCamera->y));

		// Center of rotation
		SDL_Point center{ static_cast<int>(roundf(entityPos.getX())), static_cast<int>(roundf(entityPos.getY())) };

		// Draw
		mTexture->draw(screenPosX, screenPosY, nullptr, collisionBox.getRotAngle(), &center);
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
