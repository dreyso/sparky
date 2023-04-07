#pragma once
#include "component.h"
#include "../entity.h"
#include "../vec.h"

#include <SDL.h>


class KeyPressAccelComponent : public Component
{
public:
	KeyPressAccelComponent() = delete;
	KeyPressAccelComponent(Entity* owner, SDL_Event* event, float accelForce, float maxVel, float dragConst);
	virtual ~KeyPressAccelComponent() = default;

	// Check the key-presses and set acceleration accordingly
	void handleEvent() override;
	// Update acceleration and mechanical component
	void update1(float deltaTime) override;

	Vec calcDrag(const Vec& accel, const Vec& vel, float deltaTime);

private:
	SDL_Event* mEvent;            // non-owning pointer
	float mAccelConst;            // The magnitude of the entity's acceleration
	float mDiagonalAccelConst;    // The magnitude of the entity's diagonal acceleration

	float mMaxVel;                // Enforced by the force of drag
	float mDragConst;             // Arbitrary drag value

	Vec mAccel{ 0.f, 0.f };
	Vec mAccelDirectionVec{ 0.f,0.f };
};