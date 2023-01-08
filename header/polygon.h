#pragma once
#include "vec.h"

#include <vector>
#include <fstream>



class ConvexPolygon;

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
	void rotateBy(float degrees);
	void rotateTo(float degrees);
	void offsetVerticesBy(float distance);

	const Vec& getPos() const;
	const std::vector<Vec>& getVertices() const;
	std::vector<ConvexPolygon> triangulate() const;

	// Used by ctor to check if vertices are in clockwise winding order
	static bool isClockwise(const std::vector<Vec>& vertices);

	// Used by ctor to check for self intersections
	static bool isSelfIntersecting(const std::vector<Vec>& vertices);

	// Returns a list of the edges of the polygon as clockwise vectors (offsets come first)
	static std::vector<std::pair<Vec, Vec>> getEdges(const std::vector<Vec>& vertices);

	// Used by ctor to check for collinear edges
	static std::vector<Vec>::size_type findCollinearEdge(const std::vector<Vec>& vertices);

	/**
	* Description:
	* Loads all polygons from an SVG file, polygons being stored as (points="...").
	*
	* Pre-conditions:
	* The polygons must have at least 3 vertices and have no collinear or self intersecting edges. If a polygons doesn't
	* satisfy a requirement, the polyon constructor will throw an std::exception.
	* Note that quadrilaterals with a collinear edge (5 vertices) will have the culprit removed and
	* be allowed through as the same shape, minus a vertex.
	*
	* Input:
	* A path to an SVG file.
	*
	* Output:
	* A vector of polygons.
	*/
	static std::vector<Polygon> read_SVG_polygons(const std::ifstream& svgFile);
protected:
	// Called by convex polygon ctor to avoid unnecessary checks
	// Integer is for function overloading
	Polygon(std::vector<Vec>& vertices, int);
	Polygon(std::vector<Vec>&& vertices, int);

private:
	
	void initPos();
	void rotateVerticesBy(float degrees);
	void updateAbsoluteVertices();

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

