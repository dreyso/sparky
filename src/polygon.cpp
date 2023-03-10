#pragma once
#include "../header/polygon.h"
#include "../header/util.h"

#include <cmath>
#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <regex>
#include <limits>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cassert>


void Rect::swap(Rect& other)
{
    Rect temp{ other };
    other = *this;
    *this = temp;
}

void Rect::set(float x, float y, float width, float height)
{
    this->x = x;
    this->y = y;
    if (width <= 0)
        throw(std::invalid_argument{ "Error: Negative or 0 width\n" });
    this->w = width;
    if (height <= 0)
        throw(std::invalid_argument{ "Error: Negative or 0 height\n" });
    this->h = height;
}

void Rect::set(const Vec& pos, float width, float height)
{
    this->x = pos.getX();
    this->y = pos.getY();
    if (width <= 0)
        throw(std::invalid_argument{ "Error: Negative or 0 width\n" });
    this->w = width;
    if (height <= 0)
        throw(std::invalid_argument{ "Error: Negative or 0 height\n" });
    this->h = height;
}

void Rect::setPos(const Vec& pos)
{
    this->x = pos.getX();
    this->y = pos.getY();
}

Vec Rect::getPos() const
{
    return Vec{ x,y };
}

bool Rect::isIntersecting(const Rect& a, const Rect& b)
{
    // Check if a and b overlap on the x axis
    if (a.x > b.x + b.w || b.x > a.x + a.w)
        return false;

    // Check if a and b overlap on the y axis
    else if (a.y > b.y + b.h || b.y > a.y + a.h)
        return false;
    
    // The rectangles overlap on both axii, so they must intersect
    else
        return true;
}

// Finds the smallest rectangle that fits the 2 passed in rectangles
Rect Rect::combine(const Rect& a, const Rect& b)
{
    Rect combined;

    // Find smallest x
    combined.x = std::min(a.x, b.x);

    // Find largest x
    combined.w = std::max(a.x + a.w, b.x + b.w);

    // Find smallest y
    combined.x = std::min(a.y, b.y);

    // Find largest y
    combined.h = std::max(a.x + a.h, b.x + b.h);

    return combined;
}

void offsetVerticesBy(std::vector<Vec>& vertices, float distance)
{
    if (vertices.size() <= 3)
        return;

    auto calcDirection = [](const Vec& prev, const Vec& middle, const Vec& next) 
    {
        Vec edge1{ middle - prev };
        Vec edge2{ next - middle };

        edge1.normalize();
        edge2.normalize();

        // Get the vector that bisects the angle formed by the edges
        auto direction = (edge1 + edge2);
        direction.normalize();

        //  Rotate the direction by -90 deg, the vector points out of the polygon
        direction *= Matrix{ 0.f, -1.f, 1.f, 0.f };
        return direction;
    };

    // Handle first element being the middle vertex
    vertices.front() += calcDirection(vertices.back(), vertices.front(), vertices[1]) * distance;
    
    for (int iMiddle = 1; iMiddle < vertices.size() - 1; ++iMiddle)
    {
        int prev = iMiddle - 1;
        int next = iMiddle + 1;

        // Try every consecutive pair of edges
        Vec edge1{ vertices[iMiddle] - vertices[prev] };
        Vec edge2{ vertices[next] - vertices[iMiddle] };

        edge1.normalize();
        edge2.normalize();

        // Get the vector that bisects the angle formed by the edges
        auto direction = (edge1 + edge2);
        direction.normalize();

        //  Rotate the direction by -90 deg, the vector points out of the polygon
        direction *= Matrix{ 0.f, -1.f, 1.f, 0.f };

        // Using the calculated direction and provided distance, offset the relative vertex
        vertices[iMiddle] += direction * distance;
    }

    // Handle last element being the middle vertex
    vertices.back() += calcDirection(vertices[vertices.size() - 2], vertices.back(), vertices.front()) * distance;
}

