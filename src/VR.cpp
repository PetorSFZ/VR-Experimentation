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

static Model loadVRModel(const char* deviceName) noexcept
{
	using sfz::gl::Vertex;

	// Load model
	vr::RenderModel_t* modelPtr = nullptr;
	vr::EVRRenderModelError error = vr::VRRenderModelError_None;
	while (true) {
		error = vr::VRRenderModels()->LoadRenderModel_Async(deviceName, &modelPtr);
		if (error != vr::VRRenderModelError_Loading) {
			break;
		}
		SDL_Delay(1);
	}
	if (error != vr::VRRenderModelError_None) {
		printErrorMessage("VR: Failed to load model: %s", deviceName);
		return Model();
	}

	// Load texture
	vr::RenderModel_TextureMap_t* texturePtr = nullptr;
	while (true) {
		error = vr::VRRenderModels()->LoadTexture_Async(modelPtr->diffuseTextureId, &texturePtr);
		if (error != vr::VRRenderModelError_Loading) {
			break;
		}
		SDL_Delay(1);
	}
	if (error != vr::VRRenderModelError_None) {
		printErrorMessage("VR: Failed to load texture: %s", deviceName);
		vr::VRRenderModels()->FreeRenderModel(modelPtr);
		return Model();
	}


	Model tmp;

	// Copy over vertices
	tmp.vertices = DynArray<Vertex>(modelPtr->unVertexCount, modelPtr->unVertexCount);
	for (size_t i = 0; i < tmp.vertices.size(); i++) {
		sfz::gl::Vertex vertTmp;
		vertTmp.pos = vec3(modelPtr->rVertexData[i].vPosition.v);
		vertTmp.normal = vec3(modelPtr->rVertexData[i].vNormal.v);
		vertTmp.uv = vec2(modelPtr->rVertexData[i].rfTextureCoord);
		tmp.vertices[i] = vertTmp;
	}

	// Copy over indices
	tmp.indices = DynArray<uint32_t>(modelPtr->unTriangleCount * 3, 0, modelPtr->unTriangleCount * 3);
	for (size_t i = 0; i < tmp.indices.size(); i++) {
		tmp.indices[i] = modelPtr->rIndexData[i];
	}

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

	// Create OpenGL texture
	glGenTextures(1, &tmp.glColorTexture);
	glBindTexture(GL_TEXTURE_2D, tmp.glColorTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texturePtr->unWidth, texturePtr->unHeight, 0, GL_RGBA,
	             GL_UNSIGNED_BYTE, texturePtr->rubTextureMapData);

	// Set filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	float largestAnisotropy;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largestAnisotropy);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largestAnisotropy);

	// Generate mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);

	// Cleanup
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	vr::VRRenderModels()->FreeRenderModel(modelPtr);
	vr::VRRenderModels()->FreeTexture(texturePtr);
	
	return std::move(tmp);
}

