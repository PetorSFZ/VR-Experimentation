// Copyright (c) Peter Hillerström (skipifzero.com, peter@hstroem.se)

#pragma once

#include "sfz/containers/DynArray.hpp"
#include "sfz/gl/Model.hpp"
#include "sfz/math/Matrix.hpp"
#include "sfz/math/Vector.hpp"

namespace sfz {

using gl::Model;

// Eye constants
// ------------------------------------------------------------------------------------------------

constexpr uint32_t LEFT_EYE = 0;
constexpr uint32_t RIGHT_EYE = 1;
constexpr uint32_t VR_EYES[] = { LEFT_EYE, RIGHT_EYE };

// Head Mounted Device
// ------------------------------------------------------------------------------------------------

struct HMD final {
	float near = 0.01f;
	mat4 headMatrix = identityMatrix4<float>();
	mat4 eyeMatrix[2] = { identityMatrix4<float>(), identityMatrix4<float>() };
	mat4 projMatrix[2] = { identityMatrix4<float>(), identityMatrix4<float>() };
	inline vec3 headPos() const noexcept { return translation(headMatrix); }
};

// Controllers
// ------------------------------------------------------------------------------------------------

struct Controller final {
	mat4 transform;
	inline vec3 pos() const noexcept { return translation(transform); }
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

	/// Submits the textures for each eye to OpenVR runtime
	/// The gammaCorrect flag tells whether the submitted textures are gamma corrected or not.
	/// The uvMax parameter tells how large part of the texture should be used ((1,1) is all of it),
	/// useful in order to implement dynamic resolution.
	void submit(void* sdlWindowPtr, uint32_t leftEyeTex, uint32_t rightEyeTex,
	            vec2 uvMax = vec2(1.0), bool gammaCorrect = false) noexcept;

	// Getters
	// --------------------------------------------------------------------------------------------

	inline bool isInitialized() const noexcept { return mSystemPtr != nullptr; }
	inline vec2i recommendedRenderTargetSize() const noexcept { return mRecommendedRenderTargetSize; }
	inline const HMD& hmd() const noexcept { return mHMD; }
	inline HMD& hmd() noexcept { return mHMD; }

	inline const Controller& controller(uint32_t eye) const noexcept { return mControllers[eye]; }
	inline const Model& controllerModel(uint32_t eye) const noexcept { return mControllerModels[eye]; }

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
	vec2i mRecommendedRenderTargetSize = vec2i(0, 0);
	HMD mHMD;
	Controller mControllers[2];
	Model mControllerModels[2];
	mutable DynArray<char> mTempStrBuffer;
};

} // namespace sfz
