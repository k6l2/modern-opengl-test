#include "Window.h"
#include "Color.h"
#include "GfxProgram.h"
GfxProgram gProgTextureless;
GfxProgram gProgTextured;
int main(int argc, char** argv)
{
///	gProgTextured.load();
///	gProgTextureless.load();
	Window* window = nullptr;
	auto cleanup = [&](int retVal)->int
	{
		if (retVal != EXIT_SUCCESS)
		{
			SDL_assert(false);
		}
		if (window)
		{
			Window::destroy(window);
			window = nullptr;
		}
		SDL_Quit();
		return retVal;
	};
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
			"unable to init SDL: '%s'\n", SDL_GetError());
		return cleanup(EXIT_FAILURE);
	}
	window = Window::create("modern-opengl-test", { 1280,720 }, 
							&gProgTextureless, &gProgTextured);
	if (!window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Window creation failed!\n");
		return cleanup(EXIT_FAILURE);
	}
	// MAIN APPLICATION LOOP //////////////////////////////////////////////////
	while (true)
	{
		SDL_Event event;
		bool quit = false;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			}
		}
		if (quit)
		{
			break;
		}
		window->clear({ 0.2f, 0.2f, 0.2f, 1.f });
		window->swapBuffer();
	}
	return cleanup(EXIT_SUCCESS);
}
