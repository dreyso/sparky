#pragma once
#include <array>
#include <utility>
#include <tuple>


enum class Solution {NO_SOLUTION, POINT_SOLUTION, COLLINEAR_SOLUTION};

class Matrix;

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
    // Get the magnitude of the shadow
    float scalarProjectOn(const Vec& target) const;
    // Get the shadow as a ratio of the target vector
    float ratioProjectOn(const Vec& target) const;
    // get the shadow
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
    * Given 2 vectors, a and b, and their offsets, A and B, finds the type of intersection a and b have (if any),
    * and for point intersections, returns the coordinates of the point. For all other cases, the vector returned is
    * a zero vector.
    *
    * Pre-conditions:
    * Throws exception if a or b are zero vectors.
    *
    * Input:
    * Four vectors, a and its offset A, b and its offset B.
    *
    * Output:
    * A std::pair with the first element notating the type of intersection, and
    * the second element mapping its location (for point intersections only).
    */
    static std::tuple<Solution, Vec, Vec> findIntersection(const Vec& A, const Vec& a, const Vec& B, const Vec& b);

private:
    float mX = 0.f, mY = 0.f;
}; 

class Matrix
{
public:
    Matrix() = default;
    Matrix(float x1, float y1, float x2, float y2) : mMatrix{ { { {x1,y1} }, { {x2,y2} } } } {}
    Matrix(const Vec& a, const Vec& b) : mMatrix{ { { {a.getX(),a.getY()} }, { {b.getX(),b.getY()} } } } {}

    ~Matrix() = default;

    std::array<float, 2>& operator[](int row);
    const std::array<float, 2>& operator[](int row) const;

private:
    std::array<std::array<float, 2>, 2> mMatrix{ { { {0.f,0.f} }, { {0.f,0.f} } } };
};