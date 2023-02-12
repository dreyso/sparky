#pragma once
#include "vec.h"
#include "Polygon.h"
#include "util.h"
#include "grid.h"

#include <memory>
#include <vector>
#include <fstream>


namespace MapInternals
{
    class TriangleSet : public Region
    {
    public:
        TriangleSet() = default;
        ~TriangleSet() = default;

        void setCollisionTriangles(const std::vector<const ConvexPolygon*>& triangles);
        void setCollisionTriangles(std::vector<const ConvexPolygon*>&& triangles);

        const std::vector<const ConvexPolygon*>& getCollisionTriangles() const;

    private:
        // Collidable triangles in this region
        std::vector<const ConvexPolygon*> mCollisionTriangles;
    };
}

class CollisionMap
{
public:
    CollisionMap() = delete;
    CollisionMap(const char* pathToSVG, int SVG_Width, int SVG_Height);
    ~CollisionMap() = default;

    // Get map collisions
    Vec resolveCollisions(const ConvexPolygon& entity) const;
    
    // Checks if a point is inside a collidable map area
    bool isPointColliding(const Vec& point) const;

    const std::vector<Polygon>& getPolygons() const;

private:
    // A list of all of the polygons on the map
    std::vector<Polygon> mMapPolygons;
    
    // A list of all of the triangles on the map
    std::vector<ConvexPolygon> mMapTriangles;
    
    // A grid of the map, where each region stores the triangles that touch it
    Grid<MapInternals::TriangleSet> mTriangleGrid;
};


