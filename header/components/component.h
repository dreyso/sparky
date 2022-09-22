#pragma once
#include <SDL.h>

#include <unordered_map>
#include <memory>


class Entity;

class Component {
public:
	Component() = delete;
	virtual ~Component() {}

	virtual void handleEvent() {}
	virtual void update1(float deltaTime) {}
	virtual void update2(float deltaTime) {}
	virtual void update3(float deltaTime) {}
	virtual void draw() {}

protected:
	// Can only be created from a derived class
	Component(Entity* owner) : mOwner{ owner } {}
	Entity* mOwner;
};


