SDL_Window* window = nullptr;
SDL_GLContext context = NULL;
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
			"unable to init SDL: '%s'\n", SDL_GetError());
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
			"failed to create window: '%s'\n", SDL_GetError());
		return cleanup(EXIT_FAILURE);
	}
	context = SDL_GL_CreateContext(window);
	if (context == NULL)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"failed to create context: '%s'\n", SDL_GetError());
		return cleanup(EXIT_FAILURE);
	}
	// check the values of our GL context, since the system could give us
	//	something different //
	{
		int vMajor, vMinor, profile;
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, &profile);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &vMajor);
		SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &vMinor);
		SDL_Log("======= OpenGL Context Attributes ==========\n");
		SDL_Log("Profile = '%s'\n", profile == SDL_GL_CONTEXT_PROFILE_COMPATIBILITY ? "Compatibility" :
									profile == SDL_GL_CONTEXT_PROFILE_ES ? "ES" : "Core");
		SDL_Log("Version = %i.%i\n", vMajor, vMinor);
		SDL_Log("============================================\n");
		if (vMajor != 4 || vMinor != 6 || profile != SDL_GL_CONTEXT_PROFILE_CORE)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"failed to retrieve supported OpenGL context from system!\n");
			return cleanup(EXIT_FAILURE);
		}
	}
	// Initialize GLEW //
	{
		glewExperimental = GL_TRUE;
		const GLenum oglStatus = glewInit();
		if (oglStatus != GLEW_OK)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"failed to initialize GLEW: '%s'\n", glewGetErrorString(oglStatus));
			return cleanup(EXIT_FAILURE);
		}
	}
	// Query shader binary format support //
	{
		GLint numShaderBinFormats;
		glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &numShaderBinFormats);
		if (numShaderBinFormats <= 0)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"System doesn't support binary shaders! numShaderBinFormats==0\n");
			return cleanup(EXIT_FAILURE);
		}
		vector<GLint> shaderBinFormats(numShaderBinFormats);
		glGetIntegerv(GL_SHADER_BINARY_FORMATS, shaderBinFormats.data());
		auto decodeShaderBinFormat = [](GLint format)->string
		{
			switch (format)
			{
			case GL_SHADER_BINARY_FORMAT_SPIR_V:
				return "GL_SHADER_BINARY_FORMAT_SPIR_V";
			///case GL_SHADER_BINARY_FORMAT_SPIR_V_ARB:
			///	return "GL_SHADER_BINARY_FORMAT_SPIR_V_ARB";
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
				"System doesn't support required shader binary format! '%s'\n",
				decodeShaderBinFormat(k10::SHADER_BINARY_FORMAT).c_str());
			return cleanup(EXIT_FAILURE);
		}
	}
	// Test loading a SPIR-V shader program //
	{
		vector<Uint8> shaderBin = k10::readFile("shader-bin/simple-draw-tri-vert.spv");
		GLuint shader = glCreateShader(GL_VERTEX_SHADER);
		if (shader == NULL)
		{
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"Failed to create shader object.\n");
			return cleanup(EXIT_FAILURE);
		}
		SDL_assert(glGetError() == GL_NO_ERROR);
		glShaderBinary(1, &shader, k10::SHADER_BINARY_FORMAT, 
					   shaderBin.data(), static_cast<GLsizei>(shaderBin.size()));
		const GLenum oglStatus = glGetError();
		if (oglStatus != GL_NO_ERROR)
		{
			switch (oglStatus)
			{
			case GL_INVALID_OPERATION:
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					"GL_INVALID_OPERATION: more than one shader handle refers to the same shader object\n");
				break;
			case GL_INVALID_ENUM:
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					"GL_INVALID_ENUM: 'binaryFormat'(%i) is not an accepted value \n", 
					k10::SHADER_BINARY_FORMAT);
				break;
			case GL_INVALID_VALUE:
				SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					"GL_INVALID_VALUE: data pointed to by 'binary' does not match the format specified "
						"by 'binaryFormat'(%i), or shader(%i) is possibly invalid!\n", 
					k10::SHADER_BINARY_FORMAT, shader);
				break;
			}
			SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
				"failed to load pre-compiled shader 'shader-bin/simple-draw-tri-vert.spv'\n");
			return cleanup(EXIT_FAILURE);
		}
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
