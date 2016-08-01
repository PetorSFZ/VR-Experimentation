// Copyright (c) Peter Hillerström (skipifzero.com, peter@hstroem.se)

#include "VR.hpp"

#include <openvr.h>
#include <SDL.h>

#include "sfz/Assert.hpp"
#include "sfz/containers/DynString.hpp"
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

static Model modelFromRenderModel(vr::RenderModel_t* renderModelPtr) noexcept
{
	using sfz::gl::Vertex;
	Model tmp;
	
	// Copy over vertices
	tmp.vertices = DynArray<Vertex>(renderModelPtr->unVertexCount, renderModelPtr->unVertexCount);
	for (size_t i = 0; i < tmp.vertices.size(); i++) {
		sfz::gl::Vertex vertTmp;
		vertTmp.pos = vec3(renderModelPtr->rVertexData[i].vPosition.v);
		vertTmp.normal = vec3(renderModelPtr->rVertexData[i].vNormal.v);
		vertTmp.uv = vec2(renderModelPtr->rVertexData[i].rfTextureCoord);
		tmp.vertices[i] = vertTmp;
	}

	// Copy over indices
	tmp.indices = DynArray<uint32_t>(renderModelPtr->unTriangleCount * 3, 0, renderModelPtr->unTriangleCount * 3);
	for (size_t i = 0; i < tmp.indices.size(); i++) {
		tmp.indices[i] = renderModelPtr->rIndexData[i];
	}

	// TODO: Copy texture
	tmp.glColorTexture = renderModelPtr->diffuseTextureId;


	// Create Vertex Array object
	glGenVertexArrays(1, &tmp.glVAO);
	glBindVertexArray(tmp.glVAO);

	// Create and fill vertex buffer
	glGenBuffers(1, &tmp.glVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, tmp.glVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, tmp.vertices.size() * sizeof(Vertex), tmp.vertices.data(), GL_STATIC_DRAW);

	// Locate components in vertex buffer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, pos));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	// Create and fill index buffer
	glGenBuffers(1, &tmp.glIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmp.glIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * tmp.indices.size(), tmp.indices.data(), GL_STATIC_DRAW);

	// Cleanup
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return std::move(tmp);
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

	// Get recommended render target size
	{
		uint32_t w = 0, h = 0;
		system->GetRecommendedRenderTargetSize(&w, &h);
		mRecommendedRenderTargetSize = vec2i(int32_t(w), int32_t(h));
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

	// Retrieve positions of all currently tracked devices
	bool deviceActive[vr::k_unMaxTrackedDeviceCount];
	vr::ETrackedDeviceClass deviceClass[vr::k_unMaxTrackedDeviceCount];
	vr::TrackedDevicePose_t devicePoses[vr::k_unMaxTrackedDeviceCount];
	{
		vr::VRCompositor()->WaitGetPoses(devicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
		for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
			deviceActive[i] = devicePoses[i].bPoseIsValid;
			deviceClass[i] = system->GetTrackedDeviceClass(i);
		}
	}

	// Update head matrix
	for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		if (deviceActive[i] && deviceClass[i] == vr::TrackedDeviceClass_HMD) {
			mHMD.headMatrix = inverse(convertSteamVRMatrix(devicePoses[i].mDeviceToAbsoluteTracking));
			break;
		}
	}

	// Update eye and projection matrices
	mHMD.eyeMatrix[LEFT_EYE] = getEyeMatrix(system, LEFT_EYE);
	mHMD.eyeMatrix[RIGHT_EYE] = getEyeMatrix(system, RIGHT_EYE);
	mHMD.projMatrix[LEFT_EYE] = getProjectionMatrix(system, LEFT_EYE, mHMD.near, 1000.0f);
	mHMD.projMatrix[RIGHT_EYE] = getProjectionMatrix(system, RIGHT_EYE, mHMD.near, 1000.0f);

	
	int controllerCount = 0;
	for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		if (!deviceActive[i] || deviceClass[i] != vr::TrackedDeviceClass_Controller) continue;

		// Retrieve name of device
		uint32_t nameLen = system->GetStringTrackedDeviceProperty(i, vr::Prop_RenderModelName_String, NULL, 0);
		mTempStrBuffer.ensureCapacity(nameLen + 1);
		mTempStrBuffer.setSize(nameLen + 1);
		system->GetStringTrackedDeviceProperty(i, vr::Prop_RenderModelName_String,
			mTempStrBuffer.data(), mTempStrBuffer.capacity());

		// Load model
		vr::RenderModel_t* modelPtr;
		vr::EVRRenderModelError error;
		while (true) {
			error = vr::VRRenderModels()->LoadRenderModel_Async(mTempStrBuffer.data(), &modelPtr);
			if (error != vr::VRRenderModelError_Loading) {
				break;
			}
			SDL_Delay(1);
		}

		mControllerModels[controllerCount] = modelFromRenderModel(modelPtr);
		// TODO: Free modelPtr (with FreeRenderModel())?

		//mControllers[controllerCount].transform = inverse(convertSteamVRMatrix(devicePoses[i].mDeviceToAbsoluteTracking));
		mControllers[controllerCount].transform = convertSteamVRMatrix(devicePoses[i].mDeviceToAbsoluteTracking);

		controllerCount += 1;
		if (controllerCount == 2) break;
	}
}

void VR::submit(void* sdlWindowPtr, uint32_t leftEyeTex, uint32_t rightEyeTex,
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
	
	// Flush and wait for swap.
	glFlush();
	glFinish();

	// Make compositor begin work immediately (don't wait for WaitGetPoses())
	vr::VRCompositor()->PostPresentHandoff();
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
