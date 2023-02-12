#pragma once
#include "component.h"
#include "../entity.h"
#include "../collision_map.h"
#include "../vec.h"

#include <SDL.h>


class MapCollisionComponent : public Component
{
public:
	MapCollisionComponent() = delete;
	MapCollisionComponent(Entity* owner, CollisionMap& map) : Component{ owner }, mMap{ &map }{}
	virtual ~MapCollisionComponent() = default;

	// Detect collison with map and update mechanical component
	void update2(float deltaTime) override;

private:
	CollisionMap* mMap;	// Non-owning pointer
};