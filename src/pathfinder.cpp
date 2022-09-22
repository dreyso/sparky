#include "../header/pathfinder.h"
#include "../header/map.h"
#include "../header/vec.h"

#include <SDL.h>

#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include <functional>

#define DIAGONAL_DISTANCE 14.f
#define ACROSS_DISTANCE 10.f
#define RAYCAST_WIDTH 40.f
#define RAYCAST_LENGTH 10.f


using namespace MapInternals;

NodeInfo::NodeInfo() { reset(); }

void NodeInfo::reset()
{
    mHcost = 0;
    mGcost = 0;
    mParent = nullptr;
}

int NodeInfo::getFcost() { return mGcost + mHcost; }

Pathfinder::Pathfinder(SDL_Renderer* defaultRenderer) : Map{ defaultRenderer }
{
    heapComp = [this](Tile* op2, Tile* op1) { return higherPotential(op2, op1); };

    // Build pathfinding graphs
    buildRouteGraph();
    connectRouteGraph();
    buildRampGraph();
    connectRampGraph();
}

Pathfinder::~Pathfinder() {}

bool Pathfinder::isAccessible(const Tile* startTile, const Tile* destTile)
{
    Vec startPoint;
    startPoint.setX(static_cast<float>(startTile->getCollisionBox()->x) + static_cast<float>(TILE_SIDE_LENGTH) / 2.f);
    startPoint.setY(static_cast<float>(startTile->getCollisionBox()->y) + static_cast<float>(TILE_SIDE_LENGTH) / 2.f);

    Vec destPoint;
    destPoint.setX(static_cast<float>(destTile->getCollisionBox()->x) + static_cast<float>(TILE_SIDE_LENGTH) / 2.f);
    destPoint.setY(static_cast<float>(destTile->getCollisionBox()->y) + static_cast<float>(TILE_SIDE_LENGTH) / 2.f);

    return isAccessible(startPoint, destPoint);
}

bool Pathfinder::isAccessible(const SDL_FRect& entity, const Tile* destTile)
{
    Vec startPoint;
    startPoint.setX(entity.x + entity.w / 2.f);
    startPoint.setY(entity.y + entity.h / 2.f);

    Vec destPoint;
    destPoint.setX(static_cast<float>(destTile->getCollisionBox()->x) + static_cast<float>(TILE_SIDE_LENGTH) / 2.f);
    destPoint.setY(static_cast<float>(destTile->getCollisionBox()->y) + static_cast<float>(TILE_SIDE_LENGTH) / 2.f);

    return isAccessible(startPoint, destPoint);
}

bool Pathfinder::isAccessible(const SDL_FRect& startEntity, const SDL_FRect& destEntity)
{
    Vec startPoint;
    startPoint.setX(startEntity.x + startEntity.w / 2.f);
    startPoint.setY(startEntity.y + startEntity.h / 2.f);

    Vec destPoint;
    destPoint.setX(destEntity.x + destEntity.w / 2.f);
    destPoint.setY(destEntity.y + destEntity.h / 2.f);

    return isAccessible(startPoint, destPoint);
}

bool Pathfinder::isAccessible(const Vec& startPoint, const Vec& destPoint)
{
    // Cast a ray between 2 points and check for wall tiles along the ray

    // Get vector between start and dest
    Vec orthagonal{ destPoint - startPoint };

    // Make it orthagonal to itself
    orthagonal *= Matrix{ 0, -1, 1, 0 };

    // Normalize it
    orthagonal.normalize();

    // Create 2 parallel lines to the initial start-dest line (using the perpendicular unit vector)
    // StartA ----- DestA
    // Start -------- Dest
    // StartB ----- DestB
    // RAYCAST_WIDTH is the distance between the initial line and A or B line
    Vec shiftedStartPointA = startPoint;
    shiftedStartPointA += orthagonal * (RAYCAST_WIDTH / 2.f);

    Vec shiftedDestPointA = destPoint;
    shiftedDestPointA += orthagonal * (RAYCAST_WIDTH / 2.f);

    Vec shiftedStartPointB = startPoint;
    shiftedStartPointB -= orthagonal * (RAYCAST_WIDTH / 2.f);

    Vec shiftedDestPointB = destPoint;
    shiftedDestPointB -= orthagonal * (RAYCAST_WIDTH / 2.f);

    // Get current vectors, will be gradually increased from start point to dest point
    Vec currentA{ shiftedStartPointA };
    Vec currentB{ shiftedStartPointB };

    // Unit vector to move both current A and B vectors 
    Vec currentUnitVec{ destPoint - startPoint };
    currentUnitVec.normalize();

    // The tiles that the current vectors fall on
    Tile* currentTileA = getTileFromWorldPoint(currentA);
    Tile* currentTileB = getTileFromWorldPoint(currentB);

    while (currentTileA->getType() != WALL_TILE && currentTileB->getType() != WALL_TILE)
    {
        // Reached destination, success (if A finished, then so did B)
        if ((shiftedDestPointA - currentA).getMagnitude() <= 10.f)
            return true;

        // Move towards destination at constant rate
        currentA += currentUnitVec * RAYCAST_LENGTH;
        currentB += currentUnitVec * RAYCAST_LENGTH;

        currentTileA = getTileFromWorldPoint(currentA);
        currentTileB = getTileFromWorldPoint(currentB);
    }
    return false;
}