std::vector<Vec> offsetVerticesBy(const std::vector<Vec>& vertices, float distance)
{
    std::vector<Vec> newVertices{ vertices };
    offsetVerticesBy(newVertices, distance);
    return newVertices;
}

Polygon::Polygon(std::vector<Vec> vertices) : mAbsoluteVertices{ std::move(vertices) }
{
    integrityCheck();
    initPos();  // Determine center of the polygon
    setAABB();   //  Init the bounding box
}

void Polygon::swap(Polygon& other) noexcept
{
    std::swap(mAbsoluteVertices, other.mAbsoluteVertices);
    std::swap(mRelativeVertices, other.mRelativeVertices);
    mPos.swap(other.mPos);
    {
        float temp{ mRotAngle };
        mRotAngle = other.mRotAngle;
        other.mRotAngle = temp;
    }
    mAABB.swap(other.mAABB);
    mAABB_Offset.swap(other.mAABB_Offset);
}

Polygon::Polygon(const Polygon& other) 
    : mAbsoluteVertices{ other.mAbsoluteVertices }, mRelativeVertices{ other.mRelativeVertices }, 
    mPos{ other.mPos }, mRotAngle{ other.mRotAngle }, mAABB{ other.mAABB }, mAABB_Offset{ other.mAABB_Offset }{}

Polygon::Polygon(Polygon&& other) noexcept : Polygon()
{
    swap(other);
}

Polygon& Polygon::operator=(Polygon other)
{
    swap(other);
    return *this;
}

Polygon& Polygon::operator=(std::vector<Vec> vertices)
{
    Polygon temp{ std::move(vertices) };
    swap(temp);
    return *this;
}

bool Polygon::isClockwise(const std::vector<Vec>& vertices)
{
    float area = 0.f;

    for (int i = 0; i < vertices.size() - 1; ++i)
    {
        // Get every (but last) adjacent pair of vertices
        Vec 𝙫0{ vertices[i] };
        Vec 𝙫1{ vertices[i + 1] };

        // Find the area under the curve (2x actual area)
        area += (𝙫1.getX() - 𝙫0.getX()) * (𝙫1.getY() + 𝙫0.getY());
    }
    
    // Get last adjacent pair of vertices
    area += (vertices.front().getX() - vertices.back().getX()) * (vertices.front().getY() + vertices.back().getY());

    // If area is negative, the vertices are in counterclockwise order
    // If it's 0, the polygon self intersects
    return area > 0;
}

bool Polygon::removeCollinearEdges(std::vector<Vec>& vertices)
{
    if (vertices.size() < 3)
        return false;

    // Record if a collinear edge has been removed in the loop below
    bool removedEdge = false;
    
    int iFirst = -1;
    while (iFirst < vertices.size() && vertices.size() >= 3)
    {
        // Get every trio of vertices
        ++iFirst;
        int second = iFirst + 1;
        int third = iFirst + 2;

        // Wrap second and third vertices
        if (iFirst > vertices.size() - 3)
        {
            iFirst = cycleIndex(iFirst, vertices.size());
            second = cycleIndex(second, vertices.size());
            third = cycleIndex(third, vertices.size());
        }

        // Define 2 vectors from the first vertex to the other 2
        auto v1{ vertices[second] - vertices[iFirst] };
        auto v2{ vertices[third] - vertices[iFirst] };

        // If any of the cross products are 0, the edges are collinear
        if (v2.cross(v1) == 0.f)
        {
            // Delete the vertex
            vertices.erase(vertices.begin() + second);
            removedEdge = true;

            // Last element check
            iFirst = std::min((int) vertices.size() - 1, iFirst);

            // Avoid moving forward without checking again
            --iFirst;
        }
    }
    return removedEdge;
}

