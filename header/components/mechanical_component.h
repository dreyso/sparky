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
	MechanicalComponent(Entity* owner, const ConvexPolygon& collisionMesh) : Component{ owner }, mCollisionMesh{ collisionMesh }{}
	MechanicalComponent(Entity* owner, ConvexPolygon&& collisionMesh) : Component{ owner }, mCollisionMesh{ std::move(collisionMesh) }{}
	
	MechanicalComponent(Entity* owner,  const ConvexPolygon& collisionMesh, const Vec& vel)
		: Component{ owner }, mCollisionMesh{ collisionMesh }, mVel{ vel }{}
	MechanicalComponent(Entity* owner, ConvexPolygon&& collisionMesh, const Vec& vel)
		: Component{ owner }, mCollisionMesh{ std::move(collisionMesh) }, mVel{ vel }{}

	virtual ~MechanicalComponent() {}

	void update(float deltaTime, const Vec& accel);

	void resetVel();            // Sets velocity to 0

	void addToPos(const Vec& toAdd);
	const Vec& getPos() const;
	const Vec& getVel() const;
	const ConvexPolygon& getCollisionMesh();
	double getRotationAngle() const;

protected:
	ConvexPolygon mCollisionMesh;
	Vec mVel{ 0.f, 0.f };
};
