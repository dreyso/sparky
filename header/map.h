#pragma once
#include "texture.h"
#include "collision.h"
#include "vec.h"
#include "Polygon.h"

#include <SDL.h>

#include <memory>
#include <vector>
#include <fstream>


namespace MapInternals
{
    // A sqaure
    class Region
    {
    public:
        Region() = default;
        ~Region() = default;

        // Initialize position (upper-left corner) and size
        void setRegion(int x, int y, int regionSideLength);
        void setCollisionTriangles(const std::vector<const ConvexPolygon*>& triangles);
        void setCollisionTriangles(std::vector<const ConvexPolygon*>&& triangles);

        const Rect& getCollisionBox() const;
        const std::vector<const ConvexPolygon*>& getCollisionTriangles() const;


    private:
        int mRegionSideLength = 0.f;
        Rect mCollisionBox;
        
        // Collidable triangles in this region
        std::vector<const ConvexPolygon*> mCollisionTriangles;
    };
}

class Map
{
public:
    Map() = delete;
    Map(SDL_Renderer* defaultRenderer, const char* pathToSVG);
    ~Map() = default;

    // Get map collisions
    Vec resolveCollisions(const ConvexPolygon& entity);
    
    // Checks if a point is inside a collidable map area
    bool checkPointCollision(const Vec& point);

    // Render all of the tiles in the camera
    void draw(const SDL_FRect& pCamera);

protected:
    // Utility map variables
    static inline int REGION_SIDE_LENGTH = 100;
    static inline int TOTAL_REGIONS = 0;
    static inline int MAP_WIDTH_IN_REGIONS = 0;
    static inline int MAP_HEIGHT_IN_REGIONS = 0;
    static inline int MAP_WIDTH = 0.f;
    static inline int MAP_HEIGHT = 0.f;

    // Checks if a point in inside map bounds
    bool isPointInWorldBounds(const Vec& point) const;

    // Returns a region that a point lies on
    MapInternals::Region& getRegionFromWorldPoint(const Vec& point);

private:
    // SVG file loaded as an SDL texture
    Texture mMapTexture;
    
    // A list of all of the polygons on the map
    std::vector<Polygon> mMapPolygons;
    
    // A list of all of the triangles on the map
    std::vector<ConvexPolygon> mMapTriangles;
    
    // A grid of the map, where each region stores the triangles that touch it
    std::vector<std::vector<MapInternals::Region>> mRegions;
};

