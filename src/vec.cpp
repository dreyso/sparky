#include "../header/vec.h"

#include <array>
#include <utility>
#include <tuple>
#include <stdexcept>

const float EQUIVALENCE_THRESHOLD = 0.01f;

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
	// Second vector, 𝙫1, must be to the left of 𝙫0 for positive area
	return mX * vec2.mY - mY * vec2.mX;
}

float Vec::scalarProjectOn(const Vec& target) const
{
	if(target.isZeroVector())
		throw(std::runtime_error{ "Error: Cannot project on the zero vector\n" });

	return (*this * target) / target.getMagnitude();
}

// Get the shadow as a ratio of the target vector
float Vec::ratioProjectOn(const Vec& target) const
{
	if (target.isZeroVector())
		throw(std::runtime_error{ "Error: Cannot project on the zero vector\n" });

	return (*this * target) / powf(target.getMagnitude(), 2.f);
}

Vec Vec::projectOn(const Vec& target) const
{
	return (Vec{ target }) * ratioProjectOn(target);
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
	return (fabs(mX - vec.mX) < EQUIVALENCE_THRESHOLD && fabs(mY - vec.mY) < EQUIVALENCE_THRESHOLD);
}

bool Vec::operator!=(const Vec& vec) const
{
	return !(*this == vec);
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

std::tuple<Solution, Vec, Vec> Vec::findIntersection(const Vec& A, const Vec& a, const Vec& B, const Vec& b)
{
	// Method credit: https:// stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect/565282#565282

	if(a.isZeroVector() || b.isZeroVector())
		throw(std::invalid_argument{ "Error: Cannot test if zero vectors intersect\n" });

	// Frequently used values
	Vec BminusA = B - A;	// B - A
	float BminusACrossb = BminusA.cross(b);	// B - A × b
	float BminusACrossa = BminusA.cross(a);	// B - A × a
	float aCrossb = a.cross(b);	// a × b

	// a × b = 0 means the a and b are parallel
	// (B − A) × a = 0 means the vector between the position vectors is collinear to a (i.e., the starts of a and b can be drawn on one line)
	
	// If a × b = 0 and (B − A) × a = 0, then the two vectors are collinear.
	if (aCrossb == 0.f && BminusACrossa == 0.f)
	{
		// A, B, a, and b are now all span the same line
		// Arbitrarily, the direction of 'a' will be considered positive. Since all of the vectors
		// are in the same space (a line), scalar projections are equal to the magnitude, but they
		// also include direction.

		Vec positiveDirection{ a };
		positiveDirection.normalize();

		// Interval a
		float end1 = A.scalarProjectOn(positiveDirection);
		float end2 = end1 + a.getMagnitude();	// Already positive

		float aStart = std::min(end1, end2);
		float aEnd = std::max(end1, end2);

		// Interval b
		end1 = B.scalarProjectOn(positiveDirection);
		end2 = end1 + b.scalarProjectOn(positiveDirection);

		float bStart = std::min(end1, end2);
		float bEnd = std::max(end1, end2);

		// Check if a and b overlap
		if (bStart > aEnd || aStart > bEnd)
			return { Solution::NO_SOLUTION, Vec{}, Vec{} };	// Collinear and disjoint

		// Calculate the interval
		float overlapStart = std::max(aStart, bStart);
		float overlapEnd = std::min(aEnd, bEnd);

		// Check if the overlap is a single point
		if (overlapEnd - overlapStart == 0.f)	
			return std::make_tuple(Solution::POINT_SOLUTION, Vec{ positiveDirection } *overlapStart, Vec{ 0.f,0.f });	// Point solution

		//Collinear and overlapping
		return std::make_tuple(Solution::COLLINEAR_SOLUTION, Vec{ positiveDirection } * overlapStart, Vec{ positiveDirection } * (overlapEnd - overlapStart));	// Interval solution
	}

	// If a × b = 0 and (B − A) × a ≠ 0, then the vectors are parallel and don't intersect
	else if (aCrossb == 0.f && BminusACrossa != 0.f)
		return std::make_tuple(Solution::NO_SOLUTION, Vec{}, Vec{});
	
	// At this point, the lines are not parallel or collinear, so it should be possible to solve for α and β
	
	// A + αa = B + βb
	// α = (B - A) × b / (a × b)
	float α = (BminusACrossb) / aCrossb;

	// β = (B − A) × a / (a × b)
	float β = (BminusACrossa) / aCrossb;

	// If α and β don't extend their respective vectors, then the vectors intersect
	if (aCrossb != 0.f && α >= 0.f && α <= 1.f && β >= 0.f && β <= 1.f)
		return std::make_tuple(Solution::POINT_SOLUTION, A + (α * a), Vec{});

	// Otherwise, the vectors don't intersect because they're too short
	else
		return std::make_tuple(Solution::NO_SOLUTION, Vec{}, Vec{});
}
