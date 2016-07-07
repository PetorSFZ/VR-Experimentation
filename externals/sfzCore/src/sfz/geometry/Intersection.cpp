// Copyright (c) Peter Hillerstr�m (skipifzero.com, peter@hstroem.se)
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#include "sfz/geometry/Intersection.hpp"

#include "sfz/geometry/AABB.hpp"
#include "sfz/geometry/AABB2D.hpp"
#include "sfz/geometry/Circle.hpp"
#include "sfz/geometry/OBB.hpp"
#include "sfz/geometry/Plane.hpp"
#include "sfz/geometry/Sphere.hpp"

namespace sfz {

// Static functions
// ------------------------------------------------------------------------------------------------

static bool intersectsPlane(const Plane& plane, const vec3& position, float projectedRadius) noexcept
{
	// Part of plane SAT algorithm from Real-Time Collision Detection
	float dist = plane.signedDistance(position);
	return std::abs(dist) <= projectedRadius;
}

static bool abovePlane(const Plane& plane, const vec3& position, float projectedRadius) noexcept
{
	// Part of plane SAT algorithm from Real-Time Collision Detection
	float dist = plane.signedDistance(position);
	return dist >= (-projectedRadius);
}

static bool belowPlane(const Plane& plane, const vec3& position, float projectedRadius) noexcept
{
	// Part of plane SAT algorithm from Real-Time Collision Detection
	float dist = plane.signedDistance(position);
	return dist <= projectedRadius;
}

// Point inside primitive tests
// ------------------------------------------------------------------------------------------------

bool pointInside(const AABB& box, const vec3& point) noexcept
{
	return box.min()[0] < point[0] && point[0] < box.max()[0] &&
	       box.min()[1] < point[1] && point[1] < box.max()[1] &&
	       box.min()[2] < point[2] && point[2] < box.max()[2];
}

bool pointInside(const OBB& box, const vec3& point) noexcept
{
	// Modified closest point algorithm from Real-Time Collision Detection (Section 5.1.4)
	const vec3 distToPoint = point - box.position();
	const std::array<vec3,3>& axes = box.axes();
	float dist;
	for (size_t i = 0; i < 3; i++) {
		dist = dot(distToPoint, axes[i]);
		if (dist > box.halfExtents()[i]) return false;
		if (dist < -box.halfExtents()[i]) return false;
	}
	return true;
}

bool pointInside(const Sphere& sphere, const vec3& point) noexcept
{
	const vec3 distToPoint = point - sphere.position();
	return squaredLength(distToPoint) < sphere.radius() * sphere.radius();
}

bool pointInside(const Circle& circle, vec2 point) noexcept
{
	// If the length from the circles center to the specified point is shorter than or equal to
	// the radius then the Circle overlaps the point. Both sides of the equation is squared to
	// avoid somewhat expensive sqrt() function.
	return squaredLength(point - circle.pos) <= (circle.radius*circle.radius);
}

bool pointInside(const AABB2D& rect, vec2 point) noexcept
{
	return rect.min.x <= point.x && point.x <= rect.max.x &&
	       rect.min.y <= point.y && point.y <= rect.max.y;
}

// Primitive vs primitive tests (same type)
// ------------------------------------------------------------------------------------------------

bool intersects(const AABB& boxA, const AABB& boxB) noexcept
{
	// Boxes intersect if they overlap on all axes.
	if (boxA.max()[0] < boxB.min()[0] || boxA.min()[0] > boxB.max()[0]) return false;
	if (boxA.max()[1] < boxB.min()[1] || boxA.min()[1] > boxB.max()[1]) return false;
	if (boxA.max()[2] < boxB.min()[2] || boxA.min()[2] > boxB.max()[2]) return false;
	return true;
}

bool intersects(const OBB& a, const OBB& b) noexcept
{
	// OBB vs OBB SAT (Separating Axis Theorem) test from Real-Time Collision Detection
	// (chapter 4.4.1 OBB-OBB Intersection)

	const std::array<vec3,3>& aU = a.axes();
	const vec3& aE = a.halfExtents();
	const std::array<vec3,3>& bU = b.axes();
	const vec3& bE = b.halfExtents();

	// Compute the rotation matrix from b to a
	mat3 R;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			R.set(i, j, dot(aU[i], bU[j]));
		}
	}

	// Compute common subexpressions, epsilon term to counteract arithmetic errors
	const float EPSILON = 0.00001f;
	mat3 AbsR;
	for (size_t i = 0; i < 3; i++) {
		for (size_t j = 0; j < 3; j++) {
			AbsR.set(i, j, std::abs(R.at(i, j)) + EPSILON);
		}
	}

	// Calculate translation vector from a to b and bring it into a's frame of reference
	vec3 t = b.position() - a.position();
	t = vec3{dot(t, aU[0]), dot(t, aU[1]), dot(t, aU[2])};

	float ra, rb;

	// Test axes L = aU[0], aU[1], aU[2]
	for (size_t i = 0; i < 3; i++) {
		ra = aE[i];
		rb = bE[0]*AbsR.at(i,0) + bE[1]*AbsR.at(i,1) + bE[2]*AbsR.at(i,2);
		if (std::abs(t[i]) > ra + rb) return false;
	}

	// Test axes L = bU[0], bU[1], bU[2]
	for (size_t i = 0; i < 3; i++) {
		ra = aE[0]*AbsR.at(0,i) + aE[1]*AbsR.at(1,i) + aE[2]*AbsR.at(2,i);
		rb = bE[i];
		if (std::abs(t[0]*R.at(0,i) + t[1]*R.at(1,i) + t[2]*R.at(2,i)) > ra + rb) return false;
	}

	// Test axis L = aU[0] x bU[0]
	ra = aE[1]*AbsR.at(2,0) + aE[2]*AbsR.at(1,0);
	rb = bE[1]*AbsR.at(0,2) + bE[2]*AbsR.at(0,1);
	if (std::abs(t[2]*R.at(1,0) - t[1]*R.at(2,0)) > ra + rb) return false;

	// Test axis L = aU[0] x bU[1]
	ra = aE[1]*AbsR.at(2,1) + aE[2]*AbsR.at(1,1);
	rb = bE[0]*AbsR.at(0,2) + bE[2]*AbsR.at(0,0);
	if (std::abs(t[2]*R.at(1,1) - t[1]*R.at(2,1)) > ra + rb) return false;

	// Test axis L = aU[0] x bU[2]
	ra = aE[1]*AbsR.at(2,2) + aE[2]*AbsR.at(1,2);
	rb = bE[0]*AbsR.at(0,1) + bE[1]*AbsR.at(0,0);
	if (std::abs(t[2]*R.at(1,2) - t[1]*R.at(2,2)) > ra + rb) return false;

	// Test axis L = aU[1] x bU[0]
	ra = aE[0]*AbsR.at(2,0) + aE[2]*AbsR.at(0,0);
	rb = bE[1]*AbsR.at(1,2) + bE[2]*AbsR.at(1,1);
	if (std::abs(t[0]*R.at(2,0) - t[2]*R.at(0,0)) > ra + rb) return false;

	// Test axis L = aU[1] x bU[1]
	ra = aE[0]*AbsR.at(2,1) + aE[2]*AbsR.at(0,1);
	rb = bE[0]*AbsR.at(1,2) + bE[2]*AbsR.at(1,0);
	if (std::abs(t[0]*R.at(2,1) - t[2]*R.at(0,1)) > ra + rb) return false;

	// Test axis L = aU[1] x bU[2]
	ra = aE[0]*AbsR.at(2,2) + aE[2]*AbsR.at(0,2);
	rb = bE[0]*AbsR.at(1,1) + bE[1]*AbsR.at(1,0);
	if (std::abs(t[0]*R.at(2,2) - t[2]*R.at(0,2)) > ra + rb) return false;

	// Test axis L = aU[2] x bU[0]
	ra = aE[0]*AbsR.at(1,0) + aE[1]*AbsR.at(0,0);
	rb = bE[1]*AbsR.at(2,2) + bE[2]*AbsR.at(2,1);
	if (std::abs(t[1]*R.at(0,0) - t[0]*R.at(1,0)) > ra + rb) return false;

	// Test axis L = aU[2] x bU[1]
	ra = aE[0]*AbsR.at(1,1) + aE[1]*AbsR.at(0,1);
	rb = bE[0]*AbsR.at(2,2) + bE[2]*AbsR.at(2,0);
	if (std::abs(t[1]*R.at(0,1) - t[0]*R.at(1,1)) > ra + rb) return false;

	// Test axis L = aU[2] x bU[2]
	ra = aE[0]*AbsR.at(1,2) + aE[1]*AbsR.at(0,2);
	rb = bE[0]*AbsR.at(2,1) + bE[1]*AbsR.at(2,0);
	if (std::abs(t[1]*R.at(0,2) - t[0]*R.at(1,2)) > ra + rb) return false;

	// If no separating axis can be found then the OBBs must be intersecting.
	return true;
}