void Pathfinder::buildRouteGraph()
{
    // Graph nodes are tiles that are diagonally adjacent to outside corners (of walls)

    // Search entire tilemap and add all valid tiles to the graph
    for (int iRow = 0; iRow < MAP_HEIGHT_IN_TILES; ++iRow)
    {
        for (int iCol = 0; iCol < MAP_WIDTH_IN_TILES; ++iCol)
        {
            Tile* tile = &mTiles[iRow][iCol];

            // Skip wall tiles
            if (tile->getType() == WALL_TILE)
                continue;

            bool isNode = false;

            // Determine search bounds
            int firstRow = iRow - 1;
            if (firstRow < 0) firstRow = 0;

            int lastRow = iRow + 1;
            if (lastRow >= MAP_HEIGHT_IN_TILES) lastRow = MAP_HEIGHT_IN_TILES - 1;

            int firstCol = iCol - 1;
            if (firstCol < 0) firstCol = 0;

            int lastCol = iCol + 1;
            if (lastCol >= MAP_WIDTH_IN_TILES) lastCol = MAP_WIDTH_IN_TILES - 1;

            // Check for isolated diagonal adjacencies
            // Upper left
            if (firstRow < iRow && firstCol < iCol && (mTiles[firstRow][firstCol]).getType() == WALL_TILE && (mTiles[iRow - 1][iCol]).getType() != WALL_TILE && (mTiles[iRow][iCol - 1]).getType() != WALL_TILE)
                isNode = true;
            // Upper right
            else if (firstRow < iRow && lastCol > iCol && (mTiles[firstRow][lastCol]).getType() == WALL_TILE && (mTiles[iRow - 1][iCol]).getType() != WALL_TILE && (mTiles[iRow][iCol + 1]).getType() != WALL_TILE)
                isNode = true;
            // Lower right
            else if (lastRow > iRow && lastCol > iCol && (mTiles[lastRow][lastCol]).getType() == WALL_TILE && (mTiles[iRow + 1][iCol]).getType() != WALL_TILE && (mTiles[iRow][iCol + 1]).getType() != WALL_TILE)
                isNode = true;
            // Lower left
            else if (lastRow > iRow && firstCol < iCol && (mTiles[lastRow][firstCol]).getType() == WALL_TILE && (mTiles[iRow + 1][iCol]).getType() != WALL_TILE && (mTiles[iRow][iCol - 1]).getType() != WALL_TILE)
                isNode = true;

            if (isNode == true)
            {
                NodeInfo temp;
                mRouteGraph.insert({ tile, temp });
            }
        }
    }
}

void Pathfinder::connectRouteGraph()
{
    // Search entire tilemap and add all valid tiles to the graph
    for (auto& iRow : mTiles)
    {   
        for (auto& iTile : iRow)
        {
            Tile* startTile = &iTile;
            // If the current tile is a node, match it to every other node
            if (mRouteGraph.count(startTile) != 1)
                continue;
        
            for (auto& jRow : mTiles)
            {   
                for (auto& jTile : jRow)
                {
                    Tile* destTile = &jTile;
                    // Avoid relating the current node to itself
                    if (destTile == startTile)
                        continue;

                    // If dest tile is a node
                    if (mRouteGraph.count(destTile) == 1)
                    {
                        // If dest is accessible from start, add it as a neighbor
                        if (isAccessible(startTile, destTile))
                            mRouteGraph.at(startTile).mNeighbours.push_back(destTile);
                    }
                }
            }
        }
    }
}

