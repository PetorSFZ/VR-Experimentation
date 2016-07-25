#include "GameScreen.hpp"

#include "sfz/containers/StackString.hpp"
#include "sfz/gl/IncludeOpenGL.hpp"

#include "sfz/util/IO.hpp"

namespace vre {

// GameScreen: Constructors & destructors
// ------------------------------------------------------------------------------------------------

GameScreen::GameScreen(vr::IVRSystem* vrSystem) noexcept
:
	mVRSystem(vrSystem)
{
	sfz::StackString128 modelsPath;
	modelsPath.printf("%sassets/models/", sfz::basePath());
	sfz::StackString128 shadersPath;
	shadersPath.printf("%sassets/shaders/", sfz::basePath());

	mSphereModel = SimpleModel(modelsPath.str, "head_d2u_f2.obj");

	mSimpleShader = Program::fromFile(shadersPath.str, "SimpleShader.vert", "SimpleShader.frag");

	mScalingShader = Program::postProcessFromSource(R"(
		#version 330

		// Input
		in vec2 uvCoord;

		// Output
		out vec4 outFragColor;

		// Uniforms
		uniform sampler2D uLeftEyeTex;
		uniform sampler2D uRightEyeTex;
		uniform vec2 uWindowRes;
		uniform vec2 uEyeRes;
		uniform vec4 uUnusedColor = vec4(0.0, 0.0, 1.0, 1.0);
		uniform int uMode = 1; // 0 == render only left eye, 1 == render both eyes

		void main()
		{
			outFragColor = uUnusedColor;

			if (uMode == 0) {
				float windowAspect = uWindowRes.x / uWindowRes.y;
				float eyeAspect = uEyeRes.x / uEyeRes.y;

				// Window is wider than eye texture
				if (windowAspect >= eyeAspect) {
					vec2 scale = uWindowRes * uEyeRes.y / (uWindowRes.y * uEyeRes);
					vec2 offs = -vec2(max((scale.x - 1.0) * 0.5, 0.0), 0.0);
					vec2 coord = uvCoord * scale + offs;
					if (0.0 <= coord.x && coord.x <= 1.0) {
						outFragColor = texture(uLeftEyeTex, coord);
					}
				}
				// Eye texture is wider than window
				else {
					vec2 scale = uWindowRes * uEyeRes.x / (uWindowRes.x * uEyeRes);
					vec2 offs = -vec2(0.0, max((scale.y - 1.0) * 0.5, 0.0));
					vec2 coord = uvCoord * scale + offs;
					if (0.0 <= coord.y && coord.y <= 1.0) {
						outFragColor = texture(uLeftEyeTex, coord);
					}
				}
			}
			else {
				vec2 eyeRes = vec2(uEyeRes.x * 2.0, uEyeRes.y);

				float windowAspect = uWindowRes.x / uWindowRes.y;
				float eyeAspect = eyeRes.x / eyeRes.y;

				// Window is wider than eye texture
				if (windowAspect >= eyeAspect) {
					vec2 scale = uWindowRes * eyeRes.y / (uWindowRes.y * eyeRes);
					vec2 offs = -vec2(max((scale.x - 1.0) * 0.5, 0.0), 0.0);
					vec2 coord = uvCoord * scale + offs;
					if (0.0 <= coord.x && coord.x <= 1.0) {
						outFragColor = texture(coord.x < 0.5 ? uLeftEyeTex : uRightEyeTex, coord);
					}
				}
				// Eye texture is wider than window
				else {
					vec2 scale = uWindowRes * eyeRes.x / (uWindowRes.x * eyeRes);
					vec2 offs = -vec2(0.0, max((scale.y - 1.0) * 0.5, 0.0));
					vec2 coord = uvCoord * scale + offs;
					if (0.0 <= coord.y && coord.y <= 1.0) {
						outFragColor = texture(coord.x < 0.5 ? uLeftEyeTex : uRightEyeTex, coord);
					}
				}
			}
		}
	)");

	mCam = sfz::ViewFrustum(vec3(0.0f, 3.0f, -6.0f), normalize(vec3(0.0f, -0.25f, 1.0f)), normalize(vec3(0.0f, 1.0f, 0.0)), 60.0f, 1.0f, 0.01f, 100.0f);
}

// GameScreen: Overriden methods from sfz::BaseScreen
// ------------------------------------------------------------------------------------------------

UpdateOp GameScreen::update(UpdateState& state)
{
	// Handle input
	for (const SDL_Event& event : state.events) {
		switch (event.type) {
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_ESCAPE:
				return sfz::SCREEN_QUIT;
			}
			break;
		}
	}


	return sfz::SCREEN_NO_OP;
}

