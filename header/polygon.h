#pragma once
#include "vec.h"

#include <vector>


// Polygon must be convex
// Vertices must be in clockwise order
class Polygon
{
public:
	Polygon() = delete;

	// Provide absolute or relative (to origin) vertices
	Polygon(std::vector<Vec>& vertices);
	Polygon(std::vector<Vec>&& vertices);
	~Polygon() = default;

	void moveBy(Vec addToPos);
	void moveTo(Vec pos);
	void rotateBy(float addToAngle);
	void rotateTo(float angle);

	// Return 0 if there is no collision(s)
	static Vec resolveCollision(const Polygon& moveableShape, const Polygon& fixedShape);

	// Returns minimuim vector to resolve all collisions
	static Vec resolveCollisions(const Polygon& moveableShape, const std::vector<Polygon>& fixedShapes);

	const Vec& getPos() const;
	const std::vector<Vec>& getVertices() const;

private:
	bool isConvex();
	void initPos();
	void updateAbsoluteVertices();
	void updateRotation();
	
	// Returns a list of the edges of the polygon as clockwise vectors
	std::vector<Vec> getCollisionAxi() const;    

	Vec mPos;                                   // Centroid
	std::vector<Vec> mRelativeVertices;         // Only change during rotations
	std::vector<Vec> mAbsoluteVertices;         // Updated when requested

	float mRotAngle = 0.f;
};