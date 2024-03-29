﻿#pragma once
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


Polygon::Polygon(std::vector<Vec>&& vertices) : mAbsoluteVertices{ std::move(vertices) }
{
    if (mAbsoluteVertices.size() < 3)            // Polygon must contain atleast 3 vertices
        throw(std::invalid_argument{ "Error: Open polygon\n" });
    else if (isClockwise(mAbsoluteVertices) == false)             // Polygon vertices must be in clockwise order
       throw(std::invalid_argument{ "Error: Polygon vertices not in clockwise order\n" });
    else if (findCollinearEdge(mAbsoluteVertices) != mAbsoluteVertices.size())        // Polygon cannot have collinear edges
        throw(std::invalid_argument{ "Error: Polygon has collinear edges\n" });
    else if (isSelfIntersecting(mAbsoluteVertices) == true)       // Polygon must be not self intersect
        throw(std::invalid_argument{ "Error: Polygon self intersects\n" });

    initPos();  // Determine center of the polygon
}

Polygon::Polygon(std::vector<Vec>& vertices) : mAbsoluteVertices{ vertices }
{
    if (mAbsoluteVertices.size() < 3)            // Polygon must contain atleast 3 vertices
        throw(std::invalid_argument{ "Error: Open polygon\n" });
    else if (isClockwise(mAbsoluteVertices) == false)             // Polygon vertices must be in clockwise order
       throw(std::invalid_argument{ "Error: Polygon vertices not in clockwise order\n" });
    else if (findCollinearEdge(mAbsoluteVertices) != mAbsoluteVertices.size())        // Polygon cannot have collinear edges
        throw(std::invalid_argument{ "Error: Polygon has collinear edges\n" });
    else if (isSelfIntersecting(mAbsoluteVertices) == true)       // Polygon must be not self intersect
        throw(std::invalid_argument{ "Error: Polygon self intersects\n" });

    initPos();  // Determine center of the polygon
}

Polygon::Polygon(std::vector<Vec>& vertices, int) : mAbsoluteVertices{ vertices }
{
    if (mAbsoluteVertices.size() < 3)   // Polygon must contain atleast 3 vertices
        throw(std::invalid_argument{ "Error: Open polygon\n" });
    else if (isSelfIntersecting(mAbsoluteVertices) == true)       // Polygon must be not self intersect
        throw(std::invalid_argument{ "Error: Polygon self intersects\n" });

    initPos();  // Determine center of the polygon
}

Polygon::Polygon(std::vector<Vec>&& vertices, int) : mAbsoluteVertices{ std::move(vertices) }
{
    if (mAbsoluteVertices.size() < 3)   // Polygon must contain atleast 3 vertices
        throw(std::invalid_argument{ "Error: Open polygon\n" });
    else if (isSelfIntersecting(mAbsoluteVertices) == true)       // Polygon must be not self intersect
        throw(std::invalid_argument{ "Error: Polygon self intersects\n" });

    initPos();  // Determine center of the polygon
}

bool Polygon::isClockwise(const std::vector<Vec>& vertices)
{
    float area = 0.f;

    for (int i = 0; i < vertices.size(); ++i)
    {
        // Get every adjacent pair of vertices
        Vec 𝙫0{ vertices[i] };
        Vec 𝙫1{ vertices[(i + 1) % vertices.size()] };

        // Find the area under the curve (2x actual area)
        area += (𝙫1.getX() - 𝙫0.getX()) * (𝙫1.getY() + 𝙫0.getY());
    }

    // If area is negative, the vertices are in counterclockwise order
    // If it's 0, the polygon self intersects
    return area > 0;
}

std::vector<Vec>::size_type Polygon::findCollinearEdge(const std::vector<Vec>& vertices)
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

        // If any of the cross products are 0, the edges are collinear
        if (v2.cross(v1) == 0.f)
            return ((i + 1) % vertices.size());
    }
    return vertices.size();
}

