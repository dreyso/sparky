#include "../header/map.h"
#include "../header/texture.h"
#include "../header/collision.h"
#include "../header/screen_size.h"
#include "../header/vec.h"
#include "../header/polygon.h"


#include <SDL.h>

#include <fstream>
#include <new>
#include <vector>
#include <string>
#include <stdexcept>


using namespace MapInternals;

void Region::setRegion(int x, int y, int regionSideLength)
{
    // Set the length of the region's sides
    mRegionSideLength = regionSideLength;

    // Set the region's collision box
    mCollisionBox.set(static_cast<float>(x), static_cast<float>(y), static_cast<float>(x + regionSideLength), static_cast<float>(y + regionSideLength));
}

void Region::setCollisionTriangles(const std::vector<const ConvexPolygon*>& triangles)
{
    // Set the collidable triangles in this region
    mCollisionTriangles = triangles;
}

void Region::setCollisionTriangles(std::vector<const ConvexPolygon*>&& triangles)
{
    // Set the collidable triangles in this region
    mCollisionTriangles = std::move(triangles);
}

// Get the collision box
const Rect& Region::getCollisionBox() const
{
    return mCollisionBox;
}

const std::vector<const ConvexPolygon*>& Region::getCollisionTriangles() const
{
    return mCollisionTriangles;
}

Map::Map(SDL_Renderer* defaultRenderer, const char* pathToSVG) : mMapTexture{ defaultRenderer, pathToSVG }
{  
    // -- Read and triangulate polygons -----------------------------------------

    // Read the polygons in the svg file
    mMapPolygons = Polygon::read_SVG_polygons(pathToSVG);

    // Get each polygon's triangles, reserve the minimum amount first
    mMapTriangles.reserve(mMapPolygons.size());
    for (int i = 0; i < mMapPolygons.size(); ++i)
    {
        auto triangles = mMapPolygons[i].triangulate();
        mMapTriangles.insert(mMapTriangles.end(), triangles.begin(), triangles.end());
    }
   
    // -- Set map dimension variables -----------------------------------------

    MAP_WIDTH = mMapTexture.getWidth();
    MAP_HEIGHT = mMapTexture.getHeight();
    REGION_SIDE_LENGTH = 100;

    // Check that the dimensions of the map and regions are compatible
    if(MAP_WIDTH % REGION_SIDE_LENGTH != 0 || MAP_HEIGHT % REGION_SIDE_LENGTH != 0)
        throw(std::invalid_argument{ "Error: Map is not divisible into regions\n" });

    MAP_WIDTH_IN_REGIONS = MAP_WIDTH / REGION_SIDE_LENGTH;
    MAP_HEIGHT_IN_REGIONS = MAP_HEIGHT / REGION_SIDE_LENGTH;
    TOTAL_REGIONS = MAP_WIDTH_IN_REGIONS * MAP_HEIGHT_IN_REGIONS;

    // -- Create collision map -----------------------------------------

    mRegions.assign(MAP_HEIGHT_IN_REGIONS, std::vector<Region>(MAP_WIDTH_IN_REGIONS));

    // Set the regions
    for (int iRow = 0; iRow < MAP_HEIGHT_IN_REGIONS; ++iRow)
    {
        for (int iCol = 0; iCol < MAP_WIDTH_IN_REGIONS; ++iCol)
        {
            mRegions[iRow][iCol].setRegion(iCol * REGION_SIDE_LENGTH, iRow * REGION_SIDE_LENGTH, REGION_SIDE_LENGTH);
            std::vector<const ConvexPolygon*> collidedTriangles;
            
            // Find all triangles that lie in this region
            for (auto& triangle : mMapTriangles)
            {
                auto box = ConvexPolygon::rectToPolygon(mRegions[iRow][iCol].getCollisionBox());
                if (!ConvexPolygon::resolveCollision(box, triangle).isZeroVector())
                    collidedTriangles.push_back(&triangle);
            }
            // Add all colliding triangles to the region
            mRegions[iRow][iCol].setCollisionTriangles(collidedTriangles);
        }
    }
}

