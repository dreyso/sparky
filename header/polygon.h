#pragma once
#include "vec.h"

#include <vector>
#include <memory>


class Rect
{
public:

	Rect() = default;
	Rect(float x, float y, float width, float height) : x{ x }, y{ y }, w{ width }, h{ height }{}
	Rect(const Vec& pos, float width, float height) : x{ pos.getX() }, y{ pos.getY() }, w{ width }, h{ height }{}
	~Rect() = default;

	void swap(Rect& other);
	void set(float x, float y, float width, float height);
	void set(const Vec& pos, float width, float height);
	void setPos(const Vec& pos);
	Vec getPos() const;
	static bool isIntersecting(const Rect& a, const Rect& b);
	static Rect combine(const Rect& a, const Rect& b);

	float x = 0.f;
	float y = 0.f;
	float w = 0.f;
	float h = 0.f;
};

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
	*/

void offsetVerticesBy(std::vector<Vec>& vertices, float distance);

std::vector<Vec> offsetVerticesBy(const std::vector<Vec>& vertices, float distance);

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
	*/
	void rotateBy(float degrees);
	void rotateTo(float degrees);
	
	const Vec& getPos() const;

	float getRotAngle() const;
	
	const Rect& getAABB() const;

	// Get the smallest rectangle that can fit the polygon
	Rect getRectHull() const;


	const std::vector<Vec>& getVertices() const;

	/**
	* Description:
	* Breaks the polygon up into triangles via the ear clipping method.
	*/
	std::vector<ConvexPolygon> triangulate() const;

	/**
	* Description:
	* Determines the the winding order of the polygon's vertices
	* based on the sign of the net area under the polygon's edges.
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
	*/
	static std::vector<std::pair<Vec, Vec>> getEdges(const std::vector<Vec>& vertices);

	/**
	* Description:
	* Seachs through the vertices untill the first
	* collinear vertex.
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
	* Loads all polygons from an SVG file, polygons being stored as (points="..."). Minimized SVG's work best.
	*
	* Pre-conditions:
	* The list of vertices must be seperated by atleast 1 space, and the last character cannot be a space.
	* The SVG polygons must satisfy the rules of the Polygon class, otherwise the constructor will throw an exception.
	*/
	static std::vector<Polygon> readSvgPolygons(const char* pathToSVG);
	static Polygon readSvgPolygon(const char* pathToSVG);


protected:

	// Used in the move constructor
	Polygon() = default;

	// Checks if the the current vector is to the left of the next vector
	// Required for convex vertices
	static bool goingRight(const Vec& current, const Vec& next);

private:
	void integrityCheck();
	void initPos();
	void rotateVerticesBy(float degrees);
	void updateAbsoluteVertices();
	void setAABB();
	void updateAABB();


	Vec mPos;                                   // Centroid
	std::vector<Vec> mRelativeVertices;         // Only change during rotations
	std::vector<Vec> mAbsoluteVertices;         // Change during rotations and translations

	float mRotAngle = 0.f;

	// Dimensions of AABB do not change
	Rect mAABB;
	// The lower x, lower y corner of the AABB relative to the polygon's position
	Vec mAABB_Offset;
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
	static bool isConvex(const std::vector<Vec>& vertices);

	// Returns a list of the edges of the polygon rotated 90 degrees
	std::vector<Vec> getCollisionAxi() const;
};

