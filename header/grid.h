#pragma once
#include "vec.h"
#include "polygon.h"
#include <stdexcept>

// A sqaure
class Region
{
public:
    Region() = default;
    ~Region() = default;

    // Initialize position (upper-left corner) and size
    void setRegion(int x, int y, int regionSideLength)
    {
        // Set the length of the region's sides
        mRegionSideLength = regionSideLength;

        // Set the region's collision box
        mCollisionBox.set(static_cast<float>(x), static_cast<float>(y), static_cast<float>(x + regionSideLength), static_cast<float>(y + regionSideLength));
    }

    // Checks if a point on the region, edges count
    bool isPointOnRegion(const Vec& point) const
    {
        float xEnd = mCollisionBox.x + mCollisionBox.w;
        float yEnd = mCollisionBox.y + mCollisionBox.h;
        return !(point.getX() <= mCollisionBox.x || point.getX() >= xEnd || point.getY() <= 0.f || point.getY() >= yEnd);
    }

    // Get the collision box
    const Rect& getCollisionBox() const
    {
        return mCollisionBox;
    }

private:
    int mRegionSideLength = 0;
    Rect mCollisionBox;
};

//template<typename T>
//using sizeT = typename std::vector<T>::size_type;

template <typename RegionType>
class Grid
{
public:
    //using sizeT = typename std::vector<RegionType>::size_type;

    Grid(int rows, int cols, int regionSideLength)
        : mRows{ rows }, mCols{ cols }, mRegionSideLength{ regionSideLength }
    {
        mGridHeight = mRows * mRegionSideLength;
        mGridWidth = mCols * mRegionSideLength;
        mTotalRegions = mRows * mCols;

        // Allocate the grid
        mRegions.assign(mRows, std::vector<RegionType>(mCols));

        // Set the regions
        for (int iRow = 0; iRow < mRows; ++iRow)
        {
            for (int iCol = 0; iCol < mCols; ++iCol)
            {
                mRegions[iRow][iCol].setRegion(iCol * mRegionSideLength, iRow * mRegionSideLength, mRegionSideLength);
            }
        }
    }
    ~Grid() = default;

    std::vector<RegionType>& operator[](int row)
    {
        if (row >= mRows)
            throw(std::runtime_error{ "Error: Out of bounds grid access\n" });

        return mRegions[row];
    }

    const std::vector<RegionType>& operator[](int row) const
    {
        if (row >= mRows)
            throw(std::runtime_error{ "Error: Out of bounds grid access\n" });

        return mRegions[row];
    }

    // Checks if a point on the grid, edges count
    bool isPointOnGrid(const Vec& point) const
    {
        return !(point.getX() <= 0.f || point.getX() >= static_cast<float>(mGridWidth) || point.getY() <= 0.f || point.getY() >= static_cast<float>(mGridHeight));
    }

    int getRows() const
    {
        return mRows;
    }

    int getCols() const
    {
        return mCols;
    }

    int getRegionSideLength() const
    {
        return mRegionSideLength;
    }

    // Returns the regions that a point lies on
    std::vector<const RegionType*> getRegionsUnderPoint(const Vec& point) const
    {
        // Check if point is inside grid bounds
        if (!isPointOnGrid(point))
            throw(std::invalid_argument{ "Error: Point is outside of map bounds\n" });

        std::vector<const RegionType*> regionsUnderPoint;

        // Convert pixel coordinates to region in 2d array
        // E.g, 0.5 regions is on the 0th region
        // Note that if the point lies on a boundary, it will end up being in the further x, y region
        int col = static_cast<int>(point.getX()) / mRegionSideLength;
        int row = static_cast<int>(point.getY()) / mRegionSideLength;

        regionsUnderPoint.push_back(&mRegions[row][col]);

        // Check if the point lies on the x-boundary
        if (roundf(point.getX()) == point.getX() && static_cast<int>(point.getX()) % mRegionSideLength == 0)
            regionsUnderPoint.push_back(&mRegions[row][col - 1]);

        // Check if the point lies on the y-boundary
        if (roundf(point.getY()) == point.getY() && static_cast<int>(point.getY()) % mRegionSideLength == 0)
            regionsUnderPoint.push_back(&mRegions[row - 1][col]);

        // If the point lies on both boundries, there is another region
        if (regionsUnderPoint.size() == 3)
            regionsUnderPoint.push_back(&mRegions[row - 1][col - 1]);

        return regionsUnderPoint;
    }

    // Returns the regions that intersect a rectangle
    std::vector<const RegionType*> getRegionsIntersectingRect(const Rect& AABB) const
    {
        // Determine regions that inetrsect with the entity's AABB
        int firstRow = static_cast<int>(AABB.y) / mRegionSideLength;
        if (firstRow < 0) firstRow = 0;

        int lastRow = static_cast<int>(AABB.y + AABB.h) / mRegionSideLength;
        if (lastRow >= mRows) lastRow = mRows - 1;

        int firstCol = static_cast<int>(AABB.x) / mRegionSideLength;
        if (firstCol < 0) firstCol = 0;

        int lastCol = static_cast<int>(AABB.x + AABB.w) / mRegionSideLength;
        if (lastCol >= mCols) lastCol = mCols - 1;

        std::vector<const RegionType*> intersectingRegions;
        intersectingRegions.reserve(static_cast<int>(roundf(AABB.w * AABB.h / powf(static_cast<float>(mRegionSideLength), 2.f))));

        // Iterate over every region touched by the AABB
        for (int iRow = firstRow; iRow <= lastRow; ++iRow)
        {
            for (int iCol = firstCol; iCol <= lastCol; ++iCol)
            {
                // Add the region to the list
                intersectingRegions.push_back(&mRegions[iRow][iCol]);
            }
        }
        return intersectingRegions;
    }

private:
    // Utility grid variables
    int mRegionSideLength = 100;
    int mTotalRegions = 0;
    int mRows = 0;
    int mCols = 0;
    int mGridWidth = 0;
    int mGridHeight = 0;

    std::vector<std::vector<RegionType>> mRegions;
};