void Pathfinder::buildRampGraph()
{
    // Ramp graph nodes are all the tiles that aren't graph nodes or wall tiles
    // Search entire tilemap and add all valid tiles to the graph
    for (auto& iRow : mTiles)
    {
        for (auto& iTile : iRow)
        {

            Tile* currentTile = &iTile;

            // If the current tile is not a wall or a node, it is a ramp node
            if (currentTile->getType() != WALL_TILE && mRouteGraph.count(currentTile) == 0)
            {
                std::vector<Tile*> temp;
                mRampGraph.insert({ currentTile, temp });
            }
        }
    }
}

void Pathfinder::connectRampGraph()
{
    // Search entire tilemap and add all valid tiles to the graph
    for (auto& iRow : mTiles)
    {
        for (auto& iTile : iRow)
        {
            Tile* startTile = &iTile;

            // If the current tile is a ramp node, match it to every other node
            if (mRampGraph.count(startTile) == 1)
            {
                for (auto& jRow : mTiles)
                {
                    for (auto& jTile : jRow)
                    {
                        Tile* destTile = &jTile;

                        // Avoid relating the current node to itself
                        if (destTile == startTile)
                            continue;

                        // If jTile is a node
                        if (mRouteGraph.count(destTile) == 1)
                        {
                            // If dest is accessible from start, add it as a neighbor
                            if (isAccessible(startTile, destTile))
                                mRampGraph.at(startTile).push_back(destTile);

                            // Sort by closest neighbors
                            auto comp = [this, startTile](Tile* op1, Tile* op2) { return closerNode(startTile, op1, op2); };
                            sort(mRampGraph.at(startTile).begin(), mRampGraph.at(startTile).end(), comp);
                        }
                    }
                }
            }
        }
    }
}

int Pathfinder::getDistance(const Tile* startTile, const Tile* endTile) const
{
    const SDL_Rect* start = startTile->getCollisionBox();
    const SDL_Rect* end = endTile->getCollisionBox();

    int deltaX = abs(start->x - end->x) / TILE_SIDE_LENGTH;
    int deltaY = abs(start->y - end->y) / TILE_SIDE_LENGTH;

    if (deltaX > deltaY)
        return (static_cast<int>(DIAGONAL_DISTANCE) * deltaY) + (static_cast<int>(ACROSS_DISTANCE) * (deltaX - deltaY));
    else
        return (static_cast<int>(DIAGONAL_DISTANCE) * deltaX) + (static_cast<int>(ACROSS_DISTANCE) * (deltaY - deltaX));
}

float Pathfinder::getDistance(const Vec& startPoint, const Tile* endTile) const
{
    const SDL_Rect* end = endTile->getCollisionBox();

    // Find distance on both axes (add half of a tile to center point on tile)
    Vec endPoint{ static_cast<float>(end->x + TILE_SIDE_LENGTH) / 2.f, static_cast<float>(end->y + TILE_SIDE_LENGTH) / 2.f };

    // Return euclidean distance
    return (endPoint - startPoint).getMagnitude();
}

// Backtracks the route graph and create a list of points representing the path
void Pathfinder::createPath(Tile* end, std::stack<SDL_Point>& path) const
{
    Tile* currentTile = end;
    SDL_Point currentPoint;

    // Trace back untill a ramp node is found or the parent is null, exclude the first node
    while (mRampGraph.count(currentTile) != 1 && mRouteGraph.at(currentTile).mParent != nullptr)
    {
        currentPoint.x = currentTile->mCollisionBox.x + TILE_SIDE_LENGTH / 2;
        currentPoint.y = currentTile->mCollisionBox.y + TILE_SIDE_LENGTH / 2;
        path.push(currentPoint);
        currentTile = mRouteGraph.at(currentTile).mParent;
    }
}

bool Pathfinder::closerNode(Tile* startTile, Tile* op1, Tile* op2)
{
    int op1Distance = getDistance(startTile, op1);
    int op2Distance = getDistance(startTile, op2);

    if (op1Distance < op2Distance)
        return true;
    
    return false;
}

