#include "../../header/components/mechanical_component.h"
#include "../../header/vec.h"

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
        mRotationAngle = atan2(static_cast<double>(mVel.getY()), static_cast<double>(mVel.getX())) * (180.0 / M_PI);
    }

    // Update position
    mPos += mVel * deltaTime;

    // Update collision box position as well
    mCollisionBox.x = mPos.getX();
    mCollisionBox.y = mPos.getY();
}

void MechanicalComponent::resetVel()  
{ 
    mVel.setX(0.f);
    mVel.setY(0.f);
    return; 
}

void MechanicalComponent::addToPos(const Vec& toAdd)
{
    mPos += toAdd;
    // Update collision box position as well
    mCollisionBox.x = mPos.getX();
    mCollisionBox.y = mPos.getY();
}

const Vec& MechanicalComponent::getPos() const { return mPos; }

const Vec& MechanicalComponent::getVel() const { return mVel; }

const SDL_FRect& MechanicalComponent::getCollisionBox() {return mCollisionBox; }

double MechanicalComponent::getRotationAngle() const { return mRotationAngle; }

