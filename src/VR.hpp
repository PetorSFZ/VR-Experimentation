#pragma once

#include <openvr.h>

#include "sfz/math/Matrix.hpp"
#include "sfz/math/Vector.hpp"

namespace sfz {

namespace ovr {

// Eye constants
// ------------------------------------------------------------------------------------------------
constexpr uint32_t EYE_LEFT = 0;
constexpr uint32_t EYE_RIGHT = 1;
constexpr uint32_t eyes[] = { EYE_LEFT, EYE_RIGHT };

// VR matrices
// ------------------------------------------------------------------------------------------------

mat4 convertSteamVRMatrix(const vr::HmdMatrix34_t& matrix) noexcept;
mat4 convertSteamVRMatrix(const vr::HmdMatrix44_t& matrix) noexcept;

mat4 getHeadMatrix(uint32_t eye) noexcept;
mat4 getEyeMatrix(vr::IVRSystem* vrSystem, uint32_t eye) noexcept;
mat4 getProjectionMatrix(vr::IVRSystem* vrSystem, uint32_t eye, float near, float far) noexcept;

} // namespace ovr
} // namespace sfz