// Arguments are reversed for heap to store in ascending order
bool Pathfinder::higherPotential(Tile* op2, Tile* op1)
{
    if (mRouteGraph.at(op1).getFcost() < mRouteGraph.at(op2).getFcost())
        return true;
    else if (mRouteGraph.at(op1).getFcost() == mRouteGraph.at(op2).getFcost() && mRouteGraph.at(op1).mHcost < mRouteGraph.at(op2).mHcost)
        return true;
    else
        return false;

    // Greedy A*
   /*if (mRouteGraph.at(op1).mHcost < mRouteGraph.at(op2).mHcost)
       return true;
   else
       return false;*/
}

bool Pathfinder::findPath(const Vec& startPoint, const Vec& dest, std::stack<SDL_Point>& path)
{
    // A* requires a start and destination point
    Tile* firstTile = getTileFromWorldPoint(startPoint);    // The first tile on the path
    Tile* lastTile = getTileFromWorldPoint(dest);  // The last tile on the path

    // Can't pathfind outside of map bounds
    if (firstTile == nullptr || lastTile == nullptr)
        return false;

    // All tiles used to create the path must be route nodes
    // If the end points aren't route nodes, then they must be ramp nodes (otherwise the point is on a wall tile or out of the map)
    // Ramp nodes contain all accessible route nodes

    // Modify the last tile's to be it's nearest route node (sorted list)
    if (mRampGraph.count(lastTile) == 1)
        lastTile = mRampGraph.at(lastTile)[0];

    // Open set
    std::vector<Tile*> openSet;
    openSet.reserve(30);    // 30 is a tested number
    make_heap(openSet.begin(), openSet.end(), heapComp);

    // Closed set
    std::unordered_set<Tile*> closedSet;

    // If the first tile is a ramp node, add its neighbours to the open set
    if (mRampGraph.count(firstTile) == 1)
        useRamp(openSet, closedSet, firstTile, lastTile);
    else
    {
        // Start a new path
        mRouteGraph.at(firstTile).reset();

        // Add the starting node to the open set
        openSet.push_back(firstTile);
        push_heap(openSet.begin(), openSet.end(), heapComp);
    }

    while (openSet.size() > 0)
    {
        Tile* currentTile = openSet[0];
        // Remove the newly explored tile from the open set   
        pop_heap(openSet.begin(), openSet.end(), heapComp);
        openSet.pop_back();

        // Add it to the closed set
        closedSet.insert(currentTile);

        // If the current tile is the destination
        if (currentTile == lastTile)
        {
            // Backtrack the linked list and create a list of points
            createPath(lastTile, path);
            return true;
        }
        /*Heap test
        for (int i = 0; i < 5 && i < openSet.size(); ++i)
            // printf("|%d| ", openSet[i]->getFcost());
        printf("\n");*/

        for (auto neighbour : mRouteGraph.at(currentTile).mNeighbours)
        {
            // If neighbour is already closed, skip
            if (closedSet.find(neighbour) != closedSet.end())
                continue;

            int newCostToNeighbour = mRouteGraph.at(currentTile).mGcost + getDistance(currentTile, neighbour);
            bool neighbourIsOpen = find(openSet.begin(), openSet.end(), neighbour) != openSet.end();
            // If the neighbour is not in the open set or the new cost is cheaper
            if (!neighbourIsOpen || newCostToNeighbour < mRouteGraph.at(neighbour).mGcost)
            {
                mRouteGraph.at(neighbour).mGcost = newCostToNeighbour;
                mRouteGraph.at(neighbour).mHcost = getDistance(neighbour, lastTile);
                mRouteGraph.at(neighbour).mParent = currentTile;
                if (!neighbourIsOpen)
                {
                    openSet.push_back(neighbour);
                    push_heap(openSet.begin(), openSet.end(), heapComp);
                }
            }
        }
    }
    return false;
}

void Pathfinder::useRamp(std::vector<Tile*>& openSet, std::unordered_set<Tile*>& closedSet, Tile* firstTile, Tile* lastTile)
{
    // Add the first tile to the closed set
    closedSet.insert(firstTile);

    // Add the first tile's neighbours to the open set
    for (auto neighbour : mRampGraph.at(firstTile))
    {
        mRouteGraph.at(neighbour).mGcost = getDistance(firstTile, neighbour);
        mRouteGraph.at(neighbour).mHcost = getDistance(neighbour, lastTile);
        mRouteGraph.at(neighbour).mParent = firstTile;

        openSet.push_back(neighbour);
        push_heap(openSet.begin(), openSet.end(), heapComp);
    }
}