// Copyright (c) Peter Hillerström (skipifzero.com, peter@hstroem.se)

#pragma once

#include "sfz/containers/DynArray.hpp"
#include "sfz/gl/Model.hpp"
#include "sfz/math/Matrix.hpp"
#include "sfz/math/Vector.hpp"
#include "sfz/sdl/ButtonState.hpp"

namespace sfz {

using gl::Model;
using sdl::ButtonState;

// Eye constants
// ------------------------------------------------------------------------------------------------

constexpr uint32_t LEFT_EYE = 0;
constexpr uint32_t RIGHT_EYE = 1;
constexpr uint32_t VR_EYES[] = { LEFT_EYE, RIGHT_EYE };

// Tracked device
// ------------------------------------------------------------------------------------------------

enum class TrackedDeviceType {
	HMD,
	CONTROLLER,
	TRACKING_REFERENCE
};

struct TrackedDevice final {
	// Identifying information
	TrackedDeviceType type;
	uint32_t deviceId;

	// Position and rotation relative to room origin
	mat4 transform = identityMatrix4<float>();
	// TODO: Predicted transform

	// Model and texture
	Model model;

	// Helper functions
	inline vec3 pos() const noexcept { return translation(transform); }
	// TODO: dir()
	// TODO: up()
};

struct VRControllerState {
	ButtonState menuButton = ButtonState::NOT_PRESSED;
	ButtonState gripButton = ButtonState::NOT_PRESSED;
	ButtonState touchpadButton = ButtonState::NOT_PRESSED;
	vec2 touchpad = vec2(0.0f); // Range [-1.0, 1.0]
	float trigger = 0.0f; // Range [0.0, 1.0]
	// TODO: vec2 pos, vec2 dir, vec2 up
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

	inline bool isInitialized() const noexcept { return mSystemPtr != nullptr; };
	vec2i recommendedRenderTargetSize() const noexcept;
	mat4 headMatrix() const noexcept;
	mat4 eyeMatrix(uint32_t eye) const noexcept;
	mat4 projMatrix(uint32_t eye, float near = 0.01f) const noexcept;
	inline const DynArray<TrackedDevice>& trackedDevices() const noexcept { return mTrackedDevices; }

	const TrackedDevice* hmd() const noexcept;
	const TrackedDevice* leftController() const noexcept;
	const TrackedDevice* rightController() const noexcept;
	inline const VRControllerState& leftControllerState() const noexcept { return mControllerStates[LEFT_EYE]; }
	inline const VRControllerState& rightControllerState() const noexcept { return mControllerStates[RIGHT_EYE]; }

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
	VRControllerState mControllerStates[2];
	DynArray<TrackedDevice> mTrackedDevices;
	mutable DynArray<char> mTempStrBuffer;
};

} // namespace sfz
