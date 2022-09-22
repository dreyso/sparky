#include "../header/vec.h"

#include <array>
#include <utility>
#include <stdexcept>


std::array<float, 2>& Matrix::operator[](int row)
{
	return mMatrix[row];
}

const std::array<float, 2>& Matrix::operator[](int row) const
{
	return mMatrix[row];
}

float Vec::operator[](int row) const
{
	if (row == 0)
		return mX;
	else if(row == 1)
		return mY;
	else
		throw(std::runtime_error{ "Error: Out of bounds vector access\n" });
}

// Getters
float Vec::getX() const
{
	return mX;
}

float Vec::getY() const
{
	return mY;
}

bool Vec::isZeroVector() const
{
	if (mX == 0 && mY == 0)
		return true;
	return false;
}

// Setters
void Vec::setX(float xValue)
{
	mX = xValue;
}

void Vec::setY(float yValue)
{
	mY = yValue;
}

// Vector by vector operations
float Vec::operator*(const Vec& vec) const    // Dot product
{
	return mX * vec.mX + mY * vec.mY;
}

float Vec::cross(const Vec& vec2) const       // 2d cross product (determinant)
{
	return mX * vec2.mY - mY * vec2.mX;
}

float Vec::scalarProjectOn(const Vec& target) const
{
	return (*this * target) / target.getMagnitude();
}

Vec Vec::projectOn(const Vec& target) const
{
	return (*this * target) / powf(target.getMagnitude(), 2.f) * target;
}

Vec& Vec::operator=(const Vec& vec)
{
	mX = vec.mX, mY = vec.mY;
	return *this;
}

Vec& Vec::operator+=(const Vec& vec)
{
	mX += vec.mX, mY += vec.mY;
	return *this;
}

Vec& Vec::operator-=(const Vec& vec)
{
	mX -= vec.mX, mY -= vec.mY;
	return *this;
}

Vec Vec::operator+(const Vec& vec) const
{
	return Vec{mX + vec.mX, mY + vec.mY};
}

Vec Vec::operator-(const Vec& vec) const
{
	return Vec{ mX - vec.mX, mY - vec.mY };
}

bool Vec::operator==(const Vec& vec) const
{
	return mX == vec.mX && mY == vec.mY;
}

// Scalar operations
Vec& Vec::operator*=(float scalar)
{
	mX *= scalar, mY *= scalar;
	return *this;
}

Vec& Vec::operator/=(float scalar)
{
	mX /= scalar, mY /= scalar;
	return *this;
}

Vec Vec::operator*(float scalar) const
{
	return Vec{ mX * scalar, mY * scalar };
}

Vec operator* (float scalar, const Vec& vec)
{
	return Vec{ vec.mX * scalar, vec.mY * scalar };
}

Vec Vec::operator/(float scalar) const
{
	return Vec{ mX / scalar, mY / scalar };
}

// Matrix operations
Vec& Vec::operator*=(const Matrix& matrix)
{
	float tempX = matrix[0][0] * mX + matrix[0][1] * mY;
	float tempY = matrix[1][0] * mX + matrix[1][1] * mY;
	mX = tempX, mY = tempY;
	return *this;
}

Vec Vec::operator*(const Matrix& matrix) const
{
	float returnX = matrix[0][0] * mX + matrix[0][1] * mY;
	float returnY = matrix[1][0] * mX + matrix[1][1] * mY;
	return Vec{returnX, returnY};
}

// Misc
void Vec::normalize()
{
	auto mag = getMagnitude();
	if (mag == 0.f)
		throw(std::runtime_error{ "Error: Attempted to normalize zero vector\n" });

	// Divide this vector by its magnitude
	*this /= mag;
}

float Vec::getMagnitude() const
{
	return sqrtf(powf(mX, 2.f) + powf(mY, 2.f));
}

std::pair<Solution, Vec> Vec::findIntersection(const Vec& A, const Vec& 𝙖, const Vec& B, const Vec& 𝒃)
{
	// Method credit: https:// stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282

	// Frequently used values
	Vec BminusA = B - A;	// B - A
	float BminusACross𝒃 = BminusA.cross(𝒃);	// B - A × 𝒃
	float BminusACross𝙖 = BminusA.cross(𝙖);	// B - A × 𝙖
	float 𝙖Cross𝒃 = 𝙖.cross(𝒃);	// 𝙖 × 𝒃

	// 𝙖 × 𝒃 = 0 means the 𝙖 and 𝒃 are parallel
	// (B − A) × 𝙖 = 0 means the vector between the position vectors is collinear to 𝙖 (i.e., the starts of 𝙖 and 𝒃 can be drawn on one line)
	
	// If 𝙖 × 𝒃 = 0 and (B − A) × 𝙖 = 0, then the two vectors are collinear.
	if (𝙖Cross𝒃 == 0.f && BminusACross𝙖 == 0.f)
	{
		// A, B, 𝙖, and 𝒃 are now all span the same line
		// Project B - A ontu 𝙖, and B - A + 𝒃 ontu 𝙖
	
		// proj0 = Projection of (B - A) onto 𝙖
		float proj0 = BminusA.scalarProjectOn(𝙖);

		// proj1 = Projection of (B - A + 𝒃) onto 𝙖
		float proj1 = proj0 + 𝒃.scalarProjectOn(𝙖);

		if ((𝒃 * 𝙖) < 0.f) // 𝒃 and 𝙖 point in opposite directions
		{
			if (proj1 <= 1.f && proj0 >= 0.f)	// Check if [proj1, proj0] intersects [0,1]
				return std::pair{ Solution::COLLINEAR_SOLUTION, Vec{} };    // Collinear and overlapping
			else
				return std::pair{ Solution::NO_SOLUTION, Vec{} };           // Collinear and disjoint
		}
		else if (proj0 <= 1.f && proj1 >= 0.f)	// Check if [proj0, proj1] intersects interval[0, 1]
			return std::pair{ Solution::COLLINEAR_SOLUTION, Vec{} };    // Collinear and overlapping
		else
			return std::pair{ Solution::NO_SOLUTION, Vec{} };           // Collinear and disjoint
	}

	// If 𝙖 × 𝒃 = 0 and (B − A) × 𝙖 ≠ 0, then the vectors are parallel and don't intersect
	else if (𝙖Cross𝒃 == 0.f && BminusACross𝙖 != 0.f)
		return std::pair{ Solution::NO_SOLUTION, Vec{} };
	
	// At this point, the lines are not parallel or collinear, so it should be possible to solve for α and β
	
	// A + α𝙖 = B + β𝒃
	// α = (B - A) × 𝒃 / (𝙖 × 𝒃)
	float α = (BminusACross𝒃) / 𝙖Cross𝒃;

	// β = (B − A) × 𝙖 / (𝙖 × 𝒃)
	float β = (BminusACross𝙖) / 𝙖Cross𝒃;

	// If α and β don't extend their respective vectors, then the vectors intersect
	if (𝙖Cross𝒃 != 0.f && α >= 0.f && α <= 1.f && β >= 0.f && β <= 1.f)
		return std::pair{ Solution::POINT_SOLUTION, A + (α * 𝙖) };

	// Otherwise, the vectors don't intersect because they're too short
	else
		return std::pair{ Solution::NO_SOLUTION, Vec{} };
}
