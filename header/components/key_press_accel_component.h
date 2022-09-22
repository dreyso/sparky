#pragma once
#include "component.h"
#include "../entity.h"
#include "../vec.h"

#include <SDL.h>


class KeyPressAccelComponent : public Component
{
public:
	KeyPressAccelComponent() = delete;
	KeyPressAccelComponent(Entity* owner, SDL_Event* event, float accelForce, float maxVel, float dragCap);
	virtual ~KeyPressAccelComponent() = default;

	// Check the keypresses and set acceleration accordingly
	void handleEvent() override;
	// Update acceleration and mechanical component
	void update1(float deltaTime) override;

private:
	SDL_Event* mEvent;            // non-owning pointer
	float mAccelForce;            // The force the entity applies to accelerate
	float mDiagonalAccelForce;    // The force vector normailzied

	float mMaxVel;                // Enforced by the force of drag
	float mDragExponent;          // exponent = log[base: max vel](accel force)
	float mDragCap;               // Arbitrary maxiumim possible value for drag force

	Vec mAccel{ 0.f, 0.f };
	Vec mAccelDirectionVec{ 0.f,0.f };
};