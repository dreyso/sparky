#include "../header/polygon.h"
#include <cmath>
#include <vector>
#include <iterator>
#include <limits>
#include <SDL_stdinc.h> // For M_PI

#include <stdexcept>


Polygon::Polygon(std::vector<Vec>&& vertices) : mAbsoluteVertices{ std::move(vertices) }
{
    if (mAbsoluteVertices.size() < 3)            // Polygon must contain atleast 3 vertices
        throw(std::runtime_error{ "Error: Open polygon\n" });
    else if (isClockwise() == false)             // Polygon vertices must be in clockwise order
        throw(std::runtime_error{ "Error: Polygon vertices not in clockwise order\n" });
    else if (hasCollinearEdges() == true)        // Polygon cannot have collinear edges
        throw(std::runtime_error{ "Error: Polygon has collinear edges\n" });
    else if (isSelfIntersecting() == true)       // Polygon must be not self intersect
        throw(std::runtime_error{ "Error: Polygon self intersects\n" });

    initPos();  // Determine center of the polygon
}

Polygon::Polygon(std::vector<Vec>& vertices) : mAbsoluteVertices{ vertices }
{
    if (mAbsoluteVertices.size() < 3)            // Polygon must contain atleast 3 vertices
        throw(std::runtime_error{ "Error: Open polygon\n" });
    else if (isClockwise() == false)             // Polygon vertices must be in clockwise order
        throw(std::runtime_error{ "Error: Polygon vertices not in clockwise order\n" });
    else if (hasCollinearEdges() == true)        // Polygon cannot have collinear edges
        throw(std::runtime_error{ "Error: Polygon has collinear edges\n" });
    else if (isSelfIntersecting() == true)       // Polygon must be not self intersect
        throw(std::runtime_error{ "Error: Polygon self intersects\n" });

    initPos();  // Determine center of the polygon
}

Polygon::Polygon(std::vector<Vec>& vertices, int) : mAbsoluteVertices{ vertices }
{
    if (mAbsoluteVertices.size() < 3)   // Polygon must contain atleast 3 vertices
        throw(std::runtime_error{ "Error: Open polygon\n" });

    initPos();  // Determine center of the polygon
}

Polygon::Polygon(std::vector<Vec>& vertices, int) : mAbsoluteVertices{ std::move(vertices) }
{
    if (mAbsoluteVertices.size() < 3)   // Polygon must contain atleast 3 vertices
        throw(std::runtime_error{ "Error: Open polygon\n" });

    initPos();  // Determine center of the polygon
}

bool Polygon::isClockwise() const
{
    float area = 0.f;

    for (int i = 0; i < mAbsoluteVertices.size(); ++i)
    {
        // Get every adjacent pair of vertices
        Vec 𝙫0{ mAbsoluteVertices[i] };
        Vec 𝙫1{ mAbsoluteVertices[(i + 1) % mAbsoluteVertices.size()] };

        // Find the area under the curve (2x actual area)
        area += (𝙫1.getX() - 𝙫0.getX()) * (𝙫1.getY() + 𝙫0.getY());
    }

    // If area is negative, the veretices are in counterclockwise order
    // If it's 0, the polygon self intersects
    return area > 0;
}

bool Polygon::hasCollinearEdges() const
{
    for (int i = 0; i < mAbsoluteVertices.size(); ++i)
    {
        // Get every trio of vertices
        Vec v0{ mAbsoluteVertices[i] };
        Vec v1{ mAbsoluteVertices[(i + 1) % mAbsoluteVertices.size()] };
        Vec v2{ mAbsoluteVertices[(i + 2) % mAbsoluteVertices.size()] };

        // Define 2 vectors from the first vertex to the other 2
        v1 -= v0;
        v2 -= v0;

        // If any of the cross products are 0, the edges are collinear
        if (v2.cross(v1) == 0.f)
            return false;
    }
    return true;
}

bool Polygon::isSelfIntersecting() const
{
    auto edges = getEdges();

    for (auto& iEdge : edges)
    {
        for (auto& jEdge : edges)
        {
            if (Vec::findIntersection(iEdge.first, iEdge.second, jEdge.first, jEdge.second).first != Solution::NO_SOLUTION)
                return false;
        }
    }
}