bool Polygon::hasCollinearEdges(const std::vector<Vec>& vertices)
{
    for (int iMiddle = 0; iMiddle < vertices.size() && vertices.size() >= 3; ++iMiddle)
    {
        // Get every trio of vertices
        auto prev = Circulator{ vertices, iMiddle - 1 };
        auto next = Circulator{ vertices, iMiddle + 1 };

        Vec v0{ *prev };
        Vec v1{ vertices[iMiddle] };
        Vec v2{ *next };

        // Define 2 vectors from the first vertex to the other 2
        v1 -= v0;
        v2 -= v0;

        // If any of the cross products are 0, the edges are collinear
        if (v2.cross(v1) == 0.f)
        {
            return true;
        }
    }
    return false;
}

// Not indlucding adjacent edges (vertex joints and possibly adjacent collinear edge overlap)
bool Polygon::isSelfIntersecting(const std::vector<Vec>& vertices)
{
    // Triangles cannot self intersect
    if (vertices.size() == 3)
        return false;

    auto edges = getEdges(vertices);

    // Check for intersection between all edges
    for (int iEdge = 0; iEdge < edges.size(); ++iEdge)
    {
        for (int jEdge = iEdge + 2; jEdge < edges.size(); ++jEdge)
        {
            // First and last elements are adjacent, skip them
            if(iEdge == 0 && jEdge == edges.size() - 1)
                continue;

            auto solution = Vec::findIntersection(edges[iEdge].first, edges[iEdge].second, edges[jEdge].first, edges[jEdge].second);

            // All non-adjacent edges must have no solution
            if (std::get<Solution>(solution) != Solution::NO_SOLUTION)
                return true;
        }
    }

    return false;
}

std::vector<std::pair<Vec, Vec>> Polygon::getEdges(const std::vector<Vec>& vertices)
{
    std::vector<std::pair<Vec, Vec>> edges(vertices.size());

    for (int i = 0; i < edges.size(); ++i)
    {
        // Store the offset of each edge
        edges[i].first = vertices[i];

        // Subtract current vertex from the next one for a clockwise perimeter
        edges[i].second = vertices[(i + 1) % edges.size()] - vertices[i];
    }
    return edges;
}

void Polygon::integrityCheck()
{
    removeCollinearEdges(mAbsoluteVertices);    // Remove all collinear vertices from the polygon

    if (mAbsoluteVertices.size() < 3)            // Polygon must contain atleast 3 vertices
        throw(std::invalid_argument{ "Error: Open polygon\n" });
    else if (isClockwise(mAbsoluteVertices) == false)             // Polygon vertices must be in clockwise order
        throw(std::invalid_argument{ "Error: Polygon vertices not in clockwise order\n" });
    else if (isSelfIntersecting(mAbsoluteVertices) == true)       // Polygon must be not self intersect
        throw(std::invalid_argument{ "Error: Polygon self intersects\n" });
}

// Bourke, Paul (July 1997). "Polygons and meshes"
// https:// stackoverflow.com/questions/2792443/finding-the-centroid-of-a-polygon
// https:// math.stackexchange.com/questions/90463/how-can-i-calculate-the-centroid-of-polygon
// Works for all simple polygons (not self-intersecting and no holes)
// Vertices can be in clockwise or counter-clockwise order

// A variation of the shoelace algorithm
// Partiitions a polygon into triangles (starting from origin)
// Finds area and centroid of each triangle
// Multplies the two together to get a vector for each triangle
// Sums all of these vectors, and divides it by the total area of the polygon
// The resulting vector is the centroid
void Polygon::initPos()
{
    Vec centroid = { 0.f, 0.f };
    // Area can be signed
    float totalArea = 0.f;
    float triangleArea = 0.f;  // Twice the area of a shoelace triangle

    for (int i = 0; i < mAbsoluteVertices.size(); ++i)
    {
        // Cross every adjacent pair of vertices
        Vec 𝙫0{ mAbsoluteVertices[i] };
        Vec 𝙫1{ mAbsoluteVertices[(i + 1) % mAbsoluteVertices.size()] };

        // Find the area and centroid of each resulting triangle  
        triangleArea = 𝙫0.cross(𝙫1);    // Second vector, 𝙫1, must be to the left of 𝙫0 for positive area (this doesn't affect the end result)
        totalArea += triangleArea;
        centroid += (𝙫0 + 𝙫1) * triangleArea;
    }
    
    // Finalize the vector sum (divide by 3 for centroids)
    // The area of the triangles is doubled, but the ratio of triangles to total area remains the same
    centroid /= (3.f * totalArea);
    mPos = centroid;
    
    mRelativeVertices = mAbsoluteVertices;
    // Init the relative vertices
    for (auto& relativeVertex : mRelativeVertices)
    {
        relativeVertex -= mPos;
    }
}

