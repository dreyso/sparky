#pragma once
#include "vec.h"

#include <vector>


// Polygon must be not self intersect, and cannot have collinear edges
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

	const Vec& getPos() const;
	const std::vector<Vec>& getVertices() const;
	std::vector<ConvexPolygon> triangulate() const;

protected:
	// Called by convex polygon ctor to avoid unnecessary checks
	// Integer is for function overloading
	Polygon(std::vector<Vec>& vertices, int);
	Polygon(std::vector<Vec>&& vertices, int);

private:
	// Used by ctor to check if vertices are in clockwise winding order
	bool isClockwise() const;

	// Used by ctor to check for self intersections
	bool isSelfIntersecting() const;

	// Returns a list of the edges of the polygon as clockwise vectors (offsets come first)
	std::vector<std::pair<Vec, Vec>> getEdges() const;

	// Used by ctor to check for collinear edges
	bool hasCollinearEdges() const;
	
	void initPos();
	void updateAbsoluteVertices();
	void updateRotation();

	Vec mPos;                                   // Centroid
	std::vector<Vec> mRelativeVertices;         // Only change during rotations
	std::vector<Vec> mAbsoluteVertices;         // Change during rotations and translations

	float mRotAngle = 0.f;
};

class ConvexPolygon : public Polygon
{
public:
	ConvexPolygon() = delete;

	// Provide absolute or relative (to origin) vertices
	ConvexPolygon(std::vector<Vec>& vertices);
	ConvexPolygon(std::vector<Vec>&& vertices);
	~ConvexPolygon() = default;

	// Return 0 if there is no collision
	static Vec resolveCollision(const ConvexPolygon& moveableShape, const ConvexPolygon& fixedShape);

	// Returns minimuim vector to resolve all collisions
	static Vec resolveCollisions(const ConvexPolygon& moveableShape, const std::vector<ConvexPolygon>& fixedShapes);

private:
	// Used by ctor to check if the polygon is convex, also checks for collinear edges, self intersections, and a clockwise winding order
	bool isConvex();

	// Returns a list of the edges of the polygon as clockwise vectors
	std::vector<Vec> getCollisionAxi() const;
};