static TrackedDevice initTrackedDevice(vr::IVRSystem* system, uint32_t deviceIndex,
                                       vr::TrackedDevicePose_t& pose, bool& valid) noexcept
{
	static DynArray<char> tmpStrBuffer;

	TrackedDevice tmp;

	// Add device if it has valid type
	vr::ETrackedDeviceClass deviceClass = system->GetTrackedDeviceClass(deviceIndex);
	if (deviceClass == vr::TrackedDeviceClass_HMD) {
		tmp.type = TrackedDeviceType::HMD;
	} else if (deviceClass == vr::TrackedDeviceClass_Controller) {
		tmp.type = TrackedDeviceType::CONTROLLER;
	} else if (deviceClass == vr::TrackedDeviceClass_TrackingReference) {
		tmp.type = TrackedDeviceType::TRACKING_REFERENCE;
	} else {
		valid = false;
		return tmp;
	}

	// Set device id
	tmp.deviceId = deviceIndex;

	// Transform
	tmp.transform = convertSteamVRMatrix(pose.mDeviceToAbsoluteTracking);

	// Retrieve name of device
	uint32_t nameLen = system->GetStringTrackedDeviceProperty(deviceIndex,
	                               vr::Prop_RenderModelName_String,NULL, 0);
	tmpStrBuffer.ensureCapacity(nameLen + 1);
	tmpStrBuffer.setSize(nameLen + 1);
	system->GetStringTrackedDeviceProperty(deviceIndex, vr::Prop_RenderModelName_String,
	                                       tmpStrBuffer.data(), tmpStrBuffer.capacity());

	// Load model
	tmp.model = loadVRModel(tmpStrBuffer.data());

	valid = true;
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

	// Load initial devices
	{
		// Get poses
		vr::TrackedDevicePose_t devicePoses[vr::k_unMaxTrackedDeviceCount];
		vr::VRCompositor()->WaitGetPoses(devicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);
		
		// Iterate through devices
		for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
			
			// Skip invalid poses
			if (!devicePoses[i].bPoseIsValid) continue;
			
			bool valid = false;
			TrackedDevice tmp = initTrackedDevice(system, i, devicePoses[i], valid);
			if (!valid) continue;

			mTrackedDevices.add(std::move(tmp));
		}
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

	// Process SteamVR events
	vr::VREvent_t event;
	while (system->PollNextEvent(&event, sizeof(event))) {
		switch (event.eventType) {
		case vr::VREvent_TrackedDeviceActivated:
			{
				bool valid = false;
				TrackedDevice tmp = initTrackedDevice(system, event.trackedDeviceIndex,
				                                      devicePoses[event.trackedDeviceIndex], valid);
				if (!valid) continue;
				mTrackedDevices.add(std::move(tmp));
			}
			break;
		case vr::VREvent_TrackedDeviceDeactivated:
			for (uint32_t i = 0; i < mTrackedDevices.size(); i++) {
				if (mTrackedDevices[i].deviceId == event.trackedDeviceIndex) {
					mTrackedDevices.remove(i);
					break;
				}
			}
			break;
		case vr::VREvent_TrackedDeviceUpdated:
			// TODO: Do what?
			break;

		default:
			// Do nothing
			break;
		}
	}

	// Update transforms of all tracked devices
	for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; i++) {
		if (!deviceActive[i]) continue;

		// Find index in internal array
		uint32_t arrayIndex = uint32_t(~0);
		for (uint32_t j = 0; j < mTrackedDevices.size(); j++) {
			if (mTrackedDevices[j].deviceId == i) {
				arrayIndex = j;
				break;
			}
		}

		if (arrayIndex == uint32_t(~0)) {
			printErrorMessage("VR: Can't update tracked device because it does not exist. This should not happen.");
			sfz_assert_debug(false);
			// TODO: Device does not exist, need to create it
			continue;
		}

		// Update device location
		mTrackedDevices[arrayIndex].transform = convertSteamVRMatrix(devicePoses[i].mDeviceToAbsoluteTracking);
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

// VR: Getters
// ------------------------------------------------------------------------------------------------

vec2i VR::recommendedRenderTargetSize() const noexcept
{
	vr::IVRSystem* system = vrCast(mSystemPtr);
	if (system == nullptr) {
		sfz::printErrorMessage("VR: OpenVR not initialized.");
		return vec2i(0.0f);
	}

	uint32_t w = 0, h = 0;
	system->GetRecommendedRenderTargetSize(&w, &h);
	return vec2i(int32_t(w), int32_t(h));
}

mat4 VR::headMatrix() const noexcept
{
	vr::IVRSystem* system = vrCast(mSystemPtr);
	if (system == nullptr) {
		sfz::printErrorMessage("VR: OpenVR not initialized.");
		return identityMatrix4<float>();
	}

	for (const TrackedDevice& device : mTrackedDevices) {
		if (device.type == TrackedDeviceType::HMD) {
			return inverse(device.transform);
		}
	}
	
	return identityMatrix4<float>();
}

mat4 VR::eyeMatrix(uint32_t eye) const noexcept
{
	vr::IVRSystem* system = vrCast(mSystemPtr);
	if (system == nullptr) {
		sfz::printErrorMessage("VR: OpenVR not initialized.");
		return identityMatrix4<float>();
	}

	vr::HmdMatrix34_t mat = system->GetEyeToHeadTransform(vr::EVREye(eye));
	return inverse(convertSteamVRMatrix(mat));
}

mat4 VR::projMatrix(uint32_t eye, float near) const noexcept
{
	vr::IVRSystem* system = vrCast(mSystemPtr);
	if (system == nullptr) {
		sfz::printErrorMessage("VR: OpenVR not initialized.");
		return identityMatrix4<float>();
	}

	vr::HmdMatrix44_t mat = system->GetProjectionMatrix(vr::EVREye(eye), near, 1000.0f, vr::API_OpenGL);
	return convertSteamVRMatrix(mat);
}

const TrackedDevice* VR::hmd() const noexcept
{
	vr::IVRSystem* system = vrCast(mSystemPtr);
	if (system == nullptr) {
		sfz::printErrorMessage("VR: OpenVR not initialized.");
		return nullptr;
	}

	for (const TrackedDevice& device : mTrackedDevices) {
		if (device.type == TrackedDeviceType::HMD) {
			return &device;
		}
	}
	return nullptr;
}

const TrackedDevice* VR::leftController() const noexcept
{
	vr::IVRSystem* system = vrCast(mSystemPtr);
	if (system == nullptr) {
		sfz::printErrorMessage("VR: OpenVR not initialized.");
		return nullptr;
	}

	uint32_t id = system->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_LeftHand);
	for (const auto& device : mTrackedDevices) {
		if (device.deviceId == id) return &device;
	}
	return nullptr;
}

const TrackedDevice* VR::rightController() const noexcept
{
	vr::IVRSystem* system = vrCast(mSystemPtr);
	if (system == nullptr) {
		sfz::printErrorMessage("VR: OpenVR not initialized.");
		return nullptr;
	}

	uint32_t id = system->GetTrackedDeviceIndexForControllerRole(vr::TrackedControllerRole_RightHand);
	for (const auto& device : mTrackedDevices) {
		if (device.deviceId == id) return &device;
	}
	return nullptr;
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
