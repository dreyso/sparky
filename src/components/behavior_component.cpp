#include "../../header/components/behavior_component.h"
#include "../../header/components/component.h"
#include "../../header/components/mechanical_component.h"
#include "../../header/pathfinder.h"
#include "../../header/entity.h"
#include "../../header/vec.h"


BehaviorComponent::BehaviorComponent(Entity* owner, Pathfinder* pathfinder, const Entity* target, float accelForce, float maxVel, float dragCap)
    : Component{ owner }, mTarget{ target }, mAccelForce{ accelForce }, mMaxVel{ maxVel }, mDragCap{ dragCap }
{
    ++mObjCount;
     mPathfinder = pathfinder;

     // Calculate needed exponent of max velocity to cancel accerelation force
     mDragExponent = log(mAccelForce) / log(mMaxVel);   // Exponent = log[base: max vel](accel force)
}

BehaviorComponent::BehaviorComponent(Entity* owner, Pathfinder* pathfinder, const Entity* target, float accelForce, float maxVel, float dragCap, const Vec& accel)
    : Component{ owner }, mTarget{ target }, mAccelForce{ accelForce }, mMaxVel{ maxVel }, mDragCap{ dragCap }, mAccel{ accel }
{
    ++mObjCount;
    mPathfinder = pathfinder;

    // Calculate needed exponent of max velocity to cancel accerelation force
    mDragExponent = log(mAccelForce) / log(mMaxVel);   // Exponent = log[base: max vel](accel force)
}

BehaviorComponent::~BehaviorComponent()
{
    // Remove static reference if this is the last object being destroyed
    if (mObjCount == 1)
    {
        mTarget = nullptr;
        mPathfinder = nullptr;
    }
    --mObjCount;
}

void BehaviorComponent::update1(float deltaTime)
{
    // Get reference to mechanical component
    MechanicalComponent& mechComp = mOwner->getComponent<MechanicalComponent>();

    // Update the path, if it isn't empty, then advance the point 
    if (!updatePath(deltaTime) || !advancePoint())
    {
        mechComp.resetVel();
        return;
    }
    
    // Get reference to collision box
    const SDL_FRect& colBox = mechComp.getCollisionBox();

    // Calculate the angle needed to accelerate towards the next point on the path
    float deltaX, deltaY;
    deltaX = static_cast<float>(mPath.top().x) - (colBox.x + colBox.w / 2.f);
    deltaY = static_cast<float>(mPath.top().y) - (colBox.y + colBox.h / 2.f);
    float accelAngle = atan2f(deltaY, deltaX);

    // Set acceleration towards the point
    mAccel.setX(mAccelForce * cosf(accelAngle));
    mAccel.setY(mAccelForce * sinf(accelAngle));
    
    // Calculate the drag force
    if (mechComp.getVel().getX() != 0 || mechComp.getVel().getY() != 0)
    {
        // Find drag
        float dragMagnitude = powf(mechComp.getVel().getMagnitude(), mDragExponent);

        // Cap the drag (cap must be greater than the acceleration to avoid being completely canceled)
        dragMagnitude = std::min(dragMagnitude, mDragCap);

        // Find the drag vector, opposite direction to the velocity
        Vec drag{ mechComp.getVel() * -1.f };
        drag.normalize();
        drag *= dragMagnitude;

        // Apply drag to acceleration
        mAccel += drag;
    }
    
    // Update the mechanics of the entity
    mechComp.update(deltaTime, mAccel);
}

bool BehaviorComponent::updatePath(float deltaTime)
{
    // Get reference to mechanical component
    MechanicalComponent & mechComp = mOwner->getComponent<MechanicalComponent>();

    // Add passed time to total time
    mElapsedTime += deltaTime;

    // Recalculate path if sufficient time has passed and entity isn't inside a wall
    if (mElapsedTime >= 0.25f && !mPathfinder->isInWall(mechComp.getPos()))
    {
        // Clear path
        while (!mPath.empty())
            mPath.pop();

        // Get new path
        mPathfinder->findPath(mechComp.getPos(), mTarget->getComponent<MechanicalComponent>().getPos(), mPath);
        mElapsedTime = 0.f;
    }

    // Return false if path is empty
    return !mPath.empty();
}

bool BehaviorComponent::advancePoint()
{
    // Get reference to mechanical component
    MechanicalComponent& mechComp = mOwner->getComponent<MechanicalComponent>();

    // Check if the point is reached
    if (abs(static_cast<float>(mPath.top().x) - mechComp.getPos().getX()) < 30.f && abs(static_cast<float>(mPath.top().y) - mechComp.getPos().getY()) < 30.f)
    {
        // Move on to next point
        mPath.pop();
    }
    
    // Return false if path is empty
    return !mPath.empty();
}