std::vector<std::pair<Vec, Vec>> Polygon::getEdges() const
{
    auto& vertices = getVertices();

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
        triangleArea = 𝙫1.cross(𝙫0);    // First vector, 𝙫0, must be on the right of 𝙫1 for positive area (this doesn't affect the end result)
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

const Vec& Polygon::getPos() const
{
    return mPos;
}

void Polygon::moveBy(Vec addToPos)
{
    mPos += addToPos;
    updateAbsoluteVertices();
}
void Polygon::moveTo(Vec pos)
{
    mPos = pos;
    updateAbsoluteVertices();
}
void Polygon::rotateBy(float addToAngle)
{
    mRotAngle += addToAngle;
    updateRotation();
    updateAbsoluteVertices();
}
void Polygon::rotateTo(float angle)
{
    mRotAngle = angle;
    updateRotation();
    updateAbsoluteVertices();
}

void Polygon::updateAbsoluteVertices()
{
    // Absolute vetices are the sum of the polygon's position and its corresponding relative vertices
    for (int i = 0; i < mRelativeVertices.size(); ++i)
    {
        mAbsoluteVertices[i] = mPos + mRelativeVertices[i];
    }
}

void Polygon::updateRotation()
{
    float radians = mRotAngle * static_cast<float>(M_PI) / 180.f;
    Matrix rotMatrix{cosf(radians), -sinf(radians), sinf(radians), cosf(radians)};
    for (auto& relativeVertex : mRelativeVertices)
    {
        relativeVertex *= rotMatrix;
    }
}

ConvexPolygon::ConvexPolygon(std::vector<Vec>& vertices) : Polygon{ vertices, 0 }
{
    if (isConvex() == false)       // Polygon must be convex
        throw(std::runtime_error{ "Error: Concave polygon\n" });
}

ConvexPolygon::ConvexPolygon(std::vector<Vec>& vertices) : Polygon{ std::move(vertices), 0 }
{
    if (isConvex() == false)       // Polygon must be convex
        throw(std::runtime_error{ "Error: Concave polygon\n" });
}

bool ConvexPolygon::isConvex()
{
    auto& vertices = getVertices();

    bool prevPositive = (vertices[2] - vertices[0]).cross(vertices[1] - vertices[0]) >= 0.f;

    for (int i = 1; i < vertices.size(); ++i)
    {
        // Get every trio of vertices
        Vec v0{ vertices[i] };
        Vec v1{ vertices[(i + 1) % vertices.size()] };
        Vec v2{ vertices[(i + 2) % vertices.size()] };

        // Define 2 vectors from the first vertex to the other 2
        v1 -= v0;
        v2 -= v0;

        // Find the current direction of the polygon's edge
        bool currentPositive = v2.cross(v1) >= 0.f;

        // If the signs are different, the polygon is not convex
        if (currentPositive ^ prevPositive)
            return false;

        prevPositive = currentPositive;
    }
    return true;
}

Vec ConvexPolygon::resolveCollisions(const ConvexPolygon& moveableShape, const std::vector<ConvexPolygon>& fixedShapes)
{
    Vec totalSolution{ 0.f,0.f };

    for (auto iFixedShape : fixedShapes)
    {
        // Get every collision solution between the moveable and fixed shapes
        Vec solution{ resolveCollision(moveableShape, iFixedShape) };
        if (solution.isZeroVector())    // ignore trivial solutions
            continue;
        else if (totalSolution.isZeroVector())   // Initialize the overall solution variable to a non-zero vector
            totalSolution = solution;
        else   // This this iteration's solution to the total, but only extra direction/magnitude
        {
            float duplicate = solution.scalarProjectOn(totalSolution);  // Get the common direction between the current solution and the total
            duplicate /= totalSolution.getMagnitude();                  // Make duplicate a ratio of the solution
            duplicate = std::min(duplicate, 1.f);                       // Cap the duplicate ratio to its own length
            totalSolution += solution - (duplicate * totalSolution);    // Subtract the duplicate vector from the current solution, and add it to the total
        }
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

