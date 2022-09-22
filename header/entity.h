#pragma once
#include "components/Component.h"
#include "util.h"

#include <stdexcept>


class missing_component : public std::runtime_error
{
public:
	missing_component() : runtime_error("") {}
	missing_component(const char* whatArg) noexcept : runtime_error(whatArg) {}
	virtual ~missing_component() = default;

	const char* what() const noexcept override
	{
		return runtime_error::what();
	}
};

// Class design credit: https:// www.youtube.com/watch?v=XsvI8Sng6dk
class Entity
{
public:
	Entity() {}
	~Entity() {}

	void handleEvents()
	{
		for (auto& [key, value] : mComponents)
		{
			value->handleEvent();
		}
	}

	void update(float deltaTime)
	{
		for (auto& [key, value] : mComponents)
		{
			value->update1(deltaTime);
		}

		for (auto& [key, value] : mComponents)
		{
			value->update2(deltaTime);
		}
		
		for (auto& [key, value] : mComponents)
		{
			value->update3(deltaTime);
		}
	}

	void draw()
	{
		for (auto& [key, value] : mComponents)
		{
			value->draw();
		}
	}

	bool isActive() const { return mAlive; }
	void kill() { mAlive = false; }

	// Check if the entity has the specified component type
	template<typename T> bool hasComponent() const
	{
		auto it = mComponents.find(IdGen<T>::getTypeID());
		return (it == mComponents.end());
	}

	// Fowarding ref
	template<typename T, typename... Targs>
	T& addComponent(Targs&&... args)
	{
		// Create specified component and provide pointer to this entity, and forward any other arguments this type needs
		T* component(new T{ this, std::forward<Targs>(args)... });

		// Create unique base class pointer (upcasting)
		std::unique_ptr<Component> ptr{ component };

		// Move the pointer into the entity's component map
		mComponents.insert({ IdGen<T>::getTypeID(), std::move(ptr) });

		// Return a reference to the newly added component
		return *component;
	}

	// Retrieve specified component
	template<typename T> T& getComponent() const
	{
		// Get specified component from the map using its type id
		auto it = mComponents.find(IdGen<T>::getTypeID());

		// Throw exception if the component doesn't exist
		if (it == mComponents.end())
			throw(missing_component{"Error: Attempted to retrieve a component that does not exist\n"});

		Component* baseComponent = it->second.get();
		// Downcast it to its actual type
		return static_cast<T&>(*baseComponent);
	}

private:
	bool mAlive = true;
	std::unordered_map<typeID, std::unique_ptr<Component>> mComponents;
};