const std::vector<Vec>& Polygon::getVertices() const
{
    return mAbsoluteVertices;
}

const Rect& Polygon::getAABB() const
{
    return mAABB;
}

Rect Polygon::getRectHull() const
{
    float smallestX, smallestY, largestX, largestY;
    smallestX = smallestY = std::numeric_limits<float>::infinity();
    largestX = largestY = -std::numeric_limits<float>::infinity();

    for (auto& vertex : mAbsoluteVertices)
    {
        // Smallest x
        smallestX = std::min(smallestX, vertex.getX());
        // Smallest y
        smallestY = std::min(smallestY, vertex.getY());
        // Largest x
        largestX = std::max(largestX, vertex.getX());
        // Largest y
        largestY = std::max(largestY, vertex.getY());
    }

    // Convert the extrema into the x,y,w,h of a rectangle
    return Rect{ smallestX, smallestY, largestX - smallestX, largestY - smallestY };
}


const Vec& Polygon::getPos() const
{
    return mPos;
}

float Polygon::getRotAngle() const
{
    return mRotAngle;
}

std::vector<ConvexPolygon> Polygon::triangulate() const
{
    /**
    * Vertices must be convex
    * None of the other vertices must be in the triangle
    * At the end, the primary vertex is removed
    */

    auto vertices{ mAbsoluteVertices };     // Copy of polygon's vertices
    std::vector<ConvexPolygon> triangles;   // Array to be returned
    //CircularList<std::vector<Vec>> circVertices{ vertices };

     // Try every consecutive trio of vertices start is arbitrary
    //auto middle = Circulator{ vertices, static_cast<int>(vertices.size()) - 1 };
    int iMiddle = -1;
    while (vertices.size() > 3)
    {      
        // Try next trio
        iMiddle = cycleIndex(++iMiddle, vertices.size());
        int prev = cycleIndex(iMiddle - 1, vertices.size());
        int next = cycleIndex(iMiddle + 1, vertices.size());

        // -- Test if the vertex is convex -----------------------------------------
            
        Vec 𝙫0{ vertices[iMiddle] - vertices[prev] };
        Vec 𝙫1{ vertices[next] - vertices[iMiddle] };
      
        // Skip concave angles
        if (!goingRight(𝙫0, 𝙫1))
            continue;

        // Define 3rd triangle edge
        Vec 𝙫2{ vertices[prev] - vertices[next] };

        // -- Test if any vertices fall inside the triangle -----------------------------------------

        bool found = false;
        auto iCurrent = cycleIndex(iMiddle + 2, vertices.size());

        while (!found && iCurrent != prev)
        {
            // Define 3 vectors point from triangle vertices to current vertex
            Vec arr[3]{ {vertices[iCurrent] - vertices[prev]}, {vertices[iCurrent] - vertices[iMiddle]}, {vertices[iCurrent] - vertices[next]} };

            // Cross each vector with its respective edge vector
            found = (goingRight(𝙫0, arr[0]) && goingRight(𝙫1, arr[1]) && goingRight(𝙫2, arr[2]));
                
            iCurrent = cycleIndex(++iCurrent, vertices.size());
        }

        // Skip triangles that contain vertices
        if (found)
            continue;

        // -- Clip the ear -----------------------------------------

        // Add the triangle to the list
        triangles.emplace_back(std::vector<Vec>{vertices[prev], vertices[iMiddle], vertices[next]});

        // Remove the middle vertex
        vertices.erase(vertices.begin() + iMiddle);

        // Last element check
        iMiddle = std::min((int) vertices.size() - 1, iMiddle);

        // Avoid changing indices
        iMiddle -= 1;
    }

    // Add the last triangle
    triangles.emplace_back(vertices);

    return triangles;
}

