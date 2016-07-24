#include "GameScreen.hpp"

#include <openvr.h>

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


	// New experimental OpenVR stuff

	// Check if HMD is present
	if (vr::VR_IsHmdPresent()) {
		printf("HMD present\n");
	} else {
		printf("HMD NOT present\n");
		return 0;
	}

	// Load SteamVR Runtime
	vr::EVRInitError vrError = vr::VRInitError_None;
	vr::IVRSystem* vrSystem = vr::VR_Init(&vrError, vr::VRApplication_Scene);
	if (vrError != vr::VRInitError_None) {
		printf("Failed to initialize VR runtime: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(vrError));
		system("pause");
		return 0;
	}

	vr::IVRRenderModels* renderModels = (vr::IVRRenderModels*) vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &vrError);
	if (!renderModels) {
		vr::VR_Shutdown();
		printf("unable to get render model interface: %s\n", vr::VR_GetVRInitErrorAsEnglishDescription(vrError));
		system("pause");
		return 0;
	}

	// Try to init compositor?
	if (!vr::VRCompositor()) {
		printf("Compositor initialization failed\n");
		return 0;
	}

	sfz::runGameLoop(window, SharedPtr<BaseScreen>(sfz_new<vre::GameScreen>(vrSystem)));
	//sfz::runGameLoop(window, SharedPtr<BaseScreen>(sfz_new<vre::GameScreen>(nullptr)));

	// Clean up OpenVR
	vr::VR_Shutdown();

	return 0;
}