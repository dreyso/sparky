#include "../header/collision.h"
#include "../header/vec.h"

#include <SDL.h>

#include <cmath> 
#include <stdio.h>


void convert(const SDL_FRect& a, SDL_Rect& b)
{
    b.x = (int)roundf(a.x);
    b.y = (int)roundf(a.y);
    b.w = (int)roundf(a.w);
    b.h = (int)roundf(a.h);
}

void convert(const SDL_Rect& a, SDL_FRect& b)
{
    b.x = (float)a.x;
    b.y = (float)a.y;
    b.w = (float)a.w;
    b.h = (float)a.h;
}

bool checkCollision(const SDL_Rect& a, const SDL_Rect& b)
{
    return SDL_HasIntersection(&a,&b);
}

bool checkCollision(const SDL_FRect& a, const SDL_Rect& b)
{
    SDL_FRect floatB;
    floatB.x = static_cast<float>(b.x);
    floatB.y = static_cast<float>(b.y);
    floatB.w = static_cast<float>(b.w);
    floatB.h = static_cast<float>(b.h);

    return checkCollision(a, floatB);
}

bool checkCollision(const SDL_FRect& a, const SDL_FRect& b)
{
    // The sides of the rectangles
    float leftA, leftB;
    float rightA, rightB;
    float topA, topB;
    float bottomA, bottomB;

    // Calculate the sides of rect A
    leftA = a.x;
    rightA = a.x + a.w;
    topA = a.y;
    bottomA = a.y + a.h;

    // Calculate the sides of rect B
    leftB = b.x;
    rightB = b.x + b.w;
    topB = b.y;
    bottomB = b.y + b.h;

    // If any of the sides from A are outside of B
    if (bottomA <= topB)
        return false;

    if (topA >= bottomB)
        return false;

    if (rightA <= leftB)
        return false;

    if (leftA >= rightB)
        return false;

    // If none of the sides from A are outside B
    return true;
}

float findIntersectionX(const SDL_FRect& obj1, const SDL_Rect& obj2)
{
    // The sides of the rectangles
    float leftA, leftB;
    float rightA, rightB;

    const SDL_FRect* a = &obj1;

    // Convert obj2 to floating point values
    SDL_FRect floatObj2;
    convert(obj2, floatObj2);
    const SDL_FRect* b = &floatObj2;


    if (obj1.w > floatObj2.w)
    {
        a = &floatObj2;
        b = &obj1;
    }

    leftA = a->x;
    rightA = a->x + a->w;
    leftB = b->x;
    rightB = b->x + b->w;

    // Handle diagnol unpredicatbility
    float mCorrectionX;
    if (leftA < leftB)
        mCorrectionX = leftB - rightA;
    else if (rightA > rightB)
        mCorrectionX = rightB - leftA;
    else
        mCorrectionX = a->w;
    
    return mCorrectionX;
}

float findIntersectionY(const SDL_FRect& obj1, const SDL_Rect& obj2)
{
    // The sides of the rectangles
    float topA, topB;
    float bottomA, bottomB;

    const SDL_FRect* a = &obj1;

    // Convert obj2 to floating point values
    SDL_FRect floatObj2;
    convert(obj2, floatObj2);
    const SDL_FRect* b = &floatObj2;

    if (obj1.h > obj2.h)
    {
        a = &floatObj2;
        b = &obj1;
    }

    topA = a->y;
    bottomA = a->y + a->h;
    topB = b->y;
    bottomB = b->y + b->h;

    float mCorrectionY;

    if (topA < topB)
        mCorrectionY = topB - bottomA;
    else if (bottomA > bottomB)
        mCorrectionY = bottomB - topA;
    else
        mCorrectionY = a->h;
    return mCorrectionY;
}


 void buildCollisionReport(const SDL_FRect& entity, const std::vector<SDL_Rect>& collidedTiles, Vec& adjustPos)
{
    for (int iTile = 0; iTile < collidedTiles.size(); ++iTile)
    {
        Vec correction;

        // Find intersections on both axi
        float mCorrectionX = findIntersectionX(entity, collidedTiles[iTile]);
        float mCorrectionY = findIntersectionY(entity, collidedTiles[iTile]);

        // Identify the shallow axis
        if (abs(mCorrectionX) < abs(mCorrectionY))
            correction.setX(mCorrectionX);
        else
            correction.setY(mCorrectionY);
        
        // If overall adjustment is 0, set it to the current correction
        if (adjustPos.getMagnitude() == 0.f)
            adjustPos = correction;

        // Otherwise, add only the extra direction to the overall adjustment
        else
        {
            float duplicate = correction.scalarProjectOn(adjustPos);
            duplicate /= adjustPos.getMagnitude();
            duplicate = std::min(duplicate, 1.f);
            adjustPos += correction - (duplicate * adjustPos);
        }
    }
}
