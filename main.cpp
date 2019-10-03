SDL_Window* window;
SDL_GLContext context;
int main(int argc, char** argv)
{
	auto cleanup = [&](int retVal)->int
	{
		if (retVal != EXIT_SUCCESS)
		{
			SDL_assert(false);
		}
		if (context)
		{
			SDL_GL_DeleteContext(context);
		}
		if (window)
		{
			SDL_DestroyWindow(window);
			window = nullptr;
		}
		SDL_Quit();
		return retVal;
	};
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
			"unable to init SDL: '%s'", SDL_GetError());
		return cleanup(EXIT_FAILURE);
	}
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	window = SDL_CreateWindow("modern-opengl-test",
							  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
							  1024, 768, SDL_WINDOW_OPENGL);
	if (!window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"failed to create window: '%s'", SDL_GetError());
		return cleanup(EXIT_FAILURE);
	}
	context = SDL_GL_CreateContext(window);
	if (context == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"failed to create context: '%s'", SDL_GetError());
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
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		SDL_GL_SwapWindow(window);
	}
	return cleanup(EXIT_SUCCESS);
}
