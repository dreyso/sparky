#pragma once
#include <array>
#include <utility>


enum class Solution {NO_SOLUTION, POINT_SOLUTION, COLLINEAR_SOLUTION};

class Matrix
{
public:
    Matrix() = default;
    Matrix(float x1, float y1, float x2, float y2) : mMatrix{ { { {x1,y1} }, { {x2,y2} } } } {}
    ~Matrix() = default;

     std::array<float, 2>& operator[](int row);
     const std::array<float, 2>& operator[](int row) const;

private:
    std::array<std::array<float, 2>, 2> mMatrix{ { { {0.f,0.f} }, { {0.f,0.f} } } };
};

class Vec
{
public:
    Vec() = default;
    Vec(float x, float y) : mX{ x }, mY{ y }{}
    ~Vec() = default;

    // Getters
    float operator[](int row) const;
    float getX() const;
    float getY() const;
    bool isZeroVector() const;

    // Setters
    void setX(float xValue);
    void setY(float yValue);

    // Vector by vector operations
    float cross(const Vec& vec2) const;       // 2d cross product (determinant)
    float scalarProjectOn(const Vec& target) const;
    Vec projectOn(const Vec& target) const;

    Vec& operator=(const Vec& vec);
    float operator*(const Vec& vec) const;    // Dot product
    Vec& operator+=(const Vec& vec);
    Vec& operator-=(const Vec& vec);
    Vec operator+(const Vec& vec) const;
    Vec operator-(const Vec& vec) const;
    bool operator==(const Vec& vec) const;
    bool operator!=(const Vec& vec) const;


    // Scalar operations
    Vec& operator*=(float scalar);
    Vec& operator/=(float scalar);
    Vec operator*(float scalar) const;
    friend Vec operator* (float scalar, const Vec& vec);
    Vec operator/(float scalar) const;

    // Matrix operations
    Vec& operator*=(const Matrix& matrix);
    Vec operator*(const Matrix& matrix) const;

    // Misc
    void normalize();
    float getMagnitude() const;
    
    /**
    * Description:
    * Given 2 vectors, 𝙖 and 𝒃, and their offsets, A and B, finds the type of intersection 𝙖 and 𝒃 have (if any),
    * and for point intersections, returns the coordinates of the point. For all other cases, the vector returned is
    * a zero vector.
    *
    * Pre-conditions:
    * Throws exception if 𝙖 or 𝒃 are zero vectors.
    *
    * Input:
    * Four vectors, 𝙖 and its offset A, 𝒃 and its offset B.
    *
    * Output:
    * A std::pair with the first element notating the type of intersection, and
    * the second element mapping its location (for point intersections only).
    */
    static std::pair<Solution, Vec> findIntersection(const Vec& A, const Vec& 𝙖, const Vec& B, const Vec& 𝒃);

private:
    float mX = 0.f, mY = 0.f;
}; 
