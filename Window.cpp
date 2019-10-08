#include "Window.h"
#include "Drawable.h"
#include "GfxProgram.h"
Window* Window::create(char const* title, glm::ivec2 const& size,
					   GfxProgram const* const programTextureless,
					   GfxProgram const* const programTextured)
{
	Window* retVal = new Window(programTextureless, programTextured);
	auto cleanup = [&]()->Window*
	{
		SDL_assert(false);
		if (retVal->context)
		{
			SDL_GL_DeleteContext(retVal->context);
			retVal->context = NULL;
		}
		if (retVal->window)
		{
			SDL_DestroyWindow(retVal->window);
			retVal->window = nullptr;
		}
		delete retVal;
		return nullptr;
	};
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	static char const*const glsl_version = "#version 460";
	retVal->window = SDL_CreateWindow(title,
									  SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
									  size.x, size.y, SDL_WINDOW_OPENGL);
	if (!retVal->window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"failed to create window: '%s'\n", SDL_GetError());
		return cleanup();
	}
	retVal->setSize(size);
	retVal->context = SDL_GL_CreateContext(retVal->window);
	if (retVal->context == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"failed to create context: '%s'\n", SDL_GetError());
		return cleanup();
	}
	// check the values of our GL context, since the system could give us
	//	something different //
	{
		int vMajor, vMinor, profile, maxUniformBlockSize, 
			maxVertexUniformComponents, maxUniformBufferBindings,
			maxUniformLocations, maxVertexUniformBlocks,
			maxVertexUniformVectors, maxTextureSize, maxTextureImageUnits;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &vMajor);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &vMinor);
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
		glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &maxUniformBlockSize);
		glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &maxUniformBufferBindings);
		glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &maxUniformLocations);
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &maxVertexUniformComponents);
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &maxVertexUniformVectors);
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &maxVertexUniformBlocks);
		SDL_Log("======= OpenGL Context Attributes ==========\n");
		SDL_Log("\tVendor   = '%s'\n", glGetString(GL_VENDOR));
		SDL_Log("\tRenderer = '%s'\n", glGetString(GL_RENDERER));
		SDL_Log("\tProfile = '%s'\n", 
				profile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ? "Compatibility" :
				profile == SDL_GL_CONTEXT_PROFILE_ES ? "ES" : "Core");
		SDL_Log("\tVersion = %i.%i\n", vMajor, vMinor);
		SDL_Log("\tGL_MAX_TEXTURE_SIZE=%i (min=1024)\n", maxTextureSize);
		SDL_Log("\tGL_MAX_TEXTURE_IMAGE_UNITS=%i (min=16)\n", maxTextureImageUnits);
		SDL_Log("\tGL_MAX_UNIFORM_BLOCK_SIZE=%i (min=16384)\n", maxUniformBlockSize);
		SDL_Log("\tGL_MAX_UNIFORM_BUFFER_BINDINGS=%i (min=36)\n", maxUniformBufferBindings);
		SDL_Log("\tGL_MAX_UNIFORM_LOCATIONS=%i (min=1024)\n", maxUniformLocations);
		SDL_Log("\tGL_MAX_VERTEX_UNIFORM_COMPONENTS=%i (min=1024)\n", maxVertexUniformComponents);
		SDL_Log("\tGL_MAX_VERTEX_UNIFORM_VECTORS=%i (min=256)\n", maxVertexUniformVectors);
		SDL_Log("\tGL_MAX_VERTEX_UNIFORM_BLOCKS=%i (min=12)\n", maxVertexUniformBlocks);
		SDL_Log("============================================\n");
		if (vMajor != 4 || vMinor != 6 || 
			profile != SDL_GL_CONTEXT_PROFILE_CORE)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"failed to retrieve supported OpenGL context from system!\n");
			return cleanup();
		}
	}
	// Initialize GLEW //
	{
		glewExperimental = GL_TRUE;
		const GLenum oglStatus = glewInit();
		if (oglStatus != GLEW_OK)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
						 "failed to initialize GLEW: '%s'\n", 
						 glewGetErrorString(oglStatus));
			return cleanup();
		}
	}
	// Query shader binary format support //
	{
		GLint numShaderBinFormats;
		glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &numShaderBinFormats);
		if (numShaderBinFormats <= 0)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
						 "System doesn't support binary shaders! "
						 "numShaderBinFormats==0\n");
			return cleanup();
		}
		vector<GLint> shaderBinFormats(numShaderBinFormats);
		glGetIntegerv(GL_SHADER_BINARY_FORMATS, shaderBinFormats.data());
		auto decodeShaderBinFormat = [](GLint format)->string
		{
			switch (format)
			{
			case GL_SHADER_BINARY_FORMAT_SPIR_V:
				return "GL_SHADER_BINARY_FORMAT_SPIR_V";
			}
			return "UNKNOWN FORMAT";
		};
		bool shaderBinaryFormatRequirementsMet = false;
		for (GLint s = 0; s < numShaderBinFormats; s++)
		{
			if (shaderBinFormats[s] == k10::SHADER_BINARY_FORMAT)
			{
				shaderBinaryFormatRequirementsMet = true;
			}
			SDL_Log("Supported Shader Binary Format [%i]='%s'\n", 
					s, decodeShaderBinFormat(shaderBinFormats[s]).c_str());
		}
		if (!shaderBinaryFormatRequirementsMet)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
						 "System doesn't support required shader "
						 "binary format! '%s'\n",
						 decodeShaderBinFormat(k10::SHADER_BINARY_FORMAT).c_str());
			return cleanup();
		}
	}
	// Use Vsync //
//	if (SDL_GL_SetSwapInterval(1) != 0)
//	{
//		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
//					 ": Warning: Unable to set VSync! SDL Error: '%s'\n",
//					 SDL_GetError());
//	}
    // Setup Dear ImGui context
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	}
    // Setup Dear ImGui style
	{
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsClassic();
	}
    // Setup Platform/Renderer bindings
	{
		ImGui_ImplSDL2_InitForOpenGL(retVal->window, retVal->context);
		ImGui_ImplOpenGL3_Init(glsl_version);
	}
	return retVal;
}
void Window::destroy(Window* w)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
	if (w->context)
	{
		SDL_GL_DeleteContext(w->context);
		w->context = NULL;
	}
	if (w->window)
	{
		SDL_DestroyWindow(w->window);
		w->window = nullptr;
	}
}
Window::Window(GfxProgram const* const programTextureless,
			   GfxProgram const* const programTextured)
	: Renderer(programTextureless, programTextured)
{
}
void Window::clear(Color const& c)
{
	glClearColor(c.fR(), c.fG(), c.fB(), c.fA());
	glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
}
void Window::draw(Drawable& drawable, 
				  struct RenderState const& rState) const
{
	glViewport(0, 0, 
		static_cast<GLsizei>(getSize().x), 
		static_cast<GLsizei>(getSize().y));
	for (size_t b = 0; b < drawable.getBatchCount(); b++)
	{
		GfxProgram const*const gfxProgram = drawable.getTexture(b) ?
			getGfxProgramTextured() : getGfxProgramTextureless();
		gfxProgram->run(drawable, b, rState, 
						getProjection(), getView().getMatrix());
	}
}
void Window::processEvent(SDL_Event const& event)
{
	ImGui_ImplSDL2_ProcessEvent(&event);
}
void Window::swapBuffer()
{
	ImGui::Render();
	///glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	///glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
	///glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
}