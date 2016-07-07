#include "GameScreen.hpp"

#include "sfz/GL.hpp"
#include "sfz/gl/IncludeOpenGL.hpp"
#include "sfz/Screens.hpp"
#include "sfz/SDL.hpp"

using namespace sfz;
using namespace gl;
using namespace sdl;

int main(int argc, char* argv[])
{
	// Small hack to enable hi-dpi awareness on Windows.
#ifdef _WIN32
	SetProcessDPIAware();
#endif

	Session sdlSession({SDLInitFlags::EVERYTHING}, {});
	Window window("VR-Experimentation", 1920, 1080, {WindowFlags::OPENGL, WindowFlags::RESIZABLE, WindowFlags::ALLOW_HIGHDPI});

	// OpenGL context
	const int MAJOR_VERSION = 4;
	const int MINOR_VERSION = 1;
#if !defined(SFZ_NO_DEBUG)
#ifdef _WIN32
	gl::Context glContext{window.ptr(), MAJOR_VERSION, MINOR_VERSION, gl::GLContextProfile::COMPATIBILITY, true};
#else
	gl::Context glContext{window.ptr(), MAJOR_VERSION, MINOR_VERSION, gl::GLContextProfile::CORE, true};
#endif
#else
#ifdef _WIN32
	gl::Context glContext{window.ptr(), MAJOR_VERSION, MINOR_VERSION, gl::GLContextProfile::COMPATIBILITY, false};
#else
	gl::Context glContext{window.ptr(), MAJOR_VERSION, MINOR_VERSION, gl::GLContextProfile::CORE, false};
#endif
#endif

	// Initializes GLEW, must happen after GL context is created.
	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		sfz::error("GLEW init failure: %s", glewGetErrorString(glewError));
	}

	gl::printSystemGLInfo();

	// Fullscreen & VSync
	window.setVSync(VSync::OFF);
	//window.setFullscreen(Fullscreen::WINDOWED, 0);

	// Enable OpenGL debug message if in debug mode
#if !defined(SFZ_NO_DEBUG)
	gl::setupDebugMessages(gl::Severity::MEDIUM, gl::Severity::MEDIUM);
#endif

	sfz::runGameLoop(window, SharedPtr<BaseScreen>(sfz_new<vre::GameScreen>()));

	return 0;
}