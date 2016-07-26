// Copyright (c) Peter Hillerström (skipifzero.com, peter@hstroem.se)

#include "VR.hpp"

#include <openvr.h>
#include <SDL.h>

#include "sfz/Assert.hpp"
#include "sfz/gl/IncludeOpenGL.hpp"

namespace sfz {

// Statics
// ------------------------------------------------------------------------------------------------

static vr::IVRSystem* vrCast(void* ptr) noexcept
{
	return static_cast<vr::IVRSystem*>(ptr);
}

static const vr::IVRSystem* vrCast(const void* ptr) noexcept
{
	return static_cast<const vr::IVRSystem*>(ptr);
}

static mat4 convertSteamVRMatrix(const vr::HmdMatrix34_t& matrix) noexcept
{
	return mat4({
		{matrix.m[0][0], matrix.m[0][1], matrix.m[0][2], matrix.m[0][3]},
		{matrix.m[1][0], matrix.m[1][1], matrix.m[1][2], matrix.m[1][3]},
		{matrix.m[2][0], matrix.m[2][1], matrix.m[2][2], matrix.m[2][3]},
		{0.0f, 0.0f, 0.0f, 1.0f}
	});
}

static mat4 convertSteamVRMatrix(const vr::HmdMatrix44_t& matrix) noexcept
{
	return mat4({
		{matrix.m[0][0], matrix.m[0][1], matrix.m[0][2], matrix.m[0][3]},
		{matrix.m[1][0], matrix.m[1][1], matrix.m[1][2], matrix.m[1][3]},
		{matrix.m[2][0], matrix.m[2][1], matrix.m[2][2], matrix.m[2][3]},
		{matrix.m[3][0], matrix.m[3][1], matrix.m[3][2], matrix.m[3][3]}
	});
}

static mat4 getEyeMatrix(vr::IVRSystem* vrSystem, uint32_t eye) noexcept
{
	vr::HmdMatrix34_t mat = vrSystem->GetEyeToHeadTransform(vr::EVREye(eye));
	return inverse(convertSteamVRMatrix(mat));
}

static mat4 getProjectionMatrix(vr::IVRSystem* vrSystem, uint32_t eye, float near, float far) noexcept
{
	vr::HmdMatrix44_t mat = vrSystem->GetProjectionMatrix(vr::EVREye(eye), near, far, vr::API_OpenGL);
	return convertSteamVRMatrix(mat);
}

// VR: Singleton instance
// ------------------------------------------------------------------------------------------------

VR& VR::instance() noexcept
{
	static VR vr{};
	return vr;
}

// VR: Public methods
// ------------------------------------------------------------------------------------------------

bool VR::isHmdPresent() noexcept
{
	return vr::VR_IsHmdPresent();
}

bool VR::isRuntimeInstalled() noexcept
{
	return vr::VR_IsRuntimeInstalled();
}

bool VR::initialize() noexcept
{
	vr::EVRInitError error = vr::VRInitError_None;
	vr::IVRSystem* system = vr::VR_Init(&error, vr::VRApplication_Scene);
	if (error != vr::VRInitError_None) {
		printErrorMessage("VR: Failed to initialize runtime: %s",
		                  vr::VR_GetVRInitErrorAsEnglishDescription(error));
		return false;
	}

	vr::IVRRenderModels* renderModels = (vr::IVRRenderModels*)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &error);
	if (!renderModels) {
		vr::VR_Shutdown();
		mSystemPtr = nullptr;
		printErrorMessage("VR: Unable to get render model interface: %s",
		                  vr::VR_GetVRInitErrorAsEnglishDescription(error));
		return false;
	}

	// Ensure compositor is initialized
	if (!vr::VRCompositor()) {
		vr::VR_Shutdown();
		mSystemPtr = nullptr;
		printErrorMessage("VR: Failed to initialize compositor.");
		return false;
	}

	mSystemPtr = system;
	return true;
}

void VR::deinitialize() noexcept
{
	if (this->isInitialized()) {
		vr::VR_Shutdown();
		this->mSystemPtr = nullptr;
	}
}

