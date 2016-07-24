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
	vec2i fbRes = state.window.drawableDimensions();
	if (mFinalFB[LEFT_EYE].dimensions() != fbRes) {
		FramebufferBuilder builder = FramebufferBuilder(fbRes)
		                            .addDepthTexture(FBDepthFormat::F32)
		                            .addTexture(0, FBTextureFormat::RGB_U8, FBTextureFiltering::LINEAR);
		mFinalFB[LEFT_EYE] = builder.build();
		mFinalFB[RIGHT_EYE] = builder.build();
	}

	const mat4 projMatrix = mCam.projMatrix();
	const mat4 viewMatrix = mCam.viewMatrix();
	const mat4 invProjMatrix = inverse(projMatrix);
	const mat4 invViewMatrix = inverse(viewMatrix);

	// ???
	{
		vr::TrackedDevicePose_t trackedDevicePoses[16];
		vr::TrackedDevicePose_t gamePoses[16];
		vr::VRCompositor()->WaitGetPoses(trackedDevicePoses, 16, gamePoses, 16);
	}

	// Render to both eyes
	{
		glUseProgram(mSimpleShader.handle());
		gl::setUniform(mSimpleShader, "uProjMatrix", projMatrix);
		gl::setUniform(mSimpleShader, "uViewMatrix", viewMatrix);
		gl::setUniform(mSimpleShader, "uModelMatrix", identityMatrix4<float>());
		gl::setUniform(mSimpleShader, "uNormalMatrix", inverse(transpose(viewMatrix))); // inverse(tranpose(modelViewMatrix))

		for (size_t eye = LEFT_EYE; eye <= RIGHT_EYE; eye++) {
			
			glBindFramebuffer(GL_FRAMEBUFFER, mFinalFB[eye].fbo());
			glViewport(0, 0, mFinalFB[eye].width(), mFinalFB[eye].height());
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClearDepth(1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			mSphereModel.render();
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Submit framebuffers to Vive
	{
		// Submit left framebuffer
		vr::Texture_t leftEyeTex;
		leftEyeTex.handle = (void*)mFinalFB[LEFT_EYE].texture(0);
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
		rightEyeTex.handle = (void*)mFinalFB[RIGHT_EYE].texture(0);
		rightEyeTex.eType = vr::API_OpenGL;
		rightEyeTex.eColorSpace = vr::ColorSpace::ColorSpace_Linear; // TODO: Change to gamma?
		vr::VRTextureBounds_t rightEyeTexBounds;
		rightEyeTexBounds.uMin = 0.0f;
		rightEyeTexBounds.vMin = 0.0f;
		rightEyeTexBounds.uMax = 1.0f;
		rightEyeTexBounds.vMax = 1.0f;
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTex, &rightEyeTexBounds);

		// Signal driver to start processing
		//glFlush();

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


	//SDL_GL_SwapWindow(state.window.ptr());

	/*  OpenVR sample

	// for now as fast as possible
	if ( m_pHMD )
	{
		DrawControllers();
		RenderStereoTargets();
		RenderDistortion();

		vr::Texture_t leftEyeTexture = {(void*)leftEyeDesc.m_nResolveTextureId, vr::API_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture );
		vr::Texture_t rightEyeTexture = {(void*)rightEyeDesc.m_nResolveTextureId, vr::API_OpenGL, vr::ColorSpace_Gamma };
		vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture );
	}

	if ( m_bVblank && m_bGlFinishHack )
	{
		//$ HACKHACK. From gpuview profiling, it looks like there is a bug where two renders and a present
		// happen right before and after the vsync causing all kinds of jittering issues. This glFinish()
		// appears to clear that up. Temporary fix while I try to get nvidia to investigate this problem.
		// 1/29/2014 mikesart
		glFinish();
	}

	// SwapWindow
	{
		SDL_GL_SwapWindow( m_pWindow );
	}

	// Clear
	{
		// We want to make sure the glFinish waits for the entire present to complete, not just the submission
		// of the command. So, we do a clear here right here so the glFinish will wait fully for the swap.
		glClearColor( 0, 0, 0, 1 );
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	}

	// Flush and wait for swap.
	if ( m_bVblank )
	{
		glFlush();
		glFinish();
	}

	// Spew out the controller and pose count whenever they change.
	if ( m_iTrackedControllerCount != m_iTrackedControllerCount_Last || m_iValidPoseCount != m_iValidPoseCount_Last )
	{
		m_iValidPoseCount_Last = m_iValidPoseCount;
		m_iTrackedControllerCount_Last = m_iTrackedControllerCount;
		
		dprintf( "PoseCount:%d(%s) Controllers:%d\n", m_iValidPoseCount, m_strPoseClasses.c_str(), m_iTrackedControllerCount );
	}

	UpdateHMDMatrixPose();
	*/
}

void GameScreen::onQuit()
{

}

void GameScreen::onResize(vec2 dimensions, vec2 drawableDimensions)
{

}

} // namespace vre
