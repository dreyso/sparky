#include "../header/map.h"
#include "../header/texture.h"
#include "../header/collision.h"
#include "../header/screen_size.h"
#include "../header/vec.h"

#include <SDL.h>

#include <fstream>
#include <new>
#include <vector>
#include <string>

#define MAX_COLLISIONS 3
#define MAX_NEIGHBOURS 8


using namespace MapInternals;

void Tile::setTile(int x, int y, int tileSideLength, tileType type)
{
    // Set tile position
    mCollisionBox.x = x;
    mCollisionBox.y = y;

    // Set tile dimsensions
    mCollisionBox.w = tileSideLength;
    mCollisionBox.h = tileSideLength;

    // Set the type
    mType = type;
}

tileType Tile::getType() const { return mType; }

// Get the collision box
const SDL_Rect* Tile::getCollisionBox() const { return &mCollisionBox; }

Map::Map(SDL_Renderer* defaultRenderer) : mTileTextures{defaultRenderer, "assets/images/tilemap.png"}
{  
    // Open the map
    std::ifstream tileMapFile{ "assets/tilemap.map" };

    // If the map couldn't be loaded
    if (tileMapFile.is_open() == false)
    {
        fprintf(stderr, "Unable to load map file!\n");
        exit(-1);
    }

    // Read in and store tilemap dimensions
    loadMapVariables(tileMapFile);

    // Create appropriately sized grid
    mTiles.assign(MAP_HEIGHT_IN_TILES, std::vector<Tile>(MAP_WIDTH_IN_TILES));

    // Set the tiles
    for (int iRow = 0; iRow < MAP_HEIGHT_IN_TILES; ++iRow)
    {
        for (int iCol = 0; iCol < MAP_WIDTH_IN_TILES; ++iCol)
        {
            int readInTileType = -1;

            // Read tile type from file
            tileMapFile >> readInTileType;

            // Check if read-in was successfull
            if (tileMapFile.good() == false)
            {
                fprintf(stderr, "Failed to load map: read-in unsuccesful\n");
                exit(-1);
            }

            // Invalid tile type
            if (readInTileType <= -1 && readInTileType >= TOTAL_TILE_TYPES)
            {
                fprintf(stderr, "Failed to load map: invalid tile type\n");
                exit(-1);
            }
            mTiles[iRow][iCol].setTile(iCol * TILE_SIDE_LENGTH, iRow * TILE_SIDE_LENGTH, TILE_SIDE_LENGTH, static_cast<tileType>(readInTileType));
        }
    }
    // Close file
    tileMapFile.close();
    
    loadSprites();
}

bool Map::loadMapVariables(std::ifstream& tileMapFile)
{
    // Get tilemap dimensions
    int tileScale = 0;
    int mapWidthInTiles = 0;
    int mapHeightInTiles = 0;

    // Quick way to check every read-in
    auto check = [&tileMapFile]() 
    {
        if (tileMapFile.good() == false)
        {
            fprintf(stderr, "Failed to load map: read-in unsuccesful\n");
            return false;
        }
        return true;
    };

    tileMapFile.ignore(LONG_MAX, ' ');    // skip everything up to and including space
    tileMapFile >> tileScale;
    if (check() == false)                 // Check if read-in was successful
        return false;
    tileMapFile.ignore(1, '\n');          // Move on to next line

    tileMapFile.ignore(LONG_MAX, ' ');
    tileMapFile >> mapWidthInTiles;
    if (check() == false)   
        return false;
    tileMapFile.ignore(1, '\n');

    tileMapFile.ignore(LONG_MAX, ' ');
    tileMapFile >> mapHeightInTiles;
    if (check() == false)
        return false;
    tileMapFile.ignore(1, '\n');

    // Set variables
    TILE_SIDE_LENGTH = tileScale;
    MAP_WIDTH_IN_TILES = mapWidthInTiles;
    MAP_HEIGHT_IN_TILES = mapHeightInTiles;
    TOTAL_TILES = mapWidthInTiles * mapHeightInTiles;
    MAP_WIDTH = static_cast<float>(mapWidthInTiles * tileScale);
    MAP_HEIGHT = static_cast<float>(mapHeightInTiles * tileScale);

    return true;
}

