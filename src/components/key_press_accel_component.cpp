#include "../../header/components/key_press_accel_component.h"
#include "../../header/components/mechanical_component.h"
#include "../../header/components/component.h"
#include "../../header/util.h"

#include <cmath>
#include <SDL.h>


KeyPressAccelComponent::KeyPressAccelComponent(Entity* owner, SDL_Event* event, float accelForce, float maxVel, float dragConst)
	: Component{ owner }, mEvent{ event }, mAccelConst{ accelForce }, mMaxVel{ maxVel }, mDragConst{ dragConst }
{
    // Find diagonal acceleration constant that sets diagonal acceleration to acceleration in a single direction
    Vec diagonalAccel{ mAccelConst, mAccelConst };
    diagonalAccel.normalize();
    diagonalAccel *= mAccelConst;
    mDiagonalAccelConst = diagonalAccel.getX();
}

// Check the key-presses and set acceleration accordingly
void KeyPressAccelComponent::handleEvent()
{
    // If a key was pressed
    if (mEvent->type == SDL_KEYDOWN && mEvent->key.repeat == 0)
    {
        Vec x{1.f, 0.f};
        Vec y{ 0.f, 1.f };

        // Adjust the acceleration
        switch (mEvent->key.keysym.sym)
        {
        case SDLK_w: mAccelDirectionVec -= y; break;
        case SDLK_s: mAccelDirectionVec += y; break;
        case SDLK_a: mAccelDirectionVec -= x; break;
        case SDLK_d: mAccelDirectionVec += x; break;
        }
    }
    // If a key was released
    else if (mEvent->type == SDL_KEYUP && mEvent->key.repeat == 0)
    {
        Vec x{ 1.f, 0.f };
        Vec y{ 0.f, 1.f };

        // Adjust the acceleration
        switch (mEvent->key.keysym.sym)
        {
        case SDLK_w: mAccelDirectionVec += y; break;
        case SDLK_s: mAccelDirectionVec -= y; break;
        case SDLK_a: mAccelDirectionVec += x; break;
        case SDLK_d: mAccelDirectionVec -= x; break;
        }
    }
}

Vec KeyPressAccelComponent::calcDrag(const Vec& accel, const Vec& vel, float deltaTime)
{
    if (vel.isZeroVector())
        return Vec{ 0.f, 0.f };

    // Calculate drag magnitude using a custom piecewise function
    float velMag = vel.getMagnitude();
    float dragMagnitude = 0.f;

    if (velMag > mMaxVel)
        dragMagnitude = mAccelConst + mDragConst;
    else if (velMag == mMaxVel)
        dragMagnitude = mAccelConst;
    else
        dragMagnitude = mDragConst;
 
    // Avoid inverting velocity
    if (accel.isZeroVector())
        dragMagnitude = (dragMagnitude * deltaTime) > velMag ? (velMag / deltaTime) : dragMagnitude;

    // Find the drag vector, opposite direction to the velocity
    Vec drag{ vel * -1.f };
    drag.normalize();
    drag *= dragMagnitude;

    return drag;
}

// Update acceleration and mechanical component
void KeyPressAccelComponent::update1(float deltaTime)
{
    // Determine acceleration
    if (mAccelDirectionVec.getX() != 0.f && mAccelDirectionVec.getY() != 0.f)
        mAccel = mAccelDirectionVec * mDiagonalAccelConst;
    else
        mAccel = mAccelDirectionVec * mAccelConst;

    // Get reference to entity's velocity vector
    const Vec& vel = mOwner->getComponent<MechanicalComponent>().getVel();
    float velMag = vel.getMagnitude();

    mAccel += calcDrag(mAccel, vel, deltaTime);

    // Update mechanics
    mOwner->getComponent<MechanicalComponent>().update(deltaTime, mAccel);
}