void VR::update() noexcept
{
	vr::IVRSystem* system = vrCast(mSystemPtr);
	if (system == nullptr) {
		sfz::printErrorMessage("VR: OpenVR not initialized.");
		return;
	}

	// Update head position
	mat4 headMatrix = sfz::identityMatrix4<float>();
	{
		vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
		char deviceChar[vr::k_unMaxTrackedDeviceCount] ={};

		vr::VRCompositor()->WaitGetPoses(trackedDevicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

		for (int device = 0; device < 16; device++) {
			if (!trackedDevicePoses[device].bPoseIsValid) continue;
			if (system->GetTrackedDeviceClass(device) != vr::TrackedDeviceClass_HMD) continue;

			mHMD.headMatrix = inverse(convertSteamVRMatrix(trackedDevicePoses[device].mDeviceToAbsoluteTracking));
			break;
		}
	}

	// Update eye and projection matrices
	mHMD.eyeMatrix[LEFT_EYE] = getEyeMatrix(system, LEFT_EYE);
	mHMD.eyeMatrix[RIGHT_EYE] = getEyeMatrix(system, RIGHT_EYE);
	mHMD.projMatrix[LEFT_EYE] = getProjectionMatrix(system, LEFT_EYE, mHMD.near, 1000.0f);
	mHMD.projMatrix[RIGHT_EYE] = getProjectionMatrix(system, RIGHT_EYE, mHMD.near, 1000.0f);
}

void VR::submitAndSwap(void* sdlWindowPtr, uint32_t leftEyeTex, uint32_t rightEyeTex,
                       vec2 uvMax, bool gammaCorrect) noexcept
{
	vr::IVRSystem* system = vrCast(mSystemPtr);
	if (system == nullptr) {
		sfz::printErrorMessage("VR: OpenVR not initialized.");
		return;
	}

	// Select color space for textures based on whether they are gamma corrected or not
	vr::ColorSpace colorSpace = gammaCorrect ? vr::ColorSpace::ColorSpace_Gamma
	                                         : vr::ColorSpace::ColorSpace_Linear;

	// TODO: Fix bounds depending on the given uvMax

	// Submit left framebuffer
	vr::Texture_t leftEyeTexWrapper;
	leftEyeTexWrapper.handle = (void*)leftEyeTex;
	leftEyeTexWrapper.eType = vr::API_OpenGL;
	leftEyeTexWrapper.eColorSpace = colorSpace;
	vr::VRTextureBounds_t leftEyeTexBounds;
	leftEyeTexBounds.uMin = 0.0f;
	leftEyeTexBounds.vMin = 0.0f;
	leftEyeTexBounds.uMax = 1.0f;
	leftEyeTexBounds.vMax = 1.0f;
	vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexWrapper, &leftEyeTexBounds);

	// Submit right framebuffer
	vr::Texture_t rightEyeTexWrapper;
	rightEyeTexWrapper.handle = (void*)rightEyeTex;
	rightEyeTexWrapper.eType = vr::API_OpenGL;
	rightEyeTexWrapper.eColorSpace = colorSpace;
	vr::VRTextureBounds_t rightEyeTexBounds;
	rightEyeTexBounds.uMin = 0.0f;
	rightEyeTexBounds.vMin = 0.0f;
	rightEyeTexBounds.uMax = 1.0f;
	rightEyeTexBounds.vMax = 1.0f;
	vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexWrapper, &rightEyeTexBounds);
	
	//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
	// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
	// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
	// 1/29/2014 mikesart
	glFinish();

	SDL_GL_SwapWindow(static_cast<SDL_Window*>(sdlWindowPtr));

	// We want to make sure the glFinish waits for the entire present to complete, not just the submission
	// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Flush and wait for swap.
	glFlush();
	glFinish();
}

// VR: Getters
// ------------------------------------------------------------------------------------------------

vec2i VR::getRecommendedRenderTargetSize() noexcept
{
	vr::IVRSystem* system = vrCast(mSystemPtr);
	if (system == nullptr) {
		sfz::printErrorMessage("VR: OpenVR not initialized.");
		return vec2i(-1);
	}
	uint32_t w = 0, h = 0;
	system->GetRecommendedRenderTargetSize(&w, &h);
	return vec2i(int32_t(w), int32_t(h));
}

// VR: Private constructors & destructors
// ------------------------------------------------------------------------------------------------

VR::VR() noexcept
{

}

VR::~VR() noexcept
{
	this->deinitialize();
}

} // namespace sfz
