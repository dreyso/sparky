#pragma once
#include "component.h"
#include "../entity.h"
#include "../vec.h"

#include <SDL.h>


class CameraComponent : public Component
{
public:
	CameraComponent() = delete;
	CameraComponent(Entity* owner, SDL_Window* window) : Component{ owner }, mWindow{ window }{}
	virtual ~CameraComponent() = default;

	// Center the camera over the player using linear interpolation
	void update3(float deltaTime) override;

	SDL_FRect& getCamera();

private:
	SDL_Window* mWindow;    // Non-owning pointer
	SDL_FRect mCamera{ 0.f,0.f,0.f,0.f };
};