// Not indlucding adjacent edges (vertex joints and possibly adjacent collinear edge overlap)
bool Polygon::isSelfIntersecting(const std::vector<Vec>& vertices)
{
    auto edges = getEdges(vertices);

    // Compare every edge to edge[0], except for edge[1] and edge[last]
    for (int jEdge = 2; jEdge < edges.size() - 1; ++jEdge)
    {
        if (Vec::findIntersection(edges[0].first, edges[0].second, edges[jEdge].first, edges[jEdge].second).first != Solution::NO_SOLUTION)
            return true;
    }

    // Check for intersection between all remaining none adjacent edges
    for (int iEdge = 1; iEdge < edges.size(); ++iEdge)
    {
        for (int jEdge = iEdge + 2; jEdge < edges.size(); ++jEdge)
        {
            if (Vec::findIntersection(edges[iEdge].first, edges[iEdge].second, edges[jEdge].first, edges[jEdge].second).first != Solution::NO_SOLUTION)
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

const Vec& Polygon::getPos() const
{
    return mPos;
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

    for (int iMiddle = 0; iMiddle < vertices.size() && vertices.size() > 3; ++iMiddle)
    {
        // Try every consecutive trio of vertices
        auto prev = Circulator{ vertices, iMiddle - 1};
        auto middle = Circulator{ vertices, iMiddle };
        auto next = Circulator{ vertices, iMiddle + 1 };
           
        // -- Test if the vertex is convex -----------------------------------------
            
        Vec 𝙫0{ *middle - *prev };
        Vec 𝙫1{ *next - *middle };

        if (𝙫0.cross(𝙫1) > 0)   // Skip concave angles
            continue;

        // Define 3rd triangle edge
        Vec 𝙫2{ *prev - *next };

        // -- Test if any vertices fall inside the triangle -----------------------------------------

        bool found = false;
        auto current = Circulator{ vertices, iMiddle + 2};

        while (!found && current != prev)
        {
            // Define 3 vectors point from triangle vertices to current vertex
            Vec arr[3]{ {*current - *prev}, {*current - *middle}, {*current - *next} };

            // Cross each vector with its respective edge vector
            found = (𝙫0.cross(arr[0]) > 0) && (𝙫1.cross(arr[1]) > 0) && (𝙫2.cross(arr[2]) > 0);
                
            ++current;
        }

        // Skip triangles that contain vertices
        if (found)
            continue;

        // -- Clip the ear -----------------------------------------

        // Add the triangle to the list
        triangles.emplace_back(std::vector<Vec>{*prev, * middle, * next});

        // Remvoe the middle vertex
        vertices.erase(vertices.begin() + iMiddle);
        
        // Stay on current index for next iteration since the array shrunk by 1
        --iMiddle;
    }

    // Add the last triangle
    triangles.push_back(vertices);

    return triangles;
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

void Polygon::offsetVerticesBy(float distance)
{
    for (int iMiddle = 0; iMiddle < mAbsoluteVertices.size(); ++iMiddle)
    {
        // Try every consecutive pair of edges
        auto prev = Circulator{ mAbsoluteVertices, iMiddle - 1 };
        auto middle = Circulator{ mAbsoluteVertices, iMiddle };
        auto next = Circulator{ mAbsoluteVertices, iMiddle + 1 };

        Vec edge1{ *middle - *prev };
        Vec edge2{ *next - *middle };
        
        edge1.normalize();
        edge2.normalize();

        // Get the vector that bisects the angle formed by the edges
        auto direction = (edge1 + edge2);
        direction.normalize();
        
        //  Rotate the direction by -90 deg, the vector points out of the polygon
        direction *= Matrix{0.f, -1.f, 1.f, 0.f};

        // Using the calculated direction and provided distance, offset the relative vertex
        mRelativeVertices[iMiddle] += direction * distance;
    }

    // Update the absolute vertices to match the relative ones
    updateAbsoluteVertices();

    // Ensure that the polygon is still valid
    
    if (isClockwise(mAbsoluteVertices) == false)                  // Polygon vertices must be in clockwise order
        throw(std::invalid_argument{ "Error: Polygon vertices not in clockwise order\n" });
    else if (findCollinearEdge(mAbsoluteVertices) != mAbsoluteVertices.size())        // Polygon cannot have collinear edges
        throw(std::invalid_argument{ "Error: Polygon has collinear edges\n" });
    else if (isSelfIntersecting(mAbsoluteVertices) == true)       // Polygon must be not self intersect
        throw(std::invalid_argument{ "Error: Polygon self intersects\n" });
}

void Polygon::updateAbsoluteVertices()
{
    // Absolute vetices are the sum of the polygon's position and its corresponding relative vertices
    for (int i = 0; i < mRelativeVertices.size(); ++i)
    {
        mAbsoluteVertices[i] = mPos + mRelativeVertices[i];
    }
}

void Polygon::rotateVerticesBy(float degrees)
{
    // Convert adjustment into radians
    float radians = degrees * static_cast<float>(M_PI) / 180.f;
    
    // Build rotation matrix
    Matrix rotMatrix{cosf(radians), -sinf(radians), sinf(radians), cosf(radians)};

    // Multiply each relative vertex by the matrix
    for (auto& relativeVertex : mRelativeVertices)
    {
        relativeVertex *= rotMatrix;
    }
    
    // Update the absolute vertices to match the relative ones
    updateAbsoluteVertices();
}

ConvexPolygon::ConvexPolygon(std::vector<Vec>& vertices) : Polygon{ vertices, 0 }
{
    if (isConvex() == false)       // Polygon must be convex
        throw(std::invalid_argument{ "Error: Concave polygon\n" });
}

ConvexPolygon::ConvexPolygon(std::vector<Vec>&& vertices) : Polygon{ std::move(vertices), 0 }
{
    if (isConvex() == false)       // Polygon must be convex
        throw(std::invalid_argument{ "Error: Concave polygon\n" });
}


// Convex means that the next edge vector is to the right of the current one (negative cross product)
// Note: If the te polygon ends up convex, it is gaureenteed to be clockwise and have no collinear edges.
bool ConvexPolygon::isConvex()
{
    auto& vertices = getVertices();

    for (int i = 0; i < vertices.size(); ++i)
    {
        // Get every trio of vertices
        Vec v0{ vertices[i] };
        Vec v1{ vertices[(i + 1) % vertices.size()] };
        Vec v2{ vertices[(i + 2) % vertices.size()] };

        // Define 2 vectors from the first vertex to the other 2
        v1 -= v0;
        v2 -= v0;

        // If the signs are different, the polygon is not convex
        if (!(v1.cross(v2) < 0.f))
            return false;
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
        else if (solution * totalSolution <= 0)     // If the vector is in the perpendicular or in the opposite direction, just sum it
        {
            totalSolution += solution;
            continue;
        }
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

/**
* Implementation:
* read_SVG_polygons opens the provided file and copies all of its
* contents into an std::string. The string is then parsed using regex,
* the result is an iterator over all of the polygons' vertices. Each
* list of vertices is checked (and corrected) for a clockwise order,
* last vertex being a duplicate of the first, and a quadrilateral with a
* single collinear vertex.
*/
std::vector<Polygon> Polygon::read_SVG_polygons(const std::ifstream& svgFile)
{
    // Check if the file opened
    if (svgFile.is_open() == false)
        throw(std::invalid_argument{ "Error: Unable to open SVG file\n" });

    // Read file into a string buffer
    std::stringstream buffer;
    buffer << svgFile.rdbuf();

    // Move stream into string
    std::string svgString{std::move(buffer).str()};

    std::vector<Polygon> polygons;

    // -- Read in the polygons -----------------------------------------

    // Start at points=" and end at "
    std::regex polygonRegex("points=\"([^\"]*)\"");

    // Find the lists of vertices in the string
    std::sregex_iterator current(svgString.begin(), svgString.end(), polygonRegex);
    std::sregex_iterator end;
   
    // Iterate over each string of floats
    while (current != end)
    {
        std::vector<Vec> vertices;
        std::stringstream floats{ (current++)->str(1) };   // 1 means 1st capture group, '([^\"]*)'.

        // Read in each float, alternating between x and y
        while(floats.good())
        {
            float tempX, tempY;
            
            // Read in the x value
            floats >> tempX;
            
            // Check after reading in the x value
            if(!floats.good())
                throw(std::invalid_argument{ "Error: Missing y-value in vertex\n" });

            // read in the y value
            floats >> tempY;
            
            // Add the vertex to the list
            vertices.emplace_back(tempX, tempY);
        }
        
        // Make vertices clockwise
        if(!Polygon::isClockwise(vertices))
            std::reverse(vertices.begin(), vertices.end());

        // Remove last vertex if it is a duplicate of the first (typical for SVG polygons)
        if(vertices[0] == vertices[vertices.size() - 1])
            vertices.pop_back();

        // Remove extra vertex if there is one
        if (vertices.size() == 5)
        {
            auto index = Polygon::findCollinearEdge(vertices);
            if(index != vertices.size())
                vertices.erase(vertices.begin() + index);
        }

        // Add the polygon to the list
        polygons.push_back(vertices);
    }

    return polygons;
}
