#pragma once

#include "sfz/Math.hpp"
#include "sfz/Screens.hpp"
#include "sfz/SDL.hpp"

#include "sfz/gl/Program.hpp"
#include "sfz/gl/Framebuffer.hpp"

#include "sfz/geometry/ViewFrustum.hpp"

#include <openvr.h>

#include "SimpleModel.hpp"

namespace vre {

using sfz::gl::Framebuffer;
using sfz::gl::FramebufferBuilder;
using sfz::gl::FBDepthFormat;
using sfz::gl::FBTextureFiltering;
using sfz::gl::FBTextureFormat;
using sfz::gl::Program;
using sfz::sdl::Window;
using sfz::UpdateOp;
using sfz::UpdateState;
using sfz::vec2;
using sfz::vec3;
using sfz::mat4;

class GameScreen final : public sfz::BaseScreen {
public:

	// Constructors & destructors
	// --------------------------------------------------------------------------------------------

	GameScreen(vr::IVRSystem* vrSystem) noexcept;

	// Overriden methods from sfz::BaseScreen
	// --------------------------------------------------------------------------------------------

	virtual UpdateOp update(UpdateState& state) override final;
	virtual void render(UpdateState& state) override final;
	virtual void onQuit() override final;
	virtual void onResize(vec2 dimensions, vec2 drawableDimensions) override final;

private:
	// Private members
	// --------------------------------------------------------------------------------------------

	Program mSimpleShader;
	SimpleModel mSphereModel;
	sfz::ViewFrustum mCam;

	vr::IVRSystem* mVRSystem = nullptr;
};

} // namespace vre