bool Map::loadSprites()
{
    // Create the sprite sheet
    mTileSprites[MISC_TILE].x = 0;
    mTileSprites[MISC_TILE].y = 0;
    mTileSprites[MISC_TILE].w = TILE_SIDE_LENGTH;
    mTileSprites[MISC_TILE].h = TILE_SIDE_LENGTH;

    mTileSprites[FLOOR_TILE].x = 100;
    mTileSprites[FLOOR_TILE].y = 0;
    mTileSprites[FLOOR_TILE].w = TILE_SIDE_LENGTH;
    mTileSprites[FLOOR_TILE].h = TILE_SIDE_LENGTH;

    mTileSprites[WALL_TILE].x = 200;
    mTileSprites[WALL_TILE].y = 0;
    mTileSprites[WALL_TILE].w = TILE_SIDE_LENGTH;
    mTileSprites[WALL_TILE].h = TILE_SIDE_LENGTH;

    return true;
}


std::vector<SDL_Rect> Map::mergeTiles(const std::vector<const SDL_Rect*>& collidedTiles) const
{
    std::vector<SDL_Rect> mergedCollisions;

    // Two adjacent tiles or empty corner case
    if (collidedTiles.size() == 2)
    {
        // Adjacent on the y axis (row)
        if (collidedTiles[0]->y == collidedTiles[1]->y)
            mergedCollisions.push_back(SDL_Rect{ collidedTiles[0]->x, collidedTiles[0]->y, 2 * collidedTiles[0]->w, collidedTiles[0]->h });
        
        // Adjacent on the x axis (column)
        else if (collidedTiles[0]->x == collidedTiles[1]->x)
            mergedCollisions.push_back(SDL_Rect{ collidedTiles[0]->x, collidedTiles[0]->y, collidedTiles[0]->w, 2 * collidedTiles[0]->h });
    }
    if (mergedCollisions.size() == 0)
    {
        for (auto& collidedTile : collidedTiles)
            mergedCollisions.push_back(*collidedTile);
    }
    return mergedCollisions;
}

// Check wall collisions
bool Map::checkWallCollisions(const SDL_FRect& box, Vec& adjustPos)
{
    // Grab all unique tiles that the entity is on
    // by checking if each corner of the hitbox is on a different tile

    std::vector<const SDL_Rect*> collidedTileBoxes;
    Vec entityBoxCorner{0,0};
    
    // Use a lambda to verify and add a corner that lies on a wall tile not already added to the list
    auto verifyAndAdd = [this, &collidedTileBoxes, &entityBoxCorner]()
    { 
        Tile* tempCollidedTile = getTileFromWorldPoint(entityBoxCorner);
        const SDL_Rect* tempCollidedBox;

        // Proceed only if it's a wall tile
        if (tempCollidedTile->getType() == WALL_TILE)
            tempCollidedBox = tempCollidedTile->getCollisionBox();
        else
            return;

        // Check if this collision box is already present before adding it to the list
        if (find(collidedTileBoxes.begin(), collidedTileBoxes.end(), tempCollidedBox) == collidedTileBoxes.end())
            collidedTileBoxes.push_back(tempCollidedBox);

        return; 
    };

    // Must be in row major order
    // 1st corner
    entityBoxCorner = Vec{box.x, box.y};
    verifyAndAdd();

    // 2nd corner
    entityBoxCorner += Vec{ box.w, 0.f };
    verifyAndAdd();

    // 3nd corner
    entityBoxCorner += Vec{ -box.w, box.h };
    verifyAndAdd();

    // 4th corner
    entityBoxCorner += Vec{ box.w, 0 };
    verifyAndAdd();

    // If any corner is in a wall tile, resolve the collision
    if (collidedTileBoxes.size() > 0)
    {
        // If possible, merge the collided tiles to all for sliding against walls
        buildCollisionReport(box, mergeTiles(collidedTileBoxes), adjustPos);
        return true;
    }
    else
        return false;
}

// Returns a tile that a passed in point lies on
Tile* Map::getTileFromWorldPoint(const Vec& point)
{
    // Check if point is inside map bounds
    if (point.getX() < 0.f || point.getX() >= MAP_WIDTH || point.getY() < 0.f || point.getY() >= MAP_HEIGHT)
    {
        fprintf(stderr, "Point is outside the map bounds\n");
        return nullptr;
    }

    // Convert pixel coordinates to tile in 2d array
    // E.g, 0.5 tiles is on the 0th tile
    int col = static_cast<int>(point.getX()) / TILE_SIDE_LENGTH;
    int row = static_cast<int>(point.getY()) / TILE_SIDE_LENGTH;
    return &mTiles[row][col];
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