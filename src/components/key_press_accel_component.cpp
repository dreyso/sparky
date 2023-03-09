#include "../../header/components/key_press_accel_component.h"
#include "../../header/components/mechanical_component.h"
#include "../../header/components/component.h"
#include "../../header/util.h"

#include <cmath>
#include <SDL.h>


KeyPressAccelComponent::KeyPressAccelComponent(Entity* owner, SDL_Event* event, float accelForce, float maxVel, float dragCap)
	: Component{ owner }, mEvent{ event }, mAccelForce{ accelForce }, mMaxVel{ maxVel }, mDragCap{ dragCap }
{
	// Get diagonal acceleration
	float diagonalAccelForce = sqrtf(static_cast<float>(pow(mAccelForce, 2) + pow(mAccelForce, 2)));
	
    // Normalize it
	mDiagonalAccelForce = (mAccelForce / diagonalAccelForce) * mAccelForce;

    // Calculate the drag exponent (drag exp = log[base: max vel](accel force))
    mDragExponent = logf(mAccelForce)/logf(mMaxVel);
}

// Check the keypresses and set acceleration accordingly
void KeyPressAccelComponent::handleEvent()
{
    // If a key was pressed
    if (mEvent->type == SDL_KEYDOWN && mEvent->key.repeat == 0)
    {
        Vec x{1.f, 0.f};
        Vec y{ 0.f, 1.f };

        // Adjust the accelration
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

// Update acceleration and mechanical component
void KeyPressAccelComponent::update1(float deltaTime)
{
    // Determine accleration
    if (mAccelDirectionVec.getX() != 0.f && mAccelDirectionVec.getY() != 0.f)
        mAccel = mAccelDirectionVec * mDiagonalAccelForce;
    else
        mAccel = mAccelDirectionVec * mAccelForce;

    // Get reference to entity's velocity vector
    const Vec& currentVel = mOwner->getComponent<MechanicalComponent>().getVel();

    // Calculate drag, which is a power of the velocity
    if (currentVel.getX() != 0.f || currentVel.getY() != 0.f)
    {
        // Find drag
        float dragMagnitude = powf(currentVel.getMagnitude(), mDragExponent);
        // Cap the drag (cap must be greater than the acceleration to avoid being completely canceled)
        dragMagnitude = std::min(dragMagnitude, 7000.f);
        // Find the drag vector, opposite direction to the velocity
        Vec drag{ currentVel * -1.f };
        drag.normalize();
        drag *= dragMagnitude;

        // Apply drag to acceleration
        mAccel += drag;
    }

    // Update mechanics
    mOwner->getComponent<MechanicalComponent>().update(deltaTime, mAccel);
}

