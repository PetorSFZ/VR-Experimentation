#include "GameScreen.hpp"

#include "sfz/containers/StackString.hpp"
#include "sfz/gl/IncludeOpenGL.hpp"

#include "sfz/util/IO.hpp"

namespace vre {

// GameScreen: Constructors & destructors
// ------------------------------------------------------------------------------------------------

GameScreen::GameScreen() noexcept
:
	mStats(128)
{
	sfz::StackString128 modelsPath;
	modelsPath.printf("%sassets/models/", sfz::basePath());
	sfz::StackString128 shadersPath;
	shadersPath.printf("%sassets/shaders/", sfz::basePath());

	mSnakeModel = sfz::gl::tinyObjLoadModel(modelsPath.str, "head_d2u_f2.obj");

	mSimpleShader = Program::fromFile(shadersPath.str, "SimpleShader.vert", "SimpleShader.frag",
	[](uint32_t shaderProgram) {
		glBindAttribLocation(shaderProgram, 0, "inPosition");
		glBindAttribLocation(shaderProgram, 1, "inNormal");
		glBindAttribLocation(shaderProgram, 2, "inUV");
	});

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
	mStats.addSample(state.delta);
	static int printCount = 0;
	if (printCount == 0) printf("%s\n", mStats.toString());
	printCount = (printCount + 1) % 20;

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

	// Update vr instance
	sfz::VR::instance().update();

	return sfz::SCREEN_NO_OP;
}

void GameScreen::render(UpdateState& state)
{
	using namespace sfz;

	// Grab vr instance
	VR& vr = VR::instance();
	const HMD& hmd = vr.hmd();

	// Check if framebuffers need to be resized
	vec2i fbRes = vr.recommendedRenderTargetSize();
	if (mFinalFB[LEFT_EYE].dimensions() != fbRes) {
		FramebufferBuilder builder = FramebufferBuilder(fbRes)
		                            .addDepthTexture(FBDepthFormat::F32)
		                            .addTexture(0, FBTextureFormat::RGB_U8, FBTextureFiltering::LINEAR);
		mFinalFB[LEFT_EYE] = builder.build();
		mFinalFB[RIGHT_EYE] = builder.build();

		printf("Created framebuffers\nWindow: %s\nEye buffers: %s\n\n",
		       toString(state.window.drawableDimensions()).str,
		       toString(fbRes).str);

		mScalingShader.useProgram();
		setUniform(mScalingShader, "uWindowRes", state.window.drawableDimensionsFloat());
		setUniform(mScalingShader, "uEyeRes", vec2(fbRes));
	}

	// Render to both eyes
	{
		mSimpleShader.useProgram();

		glEnable(GL_CULL_FACE);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		for (uint32_t eye : VR_EYES) {
			const mat4 viewMatrix = hmd.eyeMatrix[eye] * hmd.headMatrix;
			const mat4 modelMatrix = identityMatrix4<float>();

			gl::setUniform(mSimpleShader, "uProjMatrix", hmd.projMatrix[eye]);
			gl::setUniform(mSimpleShader, "uViewMatrix", viewMatrix);
			gl::setUniform(mSimpleShader, "uModelMatrix", modelMatrix);
			gl::setUniform(mSimpleShader, "uNormalMatrix", inverse(transpose(viewMatrix * modelMatrix))); // inverse(tranpose(modelViewMatrix))*/
			
			mFinalFB[eye].bindViewportClearColorDepth();
			
			mSnakeModel.draw();
			
			mat4 controllerTransform0 = vr.controller(0).transform;
			gl::setUniform(mSimpleShader, "uModelMatrix", controllerTransform0);
			gl::setUniform(mSimpleShader, "uNormalMatrix", inverse(transpose(viewMatrix * controllerTransform0))); // inverse(tranpose(modelViewMatrix))*/
			vr.controllerModel(0).draw();

			mat4 controllerTransform1 = vr.controller(1).transform;
			gl::setUniform(mSimpleShader, "uModelMatrix", controllerTransform1);
			gl::setUniform(mSimpleShader, "uNormalMatrix", inverse(transpose(viewMatrix * controllerTransform1))); // inverse(tranpose(modelViewMatrix))*/
			vr.controllerModel(1).draw();
		}
	}

	// Copy both eye textures to window
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, state.window.drawableWidth(), state.window.drawableHeight());
		glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
		glClearDepth(1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		mScalingShader.useProgram();

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, mFinalFB[LEFT_EYE].texture(0));
		gl::setUniform(mScalingShader, "uLeftEyeTex", 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, mFinalFB[RIGHT_EYE].texture(0));
		gl::setUniform(mScalingShader, "uRightEyeTex", 1);

		mQuad.render();
	}

	// Submit framebuffers to Vive
	vr.submit(state.window.ptr(), mFinalFB[LEFT_EYE].texture(0), mFinalFB[RIGHT_EYE].texture(0));

	// Write every 4th frame to the window
	static int everyOther = 0;
	if (everyOther == 0) {
		SDL_GL_SwapWindow(state.window.ptr());
	}
	everyOther = (everyOther + 1) % 4;
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
