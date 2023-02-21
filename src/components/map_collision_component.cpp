#include "../../header/components/map_collision_component.h"
#include "../../header/components/mechanical_component.h"
#include "../../header/components/component.h"
#include "../../header/collision_map.h"

#include "../../header/vec.h"
#include "../../header/polygon.h"

#include <SDL.h>


// Detect collison with map and update mechanical component
void MapCollisionComponent::update2(float deltaTime)
{
    MechanicalComponent& mechComp = mOwner->getComponent<MechanicalComponent>();
    // Get reference to entity's collision mesh
    const ConvexPolygon& collisionMesh = mechComp.getCollisionMesh();
    
    mechComp.addToPos(mMap->resolveCollisions(collisionMesh));
}
