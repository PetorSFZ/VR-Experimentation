#pragma once

#include "sfz/Math.hpp"
#include "sfz/Screens.hpp"
#include "sfz/SDL.hpp"

namespace vre {

using sfz::sdl::Window;
using sfz::UpdateOp;
using sfz::UpdateState;
using sfz::vec2;

class GameScreen final : public sfz::BaseScreen {
public:

	// Constructors & destructors
	// --------------------------------------------------------------------------------------------

	GameScreen() noexcept;

	// Overriden methods from sfz::BaseScreen
	// --------------------------------------------------------------------------------------------

	virtual UpdateOp update(UpdateState& state) override final;
	virtual void render(UpdateState& state) override final;
	virtual void onQuit() override final;
	virtual void onResize(vec2 dimensions, vec2 drawableDimensions) override final;

private:
	// Private members
	// --------------------------------------------------------------------------------------------

	
};

} // namespace vre