// Get map collisions
Vec Map::resolveCollisions(const ConvexPolygon& entity)
{
    auto& entityAABB = entity.getAABB();
    Vec solution;

    // Determine regions that inetrsect with the entity's AABB
    int firstRow = static_cast<int>(entityAABB.y) / REGION_SIDE_LENGTH;
    if (firstRow < 0) firstRow = 0;

    int lastRow = (static_cast<int>(entityAABB.y) + SCREEN_HEIGHT) / REGION_SIDE_LENGTH;
    if (lastRow >= MAP_HEIGHT_IN_REGIONS) lastRow = MAP_HEIGHT_IN_REGIONS - 1;

    int firstCol = static_cast<int>(entityAABB.x) / REGION_SIDE_LENGTH;
    if (firstCol < 0) firstCol = 0;

    int lastCol = (static_cast<int>(entityAABB.x) + SCREEN_WIDTH) / REGION_SIDE_LENGTH;
    if (lastCol >= MAP_WIDTH_IN_REGIONS) lastCol = MAP_WIDTH_IN_REGIONS - 1;

    // Iterate over every region touched by the entity's AABB
    // Note: <= is used to render the last visable row or col
    for (int iRow = firstRow; iRow <= lastRow; ++iRow)
    {
        for (int iCol = firstCol; iCol <= lastCol; ++iCol)
        {
            auto& triangles = mRegions[iRow][iCol].getCollisionTriangles();
            for (auto& triangle : triangles)
            {
                // Add current collision solution to the overall
                ConvexPolygon::mergeResolution(solution, ConvexPolygon::resolveCollision(entity, *triangle));
            }
        }
    }
}

bool Map::isPointInWorldBounds(const Vec& point) const
{
    return (point.getX() < 0.f || point.getX() >= MAP_WIDTH || point.getY() < 0.f || point.getY() >= MAP_HEIGHT);
}

// Returns a region that a point lies on
MapInternals::Region& Map::getRegionFromWorldPoint(const Vec& point)
{
    // Check if point is inside map bounds
    if (isPointInWorldBounds(point))
        throw(std::invalid_argument{ "Error: Point is outside of map bounds\n" });

    // Convert pixel coordinates to tile in 2d array
    // E.g, 0.5 tiles is on the 0th tile
    int col = static_cast<int>(point.getX()) / REGION_SIDE_LENGTH;
    int row = static_cast<int>(point.getY()) / REGION_SIDE_LENGTH;
    return mRegions[row][col];
}

// Checks if a point is inside a wall
bool Map::isInWall(const Vec& pos)
{
    Tile* tileUnderPoint = getTileFromWorldPoint(pos);
    if (tileUnderPoint != nullptr && tileUnderPoint->getType() == WALL_TILE)
        return true;
    return false;
}

void Map::render(const SDL_FRect& camera)
{
    // Determine rendering bounds
    int firstRow = static_cast<int>(camera.y) / TILE_SIDE_LENGTH;
    if (firstRow < 0) firstRow = 0;
   
    int lastRow = (static_cast<int>(camera.y) + SCREEN_HEIGHT) / TILE_SIDE_LENGTH;
    if (lastRow >= MAP_HEIGHT_IN_TILES) lastRow = MAP_HEIGHT_IN_TILES - 1;
    
    int firstCol = static_cast<int>(camera.x) / TILE_SIDE_LENGTH;
    if (firstCol < 0) firstCol = 0;

    int lastCol = (static_cast<int>(camera.x) + SCREEN_WIDTH) / TILE_SIDE_LENGTH;
    if (lastCol >= MAP_WIDTH_IN_TILES) lastCol = MAP_WIDTH_IN_TILES - 1;

    // Render the level
    // Note: <= is used to render the last visable row or col
    for (int iRow = firstRow; iRow <= lastRow; ++iRow)
    {
        for (int iCol = firstCol; iCol <= lastCol; ++iCol)
        {
            Tile* tile = &mTiles[iRow][iCol];
            const SDL_Rect* tileCollisionBox = tile->getCollisionBox();

            int screenPosX = static_cast<int>(roundf(static_cast<float>(tileCollisionBox->x) - camera.x));
            int screenPosY = static_cast<int>(roundf(static_cast<float>(tileCollisionBox->y) - camera.y));
            mTileTextures.draw(screenPosX, screenPosY, &mTileSprites[tile->getType()]);
        }
    }
}