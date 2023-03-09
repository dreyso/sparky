#include "../header/search_graph.h"
#include "../header/collision_map.h"
#include "../header/vec.h"
#include "../header/polygon.h"

#include <SDL.h>

#include <vector>
#include <unordered_set>
#include <stack>
#include <algorithm>
#include <cmath>
#include <cassert>

#define NDEBUG

#define RAYCAST_WIDTH 1.f
#define RAYCAST_LENGTH 40.f

#define VERTICE_OFFSET 20.f
#define NODE_BUFFER 10
#define OPEN_SET_BUFFER 30
//#define GREEDY_ASTAR

using namespace GraphInternals;

Node::Node() { reset(); }

void Node::reset()
{
    mHcost = 0.f;
    mGcost = 0.f;
    mParent = nullptr;
}

float Node::getFcost() const { return mGcost + mHcost; }

SearchGraph::SearchGraph(const CollisionMap& collisionMap) : mCollisionMap{ collisionMap }
{
    // Build pathfinding graphs
    buildGraph();
    connectGraph();
}

SearchGraph::~SearchGraph() {}

bool SearchGraph::isAccessible(const Vec& startPoint, const Vec& destPoint)
{
    // Cast a ray between 2 points and check for wall tiles along the ray

    // Get vector between start and dest
    Vec orthagonal{ destPoint - startPoint };

    // If starting location is at the destination
    if (orthagonal.isZeroVector())
        return true;

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

    // Increment vectors towards their destinations while checking for collisions
    while (!mCollisionMap.isPointColliding(currentA) && !mCollisionMap.isPointColliding(currentB))
    {
        // Reached destination, success (if A finished, then so did B)
        if ((shiftedDestPointA - currentA).getMagnitude() <= RAYCAST_LENGTH)
            return true;

        // Move towards destination at constant rate
        currentA += currentUnitVec * RAYCAST_LENGTH;
        currentB += currentUnitVec * RAYCAST_LENGTH;
    }
    return false;
}

std::vector<Node*> SearchGraph::findNearestNodes(const Vec& point, int minCount)
{
    std::vector<Node*> nearestNodes;
    nearestNodes.reserve(10);

    for (int i = 0; i < mNodes.size(); ++i)
    {
        if (isAccessible(point, mNodes[i].mPoint))
            nearestNodes.push_back(&mNodes[i]);
    }
    return nearestNodes;
}

void SearchGraph::buildGraph()
{
    // Iterate over the polygons in the collision map
    for (auto& iPolygon : mCollisionMap.getPolygons())
    {
        // Expand each shape
        Polygon temp{ offsetVerticesBy(iPolygon.getVertices(), VERTICE_OFFSET) };

        // Add the new shifted vertices into the graph as nodes (if they're not colliding)
        for (auto& iVertice : temp.getVertices())
        {
            if (!mCollisionMap.isPointColliding(iVertice))
                mNodes.push_back(Node{ iVertice });
        }
    }
}

void SearchGraph::connectGraph()
{
    // Iterate over each unique pair of nodes
    for (int iNode = 0; iNode < mNodes.size(); ++iNode)
    {
        for (int jNode = iNode + 1; jNode < mNodes.size(); ++jNode)
        {
            // If there a path between the nodes, "neighbor" them
            if (isAccessible(mNodes[iNode].mPoint, mNodes[jNode].mPoint))
            {
                mNodes[iNode].mNeighbours.reserve(3);
                mNodes[iNode].mNeighbours.push_back(&mNodes[jNode]);
                mNodes[jNode].mNeighbours.push_back(&mNodes[iNode]);
            }
        }
    }
}

float SearchGraph::getDistance(const Vec& startPoint, const Vec& endPoint) const
{
    // Return euclidean distance
    return (endPoint - startPoint).getMagnitude();
}

float SearchGraph::getSquaredDistance(const Vec& startPoint, const Vec& endPoint) const
{
    // Return sqaured euclidean distance
    Vec temp = (endPoint - startPoint);
    return temp * temp;
}

