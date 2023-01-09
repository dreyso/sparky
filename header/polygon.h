#pragma once
#include "vec.h"

#include <vector>
#include <memory>


class Rect
{
public:

	Rect() = default;
	Rect(float x, float y, float width, float height) : x{ x }, y{ y }, width{ width }, height{ height }{}
	~Rect() = default;

	void set(float x, float y, float width, float height);
	static bool isIntersecting(const Rect& a, const Rect& b);

	float x = 0.f;
	float y = 0.f;
	float width = 0.f;
	float height = 0.f;
};

class ConvexPolygon;

class Polygon
{
public:

	Polygon(std::vector<Vec> vertices);

	void swap(Polygon& other) noexcept;

	Polygon(const Polygon& other);
	Polygon(Polygon&& other) noexcept;

	Polygon& operator=(std::vector<Vec> vertices);
	Polygon& operator=(Polygon other);

	~Polygon() = default;

	void moveBy(const Vec& addToPos);
	void moveTo(const Vec& pos);

	/**
	* Description:
	* Rotates the polygon about its centroid by/to the specified amount (degrees).
	*
	* Input:
	* A float representing the degrees to rotate by/to. The number must be in the
	* interval [-360, 360], otherwise the methods will throw an std::invalid_argument
	* exception.
	*
	* Output:
	* The polygon will rotate accordingly.
	*/
	void rotateBy(float degrees);
	void rotateTo(float degrees);
	
	/**
	* Description:
	* Moves every vertice by the specified distance. The offset spans the line that
	* bisects the angle the vertex forms. As a result, the position of the polygon
	* may change and some vertices may be deleted.
	*
	* Post-conditions:
	* After the transformation, the polygon will be checked if it still satisfies
	* the requirements of the class, otherwise the method will throw an std::invalid_argument
	* exception.
	* 
	* Input:
	* A float representing the offset distance.
	*
	* Output:
	* The transformed polygon.
	*/
	void offsetVerticesBy(float distance);

	const Vec& getPos() const;
	
	const Rect& getAABB() const;

	const std::vector<Vec>& getVertices() const;

	/**
	* Description:
	* Breaks the polygon up into triangles via the ear clipping method.
	*
	* Input:
	* This polygon class.
	*
	* Output:
	* A vector of triangles (as convex polygons).
	*/
	std::vector<ConvexPolygon> triangulate() const;

	/**
	* Description:
	* Determines the the winding order of the polygon's vertices
	* based on the sign of the net area under the polygon's edges.
	* Note that the polygon can be a line or even a single vertex.
	*
	* Input:
	* A vector of vertices.
	*
	* Output:
	* A boolean, true meaning the polygon is clockwise, and
	* false meaning either a counterclockwise winding order or
	* a self-intersecting polygon.
	*/
	static bool isClockwise(const std::vector<Vec>& vertices);

	/**
	* Description:
	* Uses vector math to check if any of the polygon's edges
	* intersect each other.
	*
	* Input:
	* A vector of vertices.
	*
	* Output:
	* A boolean, true meaning the polygon self-intersects, and
	* false meaning the converse.
	*/
	static bool isSelfIntersecting(const std::vector<Vec>& vertices);

	/**
	* Description:
	* Converts a set of vertices into edges, where an edge a pair
	* of vectors. The first vector is the offset, and the second 
	* is the edge itself.
	*
	* Input:
	* A vector of vertices.
	*
	* Output:
	* A vector of pairs.
	*/
	static std::vector<std::pair<Vec, Vec>> getEdges(const std::vector<Vec>& vertices);

	/**
	* Description:
	* Seachs through the vertices untill the first
	* collinear vertex.
	*
	* Input:
	* A vector of vertices.
	*
	* Output:
	* A bool, true means the vertices contain a 
	* collinear vertex, and false meaning the converse
	*/
	static bool removeCollinearEdges(std::vector<Vec>& vertices);

	// Checks if the vertices have any collinear edges
	static bool hasCollinearEdges(const std::vector<Vec>& vertices);

	/**
	* Description:
	* Loads all polygons from an SVG file, polygons being stored as (points="...").
	*
	* Pre-conditions:
	* The SVG polygons must satisfy the rules of the Polygon class, otherwise the constructor will throw an exception.
	*
	* Input:
	* A path to an SVG file.
	*
	* Output:
	* A vector of polygons.
	*/
	static std::vector<Polygon> read_SVG_polygons(const char* pathToSVG);

protected:

	// Used in the move constructor
	Polygon() = default;

private:
	
	void initPos();
	void rotateVerticesBy(float degrees);
	void updateAbsoluteVertices();
	void setBoundingBox();

	Vec mPos;                                   // Centroid
	std::vector<Vec> mRelativeVertices;         // Only change during rotations
	std::vector<Vec> mAbsoluteVertices;         // Change during rotations and translations

	float mRotAngle = 0.f;

	Rect mAABB;
};

class ConvexPolygon : public Polygon
{
public:

	ConvexPolygon(std::vector<Vec> vertices);
	ConvexPolygon(Polygon other);

	void swap(ConvexPolygon& other) noexcept;
	
	ConvexPolygon(const ConvexPolygon& other);
	ConvexPolygon(ConvexPolygon&& other) noexcept;

	ConvexPolygon& operator=(ConvexPolygon other);
	ConvexPolygon& operator=(std::vector<Vec> vertices);

	~ConvexPolygon() = default;

	void offsetVerticesBy(float distance);

	static ConvexPolygon rectToPolygon(const Rect& rect);

	bool containsPoint(const Vec& point) const;


	// Add only unique components of a solution vector
	static void mergeResolution(Vec& base, const Vec& toAdd);

	// Return 0 vector if there is no collision
	static Vec resolveCollision(const ConvexPolygon& moveableShape, const ConvexPolygon& fixedShape);

	// Returns minimuim vector to resolve all collisions
	static Vec resolveCollisions(const ConvexPolygon& moveableShape, const std::vector<ConvexPolygon>& fixedShapes);

private:

	// Used in the move constructor
	ConvexPolygon() = default;

	// Used by ctor to check if the polygon is convex, also checks for collinear edges, self intersections, and a clockwise winding order
	bool isConvex();

	// Returns a list of the edges of the polygon rotated 90 degrees
	std::vector<Vec> getCollisionAxi() const;
};

