#pragma once
#include "texture.h"
#include "collision.h"
#include "vec.h"

#include <SDL.h>

#include <memory>
#include <vector>


namespace MapInternals
{
    enum tileType : int { MISC_TILE, FLOOR_TILE, WALL_TILE, TOTAL_TILE_TYPES };

    class Tile
    {
    public:
        Tile() = default;
        ~Tile() = default;

        // Initialize position and type
        void setTile(int x, int y, int tileSideLength, tileType type);

        tileType getType() const;

        const SDL_Rect* getCollisionBox() const;

        // Tile dimensions and position
        SDL_Rect mCollisionBox = {0, 0, 0, 0};

    private:
        tileType mType = MISC_TILE;
    };
}

class Map
{
public:
    Map() = delete;
    // Can only be created inside Game::
    Map(SDL_Renderer* defaultRenderer);
    ~Map() = default;

    // Check wall collisions
    bool checkWallCollisions(const SDL_FRect& box, Vec& adjustPos);
    
    // Checks if a point is inside a wall
    bool isInWall(const Vec& point);

    // Render all of the tiles in the camera
    void render(const SDL_FRect& pCamera);

protected:
    // Initialized in class constructor
    static inline int TILE_SIDE_LENGTH = 0;
    static inline int TOTAL_TILES = 0;
    static inline int MAP_WIDTH_IN_TILES = 0;
    static inline int MAP_HEIGHT_IN_TILES = 0;
    static inline float MAP_WIDTH = 0.f;
    static inline float MAP_HEIGHT = 0.f;

    // Used to smoothen wall collisions, merges axis aligned tiles into a single collision box
    std::vector<SDL_Rect> mergeTiles(const std::vector<const SDL_Rect*>& collidedTiles) const;

    // Returns a tile that a point lies on
    MapInternals::Tile* getTileFromWorldPoint(const Vec& point);

    // Grid of all of the tiles
    std::vector<std::vector<MapInternals::Tile>> mTiles;

private:
    // Tile texture spritesheet
    Texture mTileTextures;
    SDL_Rect mTileSprites[3];

    bool loadSprites();
    bool loadMapVariables(std::ifstream& tileMapFile);
};