bool intersects(const Sphere& sphereA, const Sphere& sphereB) noexcept
{
	const vec3 distVec = sphereA.position() - sphereB.position();
	const float squaredDist = dot(distVec, distVec);
	const float radiusSum = sphereA.radius() + sphereB.radius();
	const float squaredRadiusSum = radiusSum * radiusSum;
	return squaredDist <= squaredRadiusSum;
}

bool overlaps(const Circle& lhs, const Circle& rhs) noexcept
{
	// If the length between the center of the two circles is less than or equal to the the sum of
	// the circle's radiuses they overlap. Both sides of the equation is squared to avoid somewhat 
	// expensive sqrt() function.
	float distSquared = squaredLength(lhs.pos - rhs.pos);
	float radiusSum = lhs.radius + rhs.radius;
	return distSquared <= (radiusSum * radiusSum);
}

bool overlaps(const AABB2D& lhs, const AABB2D& rhs) noexcept
{
	return lhs.min.x <= rhs.max.x &&
	       lhs.max.x >= rhs.min.x &&
	       lhs.min.y <= rhs.max.y &&
	       lhs.max.y >= rhs.min.y;
}

// AABB2D & Circle tests
// ------------------------------------------------------------------------------------------------

bool overlaps(const Circle& circle, const AABB2D& rect) noexcept
{
	// If the length between the center of the circle and the closest point on the rectangle is
	// less than or equal to the circles radius they overlap. Both sides of the equation is 
	// squared to avoid somewhat expensive sqrt() function.
	vec2 e{max(rect.min - circle.pos, 0.0f)};
	e += max(circle.pos - rect.max, 0.0f);
	return squaredLength(e) <= circle.radius * circle.radius;
}