void Polygon::moveBy(const Vec& addToPos)
{
    mPos += addToPos;
    updateAbsoluteVertices();
    updateAABB();
}
void Polygon::moveTo(const Vec& pos)
{
    mPos = pos;
    updateAbsoluteVertices();
    updateAABB();
}

void Polygon::rotateTo(float degrees)
{
    if(fabs(degrees) > 360.f)
        throw(std::invalid_argument{ "Error: Rotation angle must between [-360.f, 360.f]\n" });

    // Rotate by the amount needed to reach x degrees
    rotateVerticesBy(degrees - mRotAngle);
    
    // Update current rotation tracker
    mRotAngle = degrees;
}

void Polygon::rotateBy(float degrees)
{
    if (fabs(degrees) > 360.f)
        throw(std::invalid_argument{ "Error: Rotation angle must between [-360.f, 360.f]\n" });

    rotateVerticesBy(degrees);

    // Update current rotation tracker
    mRotAngle += degrees;

    // Prevent float overflows when rotating in a loop
    if (mRotAngle >= 360.f)
        mRotAngle -= 360.f;
    else if (mRotAngle <= -360.f)
        mRotAngle += 360.f;
}

void Polygon::updateAbsoluteVertices()
{
    // Absolute vetices are the sum of the polygon's position and its corresponding relative vertices
    for (int i = 0; i < mRelativeVertices.size(); ++i)
        mAbsoluteVertices[i] = mPos + mRelativeVertices[i];
}

void Polygon::rotateVerticesBy(float degrees)
{
    // Convert adjustment into radians
    float radians = degrees * static_cast<float>(M_PI) / 180.f;
    
    // Build rotation matrix
    Matrix rotMatrix{cosf(radians), -sinf(radians), sinf(radians), cosf(radians)};

    // Multiply each relative vertex by the matrix
    for (auto& relativeVertex : mRelativeVertices)
        relativeVertex *= rotMatrix;
    
    // Update the absolute vertices to match the relative ones
    updateAbsoluteVertices();
}

void Polygon::setAABB()
{
    // Find the longest relative vertex (distance from centroid)
    Vec radius{ 0.f, 0.f };
    
    for (auto& vertex : mRelativeVertices)
    {
        if (radius < vertex)
            radius = vertex;
    }

    // Set the position of the AABB relative to the position of the polygon
    mAABB_Offset = Vec{ -1.f, -1.f };
    mAABB_Offset *= radius.getMagnitude();

    // Create the AABB, centered on the centroid
    mAABB.set(mPos + mAABB_Offset, abs(2.f * mAABB_Offset.getX()), abs(2.f * mAABB_Offset.getY()));
}

void Polygon::updateAABB()
{
    mAABB.setPos(mPos + mAABB_Offset);
}

ConvexPolygon::ConvexPolygon(std::vector<Vec> vertices) : Polygon{ std::move(vertices) }
{
    if (isConvex(getVertices()) == false)       // Polygon must be convex
        throw(std::invalid_argument{ "Error: Concave polygon\n" });
}

ConvexPolygon::ConvexPolygon(Polygon other) : ConvexPolygon()
{
    // Swap data at base class level, ConvexPolygon has no fields to swap
    Polygon::swap(other);

    // Ensure that the polygon is convex
    if (isConvex(getVertices()) == false)       
        throw(std::invalid_argument{ "Error: Concave polygon\n" });
}

void ConvexPolygon::swap(ConvexPolygon& other) noexcept
{
    // Swap data at base class level, ConvexPolygon has no fields to swap
    Polygon::swap(other);
}

ConvexPolygon::ConvexPolygon(const ConvexPolygon& other) : Polygon{ other }{}

ConvexPolygon::ConvexPolygon(ConvexPolygon&& other) noexcept : ConvexPolygon()
{
    swap(other);
}

