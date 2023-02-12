#include "../../header/components/mechanical_component.h"
#include "../../header/vec.h"
#include "../../header/polygon.h"

#include <cmath>


void MechanicalComponent::update(float deltaTime, const Vec& accel)
{
    // Update velocity
    mVel += accel * deltaTime;

    // If magnitude of velocity falls below 30 (pixels per second), truncate to 0
    if (mVel.getMagnitude() < 30.f)
        resetVel();
    else
    {
        // Update rotation only if velocity hasn't been truncated
        mCollisionBox.rotateTo(atan2(mVel.getY(), mVel.getX()) * (180.f / static_cast<float>(M_PI)));
    }

    // Update position
    mCollisionBox.moveBy(mVel * deltaTime);
}

void MechanicalComponent::resetVel()  
{ 
    mVel.setX(0.f);
    mVel.setY(0.f);
    return; 
}

void MechanicalComponent::addToPos(const Vec& toAdd)
{
    mCollisionBox.moveBy(toAdd);
}

const Vec& MechanicalComponent::getPos() const { return mCollisionBox.getPos(); }

const Vec& MechanicalComponent::getVel() const { return mVel; }

const ConvexPolygon& MechanicalComponent::getCollisionBox() {return mCollisionBox; }

double MechanicalComponent::getRotationAngle() const { return mCollisionBox.getRotAngle(); }