bool overlaps(const AABB2D& rect, const Circle& circle) noexcept
{
	return overlaps(circle, rect);
}

// Plane & AABB tests
// ------------------------------------------------------------------------------------------------

bool intersects(const Plane& plane, const AABB& aabb) noexcept
{
	// SAT algorithm from Real-Time Collision Detection (chapter 5.2.3)

	// Projected radius on line towards closest point on plane
	float projectedRadius = aabb.halfXExtent() * std::abs(plane.normal()[0])
	                      + aabb.halfYExtent() * std::abs(plane.normal()[1])
	                      + aabb.halfZExtent() * std::abs(plane.normal()[2]);

	return intersectsPlane(plane, aabb.position(), projectedRadius);
}

bool intersects(const AABB& aabb, const Plane& plane) noexcept
{
	return intersects(plane, aabb);
}

bool abovePlane(const Plane& plane, const AABB& aabb) noexcept
{
	// Modified SAT algorithm from Real-Time Collision Detection (chapter 5.2.3)

	// Projected radius on line towards closest point on plane
	float projectedRadius = aabb.halfXExtent() * std::abs(plane.normal()[0])
	                      + aabb.halfYExtent() * std::abs(plane.normal()[1])
	                      + aabb.halfZExtent() * std::abs(plane.normal()[2]);

	return abovePlane(plane, aabb.position(), projectedRadius);
}

bool belowPlane(const Plane& plane, const AABB& aabb) noexcept
{
	// Modified SAT algorithm from Real-Time Collision Detection (chapter 5.2.3)
	// Projected radius on line towards closest point on plane
	float projectedRadius = aabb.halfXExtent() * std::abs(plane.normal()[0])
	                      + aabb.halfYExtent() * std::abs(plane.normal()[1])
	                      + aabb.halfZExtent() * std::abs(plane.normal()[2]);

	return belowPlane(plane, aabb.position(), projectedRadius);
}

// Plane & OBB tests
// ------------------------------------------------------------------------------------------------

bool intersects(const Plane& plane, const OBB& obb) noexcept
{
	// SAT algorithm from Real-Time Collision Detection (chapter 5.2.3)
	// Projected radius on line towards closest point on plane
	float projectedRadius = obb.halfXExtent() * std::abs(dot(plane.normal(), obb.xAxis()))
	                      + obb.halfYExtent() * std::abs(dot(plane.normal(), obb.yAxis()))
	                      + obb.halfZExtent() * std::abs(dot(plane.normal(), obb.zAxis()));

	return intersectsPlane(plane, obb.position(), projectedRadius);
}

bool intersects(const OBB& obb, const Plane& plane) noexcept
{
	return intersects(plane, obb);
}

bool abovePlane(const Plane& plane, const OBB& obb) noexcept
{
	// Modified SAT algorithm from Real-Time Collision Detection (chapter 5.2.3)
	// Projected radius on line towards closest point on plane
	float projectedRadius = obb.halfXExtent() * std::abs(dot(plane.normal(), obb.xAxis()))
	                      + obb.halfYExtent() * std::abs(dot(plane.normal(), obb.yAxis()))
	                      + obb.halfZExtent() * std::abs(dot(plane.normal(), obb.zAxis()));

	return abovePlane(plane, obb.position(), projectedRadius);
}

bool belowPlane(const Plane& plane, const OBB& obb) noexcept
{
	// Modified SAT algorithm from Real-Time Collision Detection (chapter 5.2.3)
	// Projected radius on line towards closest point on plane
	float projectedRadius = obb.halfXExtent() * std::abs(dot(plane.normal(), obb.xAxis()))
	                      + obb.halfYExtent() * std::abs(dot(plane.normal(), obb.yAxis()))
	                      + obb.halfZExtent() * std::abs(dot(plane.normal(), obb.zAxis()));

	return belowPlane(plane, obb.position(), projectedRadius);
}

// Plane & Sphere tests
// ------------------------------------------------------------------------------------------------

bool intersects(const Plane& plane, const Sphere& sphere) noexcept
{
	return intersectsPlane(plane, sphere.position(), sphere.radius());
}

bool intersects(const Sphere& sphere, const Plane& plane) noexcept
{
	return intersects(plane, sphere);
}

bool abovePlane(const Plane& plane, const Sphere& sphere) noexcept
{
	return abovePlane(plane, sphere.position(), sphere.radius());
}

bool belowPlane(const Plane& plane, const Sphere& sphere) noexcept
{
	return belowPlane(plane, sphere.position(), sphere.radius());
}

} // namespace sfz
