#include "../../header/components/camera_component.h"
#include "../../header/components/mechanical_component.h"
#include "../../header/components/component.h"
#include "../../header/util.h"

#include <SDL.h>

#define TO_FLOAT(intValue) (static_cast<float>(intValue))

// Center the camera over the player using linear interpolation
void CameraComponent::update3(float deltaTime)
{
    // Get dimensions of the SDL window
    int windowWidth = 0, windowHeight = 0;
    SDL_GetWindowSize(mWindow, &windowWidth, &windowHeight);

    // Set camera dimensions
    mCamera.w = TO_FLOAT(windowWidth);
    mCamera.h = TO_FLOAT(windowHeight);

    // Get entity's position
    auto& pos = mOwner->getComponent<MechanicalComponent>().getCollisionMesh().getPos();

    // Find the centered, target-destination values for the camera
    float destX = (pos.getX() - TO_FLOAT(windowWidth) / 2.f);
    float destY = (pos.getY() - TO_FLOAT(windowHeight) / 2.f);

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