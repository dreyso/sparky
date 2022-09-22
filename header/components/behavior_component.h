#pragma once
#include "component.h"
#include "../Pathfinder.h"
#include "../entity.h"
#include "../vec.h"

#include <SDL.h>
#include <stack>


class BehaviorComponent : public Component
{
	public:
	BehaviorComponent() = delete;
	BehaviorComponent(Entity* owner, Pathfinder* pathfinder, const Entity* target, float accelForce, float maxVel, float dragCap);
	BehaviorComponent(Entity* owner, Pathfinder* pathfinder, const Entity* target, float accelForce, float maxVel, float dragCap, const Vec& accel);
	virtual ~BehaviorComponent();

	void update1(float deltaTime) override;

	// Return false if path ends up empty
	bool updatePath(float deltaTime);
	bool advancePoint();

private:
	// Non-owning pointers
	static inline Pathfinder* mPathfinder = nullptr;
	const Entity* mTarget;

	float mAccelForce;      // The force the entity applies to accelerate

    // Drag force is calculated each frame by exponentiating current velocty

	float mMaxVel;          // Enforced by drag
	float mDragExponent;    // exponent = log[base: max vel](accel force)
	float mDragCap;         // Arbitrary maxiumim possible value for drag
	
	Vec mAccel{ 0.f, 0.f };
	
	float mElapsedTime = 0.f;

	std::stack<SDL_Point> mPath;

	// Keep track of how many objects exist
	static inline int mObjCount = 0;
};