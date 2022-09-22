#pragma once
#include "map.h"
#include "vec.h"

#include <SDL.h>

#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <functional>


// Node's used by the route graph
struct NodeInfo
{
    NodeInfo();

    // Zero's the g-cost, h-cost, and parent
    void reset();

    // Returns the sum of g-cost and h-cost
    int getFcost();

    // Distance from the start
    int mGcost;

    // Distance from the end
    int mHcost;

    // The predecessor node
    MapInternals::Tile* mParent;

    // A list of all of the accessible route nodes from this node
    std::vector<MapInternals::Tile*> mNeighbours;
};

class Pathfinder: public Map
{
public:
    Pathfinder() = delete;
    // Can only be created inside Game::
    Pathfinder(SDL_Renderer* defaultRenderer);
    ~Pathfinder();

    // Creates a sequence of tile's connecting 2 points
    bool findPath(const Vec& start, const Vec& end, std::stack<SDL_Point>& path);

private:
    // Check if 2 points are accessible to each other in a straight path
    bool isAccessible(const MapInternals::Tile* start, const MapInternals::Tile* dest);
    bool isAccessible(const SDL_FRect& entity, const MapInternals::Tile* destTile);
    bool isAccessible(const SDL_FRect& startEntity, const SDL_FRect& destEntity);
    bool isAccessible(const Vec& startPoint, const Vec& endPoint);

    // Build pathfinding graphs
    void buildRouteGraph();
    void connectRouteGraph();
    void buildRampGraph();
    void connectRampGraph();

    float getDistance(const Vec& startPoint, const MapInternals::Tile* endTile) const;

    // Returns the manhattan distance between 2 tiles
    int getDistance(const MapInternals::Tile* startTile, const MapInternals::Tile* endTile) const;

    // Used by A* to keep the open set (heap) sorted
    // Determines which node has the better potential to find the destination node
    bool higherPotential(MapInternals::Tile* op2, MapInternals::Tile* op1);
    // Used to sort the list of neighbors of a ramp node by proximity
    bool closerNode(MapInternals::Tile* startTile, MapInternals::Tile* op2, MapInternals::Tile* op1);

    // Uses a ramp node to start pathfinding
    void useRamp(std::vector<MapInternals::Tile*>& openSet, std::unordered_set<MapInternals::Tile*>& closedSet, MapInternals::Tile* firstTile, MapInternals::Tile* lastTile);

    // Backtracks a path in the route graph and converts in into a vector of points
    void createPath(MapInternals::Tile* end, std::stack<SDL_Point>& path) const;

    std::function<bool(MapInternals::Tile*, MapInternals::Tile*)> heapComp;

    // Holds a neighbour list of accessible route nodes for every tile on the map (except for walls and the route nodes themselves)
    std::unordered_map<MapInternals::Tile*, std::vector<MapInternals::Tile*>> mRampGraph;

    // Stores the map's important tiles that allow for navigation of the entire map
    std::unordered_map<MapInternals::Tile*, NodeInfo> mRouteGraph;
};

