#pragma once
#include "Component.h"
#include "../vec.h"
#include "../polygon.h"

#include <SDL.h>

#include <stdexcept>


class MechanicalComponent : public Component
{
public:
	MechanicalComponent() = delete;
	MechanicalComponent(Entity* owner, const ConvexPolygon& collisionBox) : Component{ owner }, mCollisionBox{ collisionBox }{}
	MechanicalComponent(Entity* owner, ConvexPolygon&& collisionBox) : Component{ owner }, mCollisionBox{ std::move(collisionBox) }{}
	
	MechanicalComponent(Entity* owner,  const ConvexPolygon& collisionBox, const Vec& vel)
		: Component{ owner }, mCollisionBox{ collisionBox }, mVel{ vel }{}
	MechanicalComponent(Entity* owner, ConvexPolygon&& collisionBox, const Vec& vel)
		: Component{ owner }, mCollisionBox{ std::move(collisionBox) }, mVel{ vel }{}

	virtual ~MechanicalComponent() {}

	void update(float deltaTime, const Vec& accel);

	void resetVel();            // Sets velocity to 0

	void addToPos(const Vec& toAdd);
	const Vec& getPos() const;
	const Vec& getVel() const;
	const ConvexPolygon& getCollisionBox();
	double getRotationAngle() const;

protected:
	ConvexPolygon mCollisionBox;
	Vec mVel{ 0.f, 0.f };
};
