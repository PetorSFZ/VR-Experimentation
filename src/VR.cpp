#include "VR.hpp"

#include "sfz/Assert.hpp"

namespace sfz {

namespace ovr {

// Statics
// ------------------------------------------------------------------------------------------------

void ensureHMDExists(vr::IVRSystem* vrSystem) noexcept
{
	if (vrSystem == nullptr) {
		sfz_assert_release(false);
		sfz::error("vr::IVRSystem* == nullptr");
	}
}

// VR matrices
// ------------------------------------------------------------------------------------------------

mat4 convertSteamVRMatrix(const vr::HmdMatrix34_t& matrix) noexcept
{
	return mat4({
		{matrix.m[0][0], matrix.m[0][1], matrix.m[0][2], matrix.m[0][3]},
		{matrix.m[1][0], matrix.m[1][1], matrix.m[1][2], matrix.m[1][3]},
		{matrix.m[2][0], matrix.m[2][1], matrix.m[2][2], matrix.m[2][3]},
		{0.0f, 0.0f, 0.0f, 1.0f}
	});
}

mat4 convertSteamVRMatrix(const vr::HmdMatrix44_t& matrix) noexcept
{
	return mat4({
		{matrix.m[0][0], matrix.m[0][1], matrix.m[0][2], matrix.m[0][3]},
		{matrix.m[1][0], matrix.m[1][1], matrix.m[1][2], matrix.m[1][3]},
		{matrix.m[2][0], matrix.m[2][1], matrix.m[2][2], matrix.m[2][3]},
		{matrix.m[3][0], matrix.m[3][1], matrix.m[3][2], matrix.m[3][3]}
	});
}

mat4 getEyeMatrix(vr::IVRSystem* vrSystem, uint32_t eye) noexcept
{
	ensureHMDExists(vrSystem);

	vr::HmdMatrix34_t mat = vrSystem->GetEyeToHeadTransform(vr::EVREye(eye));
	return inverse(convertSteamVRMatrix(mat));
}

mat4 getProjectionMatrix(vr::IVRSystem* vrSystem, uint32_t eye, float near, float far) noexcept
{
	ensureHMDExists(vrSystem);
	vr::HmdMatrix44_t mat = vrSystem->GetProjectionMatrix(vr::EVREye(eye), near, far, vr::API_OpenGL);
	return convertSteamVRMatrix(mat);
}

// Other
// ------------------------------------------------------------------------------------------------

vec2i getRecommendedRenderTargetSize(vr::IVRSystem* vrSystem) noexcept
{
	uint32_t w = 0, h = 0;
	vrSystem->GetRecommendedRenderTargetSize(&w, &h);
	return vec2i(int32_t(w), int32_t(h));
}

} // namespace ovr
} // namespace sfz
