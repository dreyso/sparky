#pragma once
#include "collision_map.h"
#include "vec.h"
#include "polygon.h"
#include "util.h"

#include <SDL.h>

#include <vector>
#include <stack>

namespace GraphInternals
{
    // Node's used by the route graph
    struct Node
    {
        Node();
        Node(const Vec& point) : mPoint{ point } {}

        // Zero's the g-cost, h-cost, and parent
        void reset();

        // Returns the sum of g-cost and h-cost
        float getFcost() const;

        // Distance from the start
        float mGcost = 0.f;

        // Distance from the end
        float mHcost = 0.f;

        // The coordinates of the node
        Vec mPoint{ 0.f, 0.f };

        // The predecessor node
        Node* mParent = nullptr;

        // A list of all of the accessible route nodes from this node
        std::vector<Node*> mNeighbours;
    };

    // A sqaure
    class NodeSet : public Region
    {
    public:
        NodeSet() = default;
        ~NodeSet() = default;

        void setNodes(const std::vector<Node*>& nodes);
        void setNodes(std::vector<Node*>&& nodes);
        const std::vector<Node*>& getNodes() const;

    private:
        // Accessible nodes in this region
        std::vector<Node*> mNodes;
    };

}

class SearchGraph
{
public:
    SearchGraph() = delete;
    SearchGraph(const CollisionMap& collisionMap, int SVG_Width, int SVG_Height);
    ~SearchGraph();

    // Creates a sequence of points connecting 2 locations
    bool findPath(const Vec& start, const Vec& end, std::stack<Vec>& path);

    // Recomputes the end of the path
    bool updatePath(const Vec& start, const Vec& end, std::stack<Vec>& path);

private:
    // Check if 2 points are accessible to each other in a straight path
    bool isAccessible(const Vec& startPoint, const Vec& endPoint);

    // Finds and returns the nearest, accessible nodes
    std::vector<GraphInternals::Node*> findNearestNodes(const Vec& point, int minCount);

    // Build graph and grid
    void buildGraph();
    void connectGraph();
    void assignGrid();

    float getDistance(const Vec& startPoint, const Vec& endPoint) const;
    float getSquaredDistance(const Vec& startPoint, const Vec& endPoint) const;


    // Used by A* to keep the open set (heap) sorted
    // Determines which node has the better potential to find the destination node
    static bool heuristic(const GraphInternals::Node* op2, const GraphInternals::Node* op1);

    // Backtracks a path in the route graph and converts in into a vector of points
    void createPath(GraphInternals::Node* end, std::stack<Vec>& path) const;

    const CollisionMap& mCollisionMap;

    // A list of all of the nodes in the graph
    std::vector<GraphInternals::Node> mNodes;

    // Partitions nodes into regions in a grid
    Grid<GraphInternals::NodeSet> mNodesGrid;
};

