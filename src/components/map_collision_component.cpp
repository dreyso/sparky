#include "../../header/components/map_collision_component.h"
#include "../../header/components/mechanical_component.h"
#include "../../header/components/component.h"
#include "../../header/map.h"

#include "../../header/vec.h"
#include "../../header/polygon.h"

#include <SDL.h>


// Detect collison with map and update mechanical component
void MapCollisionComponent::update2(float deltaTime)
{
    MechanicalComponent& mechComp = mOwner->getComponent<MechanicalComponent>();
    // Get reference to entity's collision box
    const SDL_FRect& collisionBox = mechComp.getCollisionBox();
    
    Vec adjustPos{ 0.f,0.f };
    if (mMap->checkWallCollisions(collisionBox, adjustPos))
        mechComp.addToPos(adjustPos);
}
