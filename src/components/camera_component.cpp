#include "../../header/components/camera_component.h"
#include "../../header/components/mechanical_component.h"
#include "../../header/components/component.h"
#include "../../header/util.h"

#include <SDL.h>


// Center the camera over the player using linear interpolation
void CameraComponent::update3(float deltaTime)
{
    // Get dimensions of the SDL window
    int windowWidth = 0, windowHeight = 0;
    SDL_GetWindowSize(mWindow, &windowWidth, &windowHeight);

    // Set camera dimensions
    mCamera.w = static_cast<int>(roundf(windowWidth));
    mCamera.h = static_cast<int>(roundf(windowHeight));

    // Get reference to entity's collision box
    const SDL_FRect& collisionBox = mOwner->getComponent<MechanicalComponent>().getCollisionBox();

    // Find the centered, target-destination values for the camera
    float destX = ((collisionBox.x + collisionBox.w / 2.f) - static_cast<float>(windowWidth) / 2.f);
    float destY = ((collisionBox.y + collisionBox.h / 2.f) - static_cast<float>(windowHeight) / 2.f);

    // Find how much to add to the camera's position using elapsed time as a percentage
    float addX = (destX - mCamera.x) * (3.f * deltaTime);
    float addY = (destY - mCamera.y) * (3.f * deltaTime);

    // Set a minimim value to increment every frame to avoid infinite approach
    if (addX < 0 && addX > -1.f)
        addX = -1.f;
    else if (addX > 0 && addX < 1.f)
        addX = 1.f;

    if (addY < 0 && addY > -1.f)
        addY = -1.f;
    else if (addY > 0 && addY < 1.f)
        addY = 1.f;

    // Add to the camera's position, but avoid overshooting the the destination
    float tempX = mCamera.x + addX;
    if ((addX < 0.f && tempX < destX) || (addX > 0.f && tempX > destX))
        mCamera.x = destX;
    else
        mCamera.x = tempX;

    float tempY = mCamera.y + addY;
    if ((addY < 0.f && tempY < destY) || (addY > 0.f && tempY > destY))
        mCamera.y = destY;
    else
        mCamera.y = tempY;
}

SDL_FRect& CameraComponent::getCamera() 
{
    return mCamera;
}