ConvexPolygon& ConvexPolygon::operator=(ConvexPolygon other)
{
    swap(other);
    return *this;
}

ConvexPolygon& ConvexPolygon::operator=(std::vector<Vec> vertices)
{
    ConvexPolygon temp{ std::move(vertices) };
    swap(temp);
    return *this;
}

// Convex means that the current edge vector is to the left of the next one (negative cross product)
// Note: If the the polygon ends up convex, it is gaureenteed to be clockwise and have no collinear edges.
bool ConvexPolygon::isConvex(const std::vector<Vec>& vertices)
{
    for (int i = 0; i < vertices.size(); ++i)
    {
        // Get every trio of vertices
        Vec v0{ vertices[i] };
        Vec v1{ vertices[(i + 1) % vertices.size()] };
        Vec v2{ vertices[(i + 2) % vertices.size()] };

        // Define 2 vectors from the first vertex to the other 2
        v1 -= v0;
        v2 -= v0;

        // Next edge must be to the right of the current one
        if (!goingRight(v1, v2))
            return false;
    }
   
    return true;
}

ConvexPolygon ConvexPolygon::rectToPolygon(const Rect& rect)
{
    // Determine the region's vertices
    std::vector<Vec> collisionBoxVertices
    {
        Vec{ static_cast<float>(rect.x), static_cast<float>(rect.y) },
        Vec{ static_cast<float>(rect.x), static_cast<float>(rect.y + rect.h) },
        Vec{ static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y + rect.h) },
        Vec{ static_cast<float>(rect.x + rect.w), static_cast<float>(rect.y) }
    };

    // Create and return the convex polygon
    return ConvexPolygon{std::move(collisionBoxVertices)};
}

bool ConvexPolygon::containsPoint(const Vec& point) const
{
    auto& vertices = this->getVertices();
    auto edges = Polygon::getEdges(vertices);

    for (int i = 0; i < vertices.size(); ++i)
    {
        // Cross each edge vector with a vector that points from the vertex to the query point
        // if the cross product is not greater than 0, then the point is outside of the polygon
        if (!goingRight(edges[i].second, point - vertices[i]))
            return false;
    }
    // All of the cross products were less than or equal to 0, so the point is inside the polygon
    return true;
}

// Add only unique components of a solution vector
void ConvexPolygon::mergeResolution(Vec& base, const Vec& toAdd)
{
    // Ignore trivial solutions
    if (toAdd.isZeroVector())    
        return;
    
    // If needed, initialize the base vector to a non-zero vector
    if (base.isZeroVector())   
    {
        base = toAdd;
        return;
    }

    // If the vector is in the perpendicular or in the opposite direction, just sum it
    if (toAdd * base <= 0)    
    {
        base += toAdd;
        return;
    }
    
    // -- Add the vector's extra direction/magnitude to the base vector -----------------------------------------

    // Get the common direction between the toAdd and the base, as a ratio of the base
    float duplicate = toAdd.ratioProjectOn(base);  

    // Cap the duplicate ratio to its own length
    duplicate = std::min(duplicate, 1.f);        

    // Subtract the duplicate vector from the toAdd vector, and add it to the base
    base += toAdd - (duplicate * base);            
}

Vec ConvexPolygon::resolveCollisions(const ConvexPolygon& moveableShape, const std::vector<ConvexPolygon>& fixedShapes)
{
    Vec totalSolution{ 0.f, 0.f };

    for (auto& iFixedShape : fixedShapes)
    {
        // Get every collision solution between the moveable and fixed shapes
        Vec solution{ resolveCollision(moveableShape, iFixedShape) };
        
        // Add the current solution to the total
        mergeResolution(totalSolution, solution);
    }
    return totalSolution;
}

