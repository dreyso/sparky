#include "../../header/components/mechanical_component.h"
#include "../../header/vec.h"
#include "../../header/polygon.h"

#include <cmath>


void MechanicalComponent::update(float deltaTime, const Vec& accel)
{
    // Update velocity
    mVel += accel * deltaTime;

    // If magnitude of velocity falls below 30 (pixels per second), truncate to 0
    if (mVel.getMagnitude() < 15.f)
        resetVel();
    else
    {
        // Update rotation only if velocity hasn't been truncated
        mCollisionMesh.rotateTo(atan2(mVel.getY(), mVel.getX()) * (180.f / static_cast<float>(M_PI)));
    }

    // Update position
    mCollisionMesh.moveBy(mVel * deltaTime);
}

void MechanicalComponent::resetVel()  
{ 
    mVel.setX(0.f);
    mVel.setY(0.f);
    return; 
}

void MechanicalComponent::addToPos(const Vec& toAdd)
{
    mCollisionMesh.moveBy(toAdd);
}

const Vec& MechanicalComponent::getPos() const { return mCollisionMesh.getPos(); }

const Vec& MechanicalComponent::getVel() const { return mVel; }

const ConvexPolygon& MechanicalComponent::getCollisionMesh() {return mCollisionMesh; }

double MechanicalComponent::getRotationAngle() const { return mCollisionMesh.getRotAngle(); }

