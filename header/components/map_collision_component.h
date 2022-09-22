#pragma once
#include "component.h"
#include "../entity.h"
#include "../map.h"
#include "../vec.h"

#include <SDL.h>


class MapCollisionComponent : public Component
{
public:
	MapCollisionComponent() = delete;
	MapCollisionComponent(Entity* owner, Map* map) : Component{ owner }, mMap{ map }{}
	virtual ~MapCollisionComponent() = default;

	// Detect collison with map and update mechanical component
	void update2(float deltaTime) override;

private:
	Map* mMap;	// Non-owning pointer
};