// Backtracks the route graph and create a list of points representing the path
void SearchGraph::createPath(Node* end, std::stack<Vec>& path) const
{
    const Node* currentNode = end;

    // Trace back untill a node without a parent is reached
    while (currentNode->mParent != nullptr)
    {
        // Push the location of the node on the stack
        path.push(currentNode->mPoint);

        // Continue traversing the list
        currentNode = currentNode->mParent;
    }
}

// Arguments are reversed for heap to store in ascending order
bool SearchGraph::heuristic(const Node* op2, const Node* op1)
{
#ifndef GREEDY_ASTAR

    if (op1->getFcost() < op2->getFcost())
        return true;
    else if (op1->getFcost() == op2->getFcost() && op1->mHcost < op2->mHcost)
        return true;
    else
        return false;
#else
    // Greedy A*
   if (op1->mHcost < op2->mHcost)
       return true;
   else
       return false;
#endif //GREEDY_ASTAR
}

bool SearchGraph::findPath(const Vec& start, const Vec& dest, std::stack<Vec>& path)
{
    if (isAccessible(start, dest))
    {
        path.push(dest);
        return true;
    }

    // -- Create temporary, directed nodes -----------------------------------------
  
    // Convert starting point into a temporary node
    Node firstNode{ start };
    firstNode.mNeighbours = findNearestNodes(start, NODE_BUFFER);

    // Convert dest point into a temporary node
    Node lastNode{ dest };
    auto destNeighbours = findNearestNodes(dest, NODE_BUFFER);
    for (auto iNeighbour : destNeighbours)
        iNeighbour->mNeighbours.push_back(&lastNode);
    
    // -- Initialize sets -----------------------------------------

    // Open set
    std::vector<Node*> openSet;
    openSet.reserve(OPEN_SET_BUFFER);
    make_heap(openSet.begin(), openSet.end(), heuristic);

    // Closed set
    std::unordered_set<Node*> closedSet;

    // Add the starting node to the open set
    openSet.push_back(&firstNode);
    push_heap(openSet.begin(), openSet.end(), heuristic);

    // -- Search -----------------------------------------

    while (openSet.size() > 0)
    {
        // Grab next best node according to the heuristic
        Node* currentNode = openSet[0];

        // Remove the newly explored tile from the open set   
        pop_heap(openSet.begin(), openSet.end(), heuristic);
        openSet.pop_back();

        // Add it to the closed set
        closedSet.insert(currentNode);

        // If the current tile is the destination
        if (currentNode == &lastNode)
        {
            // Push the actual dest point
            path.push(dest);

            // Backtrack the linked list and create a list of points
            createPath(&lastNode, path);

            // Undo changes to temporary dest node's neighbors
            for (auto iNeighbour : destNeighbours)
                iNeighbour->mNeighbours.pop_back();

            return true;
        }

#ifndef NDEBUG

        // Heap test
        for (int i = 0; i < 5 && i < openSet.size(); ++i)
            printf("|%f| ", openSet[i]->getFcost());
        printf("\n");
#endif //NDEBUG

        for (auto neighbour : currentNode->mNeighbours)
        {
            // If neighbour is already closed, skip it
            if (closedSet.find(neighbour) != closedSet.end())
                continue;

            float newCostToNeighbour = currentNode->mGcost + getDistance(currentNode->mPoint, neighbour->mPoint);
            bool neighbourIsOpen = find(openSet.begin(), openSet.end(), neighbour) != openSet.end();
            // If the neighbour is not in the open set or the new cost is cheaper
            if (!neighbourIsOpen || newCostToNeighbour < neighbour->mGcost)
            {
                neighbour->mGcost = newCostToNeighbour;
                neighbour->mHcost = getDistance(neighbour->mPoint, lastNode.mPoint);
                neighbour->mParent = currentNode;
                if (!neighbourIsOpen)
                {
                    openSet.push_back(neighbour);
                    push_heap(openSet.begin(), openSet.end(), heuristic);
                }
            }
        }
    }

    // This line should never be reached
    assert(false);

    // Undo changes to temporary dest node's neighbors
    for (auto iNeighbour : destNeighbours)
        iNeighbour->mNeighbours.pop_back();

    return false;
}
