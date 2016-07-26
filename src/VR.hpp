// Copyright (c) Peter Hillerström (skipifzero.com, peter@hstroem.se)

#pragma once

#include "sfz/math/Matrix.hpp"
#include "sfz/math/Vector.hpp"

namespace sfz {

// Eye constants
// ------------------------------------------------------------------------------------------------

constexpr uint32_t LEFT_EYE = 0;
constexpr uint32_t RIGHT_EYE = 1;
constexpr uint32_t VR_EYES[] = { LEFT_EYE, RIGHT_EYE };

// Struct representing the Head Mounted Device
// ------------------------------------------------------------------------------------------------

struct HMD final {
	float near = 0.01f;
	mat4 originMatrix = identityMatrix4<float>();
	mat4 headMatrix = identityMatrix4<float>();
	mat4 eyeMatrix[2] = { identityMatrix4<float>(), identityMatrix4<float>() };
	mat4 projMatrix[2] = { identityMatrix4<float>(), identityMatrix4<float>() };
	inline vec3 originPos() const noexcept { return translation(originMatrix); }
	inline vec3 headPos() const noexcept { return originPos() + translation(headMatrix); }
};

// VR manager class
// ------------------------------------------------------------------------------------------------

/// Wrapper around OpenVR, hopefully general enough that it should not be super painful to change
/// to another API in the future if necessary.
class VR final {
public:
	// Singleton instance
	// --------------------------------------------------------------------------------------------

	static VR& instance() noexcept;

	// Public methods
	// --------------------------------------------------------------------------------------------

	/// Quickly checks if hmd is present, can be called before initialization
	bool isHmdPresent() noexcept;

	/// Quickly checks if OpenVR runtime is installed, can be called before initialization
	bool isRuntimeInstalled() noexcept;

	/// Initializes OpenVR and this whole VR manager
	bool initialize() noexcept;

	/// Deinitializes OpenVR and this whole VR manager
	void deinitialize() noexcept;

	/// Updates this VR manager, should be called once in the beginning of a frame before rendering
	void update() noexcept;

	/// Submits the textures for each eye to OpenVR runtime, then calls SDL_GL_SwapWindow().
	/// The gammaCorrect flag tells whether the submitted textures are gamma corrected or not.
	/// The uvMax parameter tells how large part of the texture should be used ((1,1) is all of it),
	/// useful in order to implement dynamic resolution.
	void submitAndSwap(void* sdlWindowPtr, uint32_t leftEyeTex, uint32_t rightEyeTex,
	                   vec2 uvMax = vec2(1.0), bool gammaCorrect = false) noexcept;

	// Getters
	// --------------------------------------------------------------------------------------------

	inline bool isInitialized() const noexcept { return mSystemPtr != nullptr; }
	inline const HMD& hmd() const noexcept { return mHMD; }
	inline HMD& hmd() noexcept { return mHMD; }
	vec2i getRecommendedRenderTargetSize() noexcept;

private:
	// Private constructors & destructors
	// --------------------------------------------------------------------------------------------

	VR(const VR&) = delete;
	VR& operator= (const VR&) = delete;
	VR(VR&&) = delete;
	VR& operator= (VR&&) = delete;

	VR() noexcept;
	~VR() noexcept;

	// Private members
	// --------------------------------------------------------------------------------------------

	void* mSystemPtr = nullptr;
	HMD mHMD;
};

} // namespace sfz
