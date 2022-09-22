#pragma once
#include "Component.h"
#include "mechanical_component.h"
#include "../texture.h"
#include "../entity.h"
#include "../util.h"

#include <SDL.h>


class TextureComponent : public Component
{
public:
	TextureComponent() = delete;
	TextureComponent(Entity* owner, const SDL_FRect* camera, Texture&& texture);
	TextureComponent(Entity* owner, const SDL_FRect* camera, SDL_Renderer* defaultRenderer, std::string pathOrText, TTF_Font* defaultFont = nullptr, SDL_Color* textColor = nullptr);
	virtual ~TextureComponent();
	
	void draw() override;

private:
	void initCamera(const SDL_FRect* camera);

	Texture mTexture;

	// The camera this component will render relative to (not owned)
	static inline const SDL_FRect* mCamera = nullptr;

	// Keep track of how many objects exist
	static inline int mObjectCount = 0;
};

template<typename EntityType>
class SharedTextureComponent : public Component
{
public:
	SharedTextureComponent() = delete;

	SharedTextureComponent(Entity* owner, const SDL_FRect* camera, Texture&& texture) : Component{ owner }
	{
		initFirstObject(camera, std::move(texture));
	}

	SharedTextureComponent(Entity* owner, const SDL_FRect* camera, SDL_Renderer* defaultRenderer, std::string pathOrText, TTF_Font* defaultFont = nullptr, SDL_Color* textColor = nullptr) : Component{ owner }
	{
		initFirstObject(camera, Texture{ defaultRenderer, pathOrText, defaultFont, textColor });
	}

	// Only useable for non-first objects
	SharedTextureComponent(Entity* owner, const SDL_FRect* camera) : Component{ owner }
	{
		// Throw error if this is the first object of this class type
		if(mObjectCount == 0)
			throw(std::runtime_error{ "Error: Attempted to create a shared texture component without a texture\n" });

		initCamera(camera);
	}

	virtual ~SharedTextureComponent() 
	{
		if (mObjectCount == 1)	// If this is the last object
		{
			// Remove all static pointers
			mCamera = nullptr;
			mTexture.reset(nullptr);
		}
		
		// Remove this object from the count
		--mObjectCount;
	}

	void draw() override
	{
		// Get collision box from mechanical component
		const SDL_FRect& collisionBox = mOwner->getComponent<MechanicalComponent>().getCollisionBox();

		float textureWidthOffset = (static_cast<float>(mTexture->getWidth()) - collisionBox.w) / 2.f;
		float textureHeightOffset = (static_cast<float>(mTexture->getHeight()) - collisionBox.h) / 2.f;

		// Render the entity relative to the camera
		int screenPosX = static_cast<int>(roundf((collisionBox.x - mCamera->x) - textureWidthOffset));
		int screenPosY = static_cast<int>(roundf((collisionBox.y - mCamera->y) - textureHeightOffset));
		mTexture->draw(screenPosX, screenPosY, nullptr, mOwner->getComponent<MechanicalComponent>().getRotationAngle());
	}

private:
	void initFirstObject(const SDL_FRect* camera, Texture&& texture)
	{
		mTexture.reset(new Texture{ std::move(texture) });
		initCamera(camera);
	}

	void initCamera(const SDL_FRect* camera)
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
				throw(std::runtime_error{ "Error: Attempted to change shared texture component's camera\n" });
		}

		// Count this object
		++mObjectCount;
	}

private:
	// Texture shared between objects
	static inline std::unique_ptr<Texture> mTexture{ nullptr };

	// The camera this component will render relative to (not owned)
	static inline const SDL_FRect* mCamera = nullptr;

	// Keep track of how many objects exist
	static inline int mObjectCount = 0;
};
