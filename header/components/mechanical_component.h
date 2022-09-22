#pragma once
#include "Component.h"
#include "../vec.h"

#include <SDL.h>

#include <stdexcept>


class MechanicalComponent : public Component
{
public:
	MechanicalComponent() = delete;
	MechanicalComponent(Entity* owner, const SDL_FRect& collisionBox) : Component{ owner }, mCollisionBox{ collisionBox }
	{
		// Set initial position using the provided collision box
		mPos = Vec{ mCollisionBox.x, mCollisionBox.y};
	}
	
	MechanicalComponent(Entity* owner, const Vec& pos, const Vec& vel, const SDL_FRect& collisionBox) 
		: Component{ owner }, mPos{ pos }, mVel{ vel }, mCollisionBox{ collisionBox }
	{
		if (mPos.getX() != mCollisionBox.x || mPos.getY() != mCollisionBox.y)
			throw(std::runtime_error{ "Error: Provided mechanical component with conflicting coordinates\n" });
	}

	virtual ~MechanicalComponent() {}

	void update(float deltaTime, const Vec& accel);

	void resetVel();            // Sets velocity to 0

	void addToPos(const Vec& toAdd);
	const Vec& getPos() const;
	const Vec& getVel() const;
	const SDL_FRect& getCollisionBox();
	double getRotationAngle() const;

protected:
	Vec mPos{ 0.f, 0.f };
	Vec mVel{ 0.f, 0.f };
	SDL_FRect mCollisionBox;    // Kept up-to-date with position

	// Facing direction
	double mRotationAngle = 0.0;
};