// Use seperating axis theorm to find a resolution vector
Vec ConvexPolygon::resolveCollision(const ConvexPolygon& moveableShape, const ConvexPolygon& fixedShape)
{
    std::vector<Vec> axi;
    {
        std::vector<Vec> moveableShapeAxi{ moveableShape.getCollisionAxi() };
        std::vector<Vec> fixedShapeAxi{ fixedShape.getCollisionAxi() };

        // Combine both shapes' axi
        move(moveableShapeAxi.begin(), moveableShapeAxi.end(), std::back_inserter(axi));
        move(fixedShapeAxi.begin(), fixedShapeAxi.end(), std::back_inserter(axi));
    }

    float smallestDepth = std::numeric_limits<float>::infinity();
    Vec smallestDepthDirection;

    // Project
    for (auto& axis : axi)
    {
        // Find max and min of each shape on each axis
        // Use "scalar" projection to avoid extra division operations
        // Scalar projecting on to a normlized vector simplifies to the dot product

        float moveableShapeMax = -std::numeric_limits<float>::infinity();
        float moveableShapeMin = std::numeric_limits<float>::infinity();
        for (auto& vertex : moveableShape.getVertices())
        {
            float partialScalarProjection = vertex * axis;
            moveableShapeMax = std::max(moveableShapeMax, partialScalarProjection);
            moveableShapeMin = std::min(moveableShapeMin, partialScalarProjection);
        }

        float fixedShapeMax = -std::numeric_limits<float>::infinity();
        float fixedShapeMin = std::numeric_limits<float>::infinity();
        for (auto& vertex : fixedShape.getVertices())
        {
            float partialScalarProjection = vertex * axis;
            fixedShapeMax = std::max(fixedShapeMax, partialScalarProjection);
            fixedShapeMin = std::min(fixedShapeMin, partialScalarProjection);
        }

        // Check for overlap
        if (!(moveableShapeMin < fixedShapeMax && moveableShapeMax > fixedShapeMin))
            return Vec{ 0.f, 0.f };

        // Find overlap, always positive, regardless of axi / vertex direction
        float temp = std::min(moveableShapeMax - fixedShapeMin, fixedShapeMax - moveableShapeMin);

        // Update smallest overlap
        if (temp < smallestDepth)
        {
            smallestDepth = temp;
            smallestDepthDirection = axis;
        }
    }

    // Correct the direction
    Vec direction = moveableShape.getPos() - fixedShape.getPos();   // Get vector that points from fixed to moveable shape
    if (direction * smallestDepthDirection < 0.f)   // If the vector is in the opposite direction, invert it
        smallestDepthDirection *= -1.f;

    // Apply the overlap in that direction
    return smallestDepthDirection * smallestDepth;
}

std::vector<Vec> ConvexPolygon::getCollisionAxi() const
{
    auto& vertices = getVertices();

    std::vector<Vec> edges(vertices.size());
    for (int i = 0; i < edges.size(); ++i)
    {
        // Subtract current vertex from the next one for a clockwise perimeter
        edges[i] = vertices[(i + 1) % edges.size()] - vertices[i];

        // Rotate the vector perpendicular to itself
        edges[i] *= Matrix{ 0, -1, 1, 0 };
        edges[i].normalize();
    }
    return edges;
}

/**
* Implementation:
* Takes raw SVG polygon text and converts it into a list of vectors.
* SVG format varies, so the function replaces anything other than digits,
* decimal points, or - sign with spaces. Spaces at the end of the string
* are removed to allow istream to catch EOF on time.
*/
static std::vector<Vec> readInFloats(std::string SVG_PolygonText)
{
    // Replace anything other than digits or a decimal point with spaces
    for (char& iChar : SVG_PolygonText)
    {
        if (!(std::isdigit(iChar) || iChar == '.' || iChar == '-'))
            iChar = ' ';
    }
    // Remove any spaces at the end of the list
    while (SVG_PolygonText[SVG_PolygonText.size() - 1] == ' ')
        SVG_PolygonText.pop_back();

    // Convert string into a stream for easy reading in
    std::stringstream floats{ std::move(SVG_PolygonText) };   // 1 means 1st capture group, '([^\"]*)'.
    std::vector<Vec> vertices;

    while (floats.good())
    {
        float tempX, tempY;

        // Read in the x value
        floats >> tempX;

        // Check after reading in the x value
        if (!floats.good())
            throw(std::invalid_argument{ "Error: Missing y-value in vertex\n" });

        // read in the y value
        floats >> tempY;

        // Add the vertex to the list
        vertices.emplace_back(tempX, tempY);
    }
    return vertices;
}

