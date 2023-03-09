#include "../header/collision_map.h"
#include "../header/screen_size.h"
#include "../header/vec.h"
#include "../header/polygon.h"
#include "../header/grid.h"

#include <vector>
#include <cassert>


static const int GRID_SCALE = 150;
#define IS_SCALEABLE(width, height) ((width + height) % GRID_SCALE == 0)

using namespace MapInternals;

void TriangleSet::setCollisionTriangles(const std::vector<const ConvexPolygon*>& triangles)
{
    // Set the collidable triangles in this region
    mCollisionTriangles = triangles;
}

void TriangleSet::setCollisionTriangles(std::vector<const ConvexPolygon*>&& triangles)
{
    // Set the collidable triangles in this region
    mCollisionTriangles = std::move(triangles);
}

const std::vector<const ConvexPolygon*>& TriangleSet::getCollisionTriangles() const
{
    return mCollisionTriangles;
}

CollisionMap::CollisionMap(const char* pathToSVG, int SVG_Width, int SVG_Height)
    : mTriangleGrid{ SVG_Width / GRID_SCALE, SVG_Height / GRID_SCALE, GRID_SCALE }
{  
    // Check that the dimensions of the map and regions are compatible
    assert(IS_SCALEABLE(SVG_Width, SVG_Height));
    
    // -- Read and triangulate polygons -----------------------------------------

    // Read the polygons in the svg file
    mMapPolygons = Polygon::readSvgPolygons(pathToSVG);

    // Get each polygon's triangles, reserve the minimum amount first
    mMapTriangles.reserve(mMapPolygons.size());
    for (int i = 0; i < mMapPolygons.size(); ++i)
    {
        auto triangles = mMapPolygons[i].triangulate();
        mMapTriangles.insert(mMapTriangles.end(), triangles.begin(), triangles.end());
    }
   
    // -- Create collision map -----------------------------------------

    // Set the regions
    for (int iRow = 0; iRow < mTriangleGrid.getRows(); ++iRow)
    {
        for (int iCol = 0; iCol < mTriangleGrid.getCols(); ++iCol)
        {
            std::vector<const ConvexPolygon*> collidedTriangles;
            
            // Find all triangles that lie in this region
            for (auto& triangle : mMapTriangles)
            {
                auto box = ConvexPolygon::rectToPolygon(mTriangleGrid[iRow][iCol].getCollisionBox());
                if (!ConvexPolygon::resolveCollision(box, triangle).isZeroVector())
                    collidedTriangles.push_back(&triangle);
            }
            // Add all colliding triangles to the region
            mTriangleGrid[iRow][iCol].setCollisionTriangles(collidedTriangles);
        }
    }
}

// Get map collisions
Vec CollisionMap::resolveCollisions(const ConvexPolygon& entity) const
{
    auto& entityAABB = entity.getAABB();
    Vec solution{ 0.f, 0.f };

    auto intersectedRegions = mTriangleGrid.getRegionsIntersectingRect(entity.getAABB());

    // Iterate over every region touched by the entity's AABB
    for (auto& region : intersectedRegions)
    {
        auto& triangles = region->getCollisionTriangles();
        for (auto& triangle : triangles)
        {
            // Add current collision solution to the overall
            ConvexPolygon::mergeResolution(solution, ConvexPolygon::resolveCollision(entity, *triangle));
        }
    }
    return solution;
}

// Checks if a point is inside a collidable map area
bool CollisionMap::isPointColliding(const Vec& point) const
{
    // Point must be within map bounds to collide
    if (!mTriangleGrid.isPointOnGrid(point))
        return false;

    // Get the region the point is in
    auto regions = mTriangleGrid.getRegionsUnderPoint(point);

    // Iterate over the regions under the point
    for (auto& iRegion : regions)
    {
        // Check if the point falls into any triangles in the region
        for (auto& triangle : iRegion->getCollisionTriangles())
        {
            // Return true if the point falls inside a collision triangle
            if (triangle->containsPoint(point))
                return true;
        }
    }
    return false;
}

const std::vector<Polygon>& CollisionMap::getPolygons() const
{
    return mMapPolygons;
}