void GameScreen::render(UpdateState& state)
{
	using namespace sfz;

	// Update framebuffer sizes
	vec2i fbRes = ovr::getRecommendedRenderTargetSize(mVRSystem);
	if (mFinalFB[ovr::EYE_LEFT].dimensions() != fbRes) {
		FramebufferBuilder builder = FramebufferBuilder(fbRes)
		                            .addDepthTexture(FBDepthFormat::F32)
		                            .addTexture(0, FBTextureFormat::RGB_U8, FBTextureFiltering::LINEAR);
		mFinalFB[ovr::EYE_LEFT] = builder.build();
		mFinalFB[ovr::EYE_RIGHT] = builder.build();

		printf("Created framebuffers\nWindow: %s\nEye buffers: %s\n\n",
		       toString(state.window.drawableDimensions()).str,
		       toString(fbRes).str);

		mScalingShader.useProgram();
		setUniform(mScalingShader, "uWindowRes", state.window.drawableDimensionsFloat());
		setUniform(mScalingShader, "uEyeRes", vec2(fbRes));
	}

	/*const mat4 projMatrix = mCam.projMatrix();
	const mat4 viewMatrix = mCam.viewMatrix();
	const mat4 invProjMatrix = inverse(projMatrix);
	const mat4 invViewMatrix = inverse(viewMatrix);*/

	// Update head position
	mat4 headMatrix = sfz::identityMatrix4<float>();
	{
		vr::TrackedDevicePose_t trackedDevicePoses[vr::k_unMaxTrackedDeviceCount];
		char deviceChar[vr::k_unMaxTrackedDeviceCount] = {};

		vr::VRCompositor()->WaitGetPoses(trackedDevicePoses, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

		for (int device = 0; device < 16; device++) {
			if (!trackedDevicePoses[device].bPoseIsValid) continue;
			if (mVRSystem->GetTrackedDeviceClass(device) != vr::TrackedDeviceClass_HMD) continue;
			
			headMatrix = transpose(ovr::convertSteamVRMatrix(trackedDevicePoses[device].mDeviceToAbsoluteTracking));
			break;
		}
	}

	// Render to both eyes
	{
		mSimpleShader.useProgram();

		for (uint32_t eye : ovr::eyes) {
			const mat4 projMatrix = ovr::getProjectionMatrix(mVRSystem, eye, 0.01f, 100.0f);
			const mat4 eyeMatrix = ovr::getEyeMatrix(mVRSystem, eye);
			const mat4 viewMatrix = eyeMatrix * headMatrix;
			const mat4 modelMatrix = identityMatrix4<float>();

			gl::setUniform(mSimpleShader, "uProjMatrix", projMatrix);
			gl::setUniform(mSimpleShader, "uViewMatrix", viewMatrix);
			gl::setUniform(mSimpleShader, "uModelMatrix", modelMatrix);
			gl::setUniform(mSimpleShader, "uNormalMatrix", inverse(transpose(viewMatrix * modelMatrix))); // inverse(tranpose(modelViewMatrix))*/
			
			mFinalFB[eye].bindViewportClearColorDepth();

			mSphereModel.render();
		}
	}

	// Scale left eye to window
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, state.window.drawableWidth(), state.window.drawableHeight());
		glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		mScalingShader.useProgram();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mFinalFB[ovr::EYE_LEFT].texture(0));
		gl::setUniform(mScalingShader, "uLeftEyeTex", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mFinalFB[ovr::EYE_RIGHT].texture(0));
		gl::setUniform(mScalingShader, "uRightEyeTex", 1);

		mQuad.render();
	}

	// Submit framebuffers to Vive
	{
		// Submit left framebuffer
		vr::Texture_t leftEyeTex;
		leftEyeTex.handle = (void*)mFinalFB[ovr::EYE_LEFT].texture(0);
		leftEyeTex.eType = vr::API_OpenGL;
		leftEyeTex.eColorSpace = vr::ColorSpace::ColorSpace_Linear; // TODO: Change to gamma?
		vr::VRTextureBounds_t leftEyeTexBounds;
		leftEyeTexBounds.uMin = 0.0f;
		leftEyeTexBounds.vMin = 0.0f;
		leftEyeTexBounds.uMax = 1.0f;
		leftEyeTexBounds.vMax = 1.0f;
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTex, &leftEyeTexBounds);

		// Submit right framebuffer
		vr::Texture_t rightEyeTex;
		rightEyeTex.handle = (void*)mFinalFB[ovr::EYE_RIGHT].texture(0);
		rightEyeTex.eType = vr::API_OpenGL;
		rightEyeTex.eColorSpace = vr::ColorSpace::ColorSpace_Linear; // TODO: Change to gamma?
		vr::VRTextureBounds_t rightEyeTexBounds;
		rightEyeTexBounds.uMin = 0.0f;
		rightEyeTexBounds.vMin = 0.0f;
		rightEyeTexBounds.uMax = 1.0f;
		rightEyeTexBounds.vMax = 1.0f;
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTex, &rightEyeTexBounds);
	
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();

		SDL_GL_SwapWindow(state.window.ptr());

		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Flush and wait for swap.
		glFlush();
		glFinish();
	}
}

void GameScreen::onQuit()
{

}

void GameScreen::onResize(vec2 dimensions, vec2 drawableDimensions)
{
	mScalingShader.useProgram();
	setUniform(mScalingShader, "uWindowRes", drawableDimensions);
}

} // namespace vre