/**
* Implementation:
* SVG files do not gaureentee the order of the vertices,
* so they may need to be reversed. The first and last vertex
* may be identical, in which case it is removed.
*/
static void fixSvgVertices(std::vector<Vec>& vertices)
{
    // Make vertices clockwise
    if (!Polygon::isClockwise(vertices))
        std::reverse(vertices.begin(), vertices.end());

    // Remove last vertex if it is a duplicate of the first (typical for SVG polygons)
    if (vertices[0] == vertices[vertices.size() - 1])
        vertices.pop_back();
}


/**
* Implementation:
* Opens the provided file and copies all of its
* contents into an std::string. The string is then parsed using regex,
* the result is an iterator over all of the polygons' vertices. Each
* list of vertices is checked (and corrected) for a clockwise order and
* the last vertex being a duplicate of the first.
*/
std::vector<Polygon> Polygon::readSvgPolygons(const char* pathToSVG)
{
    // Open the file
    std::ifstream svgFile{ pathToSVG };

    // Check if the file opened
    if (svgFile.is_open() == false)
        throw(std::invalid_argument{ "Error: Unable to open SVG file\n" });

    // Read file into a string buffer
    std::stringstream buffer;
    buffer << svgFile.rdbuf();

    // Move stream into string
    std::string svgString{std::move(buffer).str()};

    std::vector<Polygon> polygons;

    // -- Read and parse the file -----------------------------------------

    // Start at points=" and end at "
    std::regex polygonRegex("points=\"([^\"]*)\"");

    // Find the lists of vertices in the string
    std::sregex_iterator current(svgString.begin(), svgString.end(), polygonRegex);
    std::sregex_iterator end;
   
    // -- Parse the vertices and create the polygons -----------------------------------------

    // Iterate over each string of floats
    while (current != end)
    {
        // Convert captured group into a list of vertices
        auto vertices{ readInFloats((current++)->str(1)) };
        
        // Fix order of vertices and remove duplicate vertex at the end
        fixSvgVertices(vertices);

        // Add the polygon to the list
        polygons.emplace_back(std::move(vertices));
    }

    return polygons;
}

/**
* Implementation:
* Opens the provided file and copies all of its
* contents into an std::string. The string is then parsed using regex,
* the result is an iterator over all of the polygon's vertices. The
* list of vertices is checked (and corrected) for a clockwise order and
* the last vertex being a duplicate of the first.
*/
Polygon Polygon::readSvgPolygon(const char* pathToSVG)
{
    // Open the file
    std::ifstream svgFile{ pathToSVG };

    // Check if the file opened
    if (svgFile.is_open() == false)
        throw(std::invalid_argument{ "Error: Unable to open SVG file\n" });

    // Read file into a string buffer
    std::stringstream buffer;
    buffer << svgFile.rdbuf();

    // Move stream into string
    std::string svgString{ std::move(buffer).str() };

    // -- Read and parse the file -----------------------------------------

    // Start at points=" and end at "
    std::regex polygonRegex("points=\"([^\"]*)\"");

    // Find the lists of vertices in the string
    std::smatch match;
    if (!std::regex_search(svgString, match, polygonRegex))
        throw(std::invalid_argument{ "Error: No polygon in SVG file\n" });
 
    // -- Parse the vertices and create the polygon -----------------------------------------

    // Convert captured group into a list of vertices
    auto vertices{ readInFloats(match[1]) };

    // Fix order of vertices and remove duplicate vertex at the end
    fixSvgVertices(vertices);

    // Add the polygon to the list
    return Polygon{ std::move(vertices) };
}

bool Polygon::goingRight(const Vec& current, const Vec& next)
{
    return current.cross(next) < 0.f;
}