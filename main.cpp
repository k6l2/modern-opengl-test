#include "Window.h"
#include "Color.h"
#include "GfxProgram.h"
GfxProgram gProgTextureless;
GfxProgram gProgTextured;
#include "VertexArray.h"
VertexArray gVaTextureless;
#include "GlobalUniformBuffer.h"
GlobalUniformBuffer gGlobalUniformBuffer;
#include "VertexBuffer.h"
VertexBuffer gVbPosition;
VertexBuffer gVbColor;
VertexBuffer gVbModel;
int main(int argc, char** argv)
{
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
		gProgTextureless.free();
		gProgTextured.free();
		SDL_Quit();
		return retVal;
	};
	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
			"unable to init SDL: '%s'\n", SDL_GetError());
		return cleanup(EXIT_FAILURE);
	}
	SDL_LogSetPriority(SDL_LOG_CATEGORY_VIDEO, SDL_LOG_PRIORITY_DEBUG);
	SDL_LogSetPriority(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_DEBUG);
	window = Window::create("modern-opengl-test", { 1280,720 }, 
							&gProgTextureless, &gProgTextured);
	if (!window)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Window creation failed!\n");
		return cleanup(EXIT_FAILURE);
	}
	//	gProgTextured.load();
	if (!gProgTextureless.load("shader-bin/simple-draw-vert.spv", 
							   "shader-bin/simple-draw-frag.spv", true))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to load textureless GfxProgram!\n");
		return cleanup(EXIT_FAILURE);
	}
	else
	{
		SDL_Log("Loaded textureless GfxProgram!\n");
	}
	if (!gVaTextureless.create(VertexArray::VertexType::TEXTURELESS))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to create textureless Vertex Array Object!\n");
		return cleanup(EXIT_FAILURE);
	}
	if (!gGlobalUniformBuffer.create())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to create GlobalUniformBuffer!\n");
		return cleanup(EXIT_FAILURE);
	}
	glm::mat4 testProjection = glm::ortho(0.f, 1280.f, 720.f, 0.f);
	// center the view on the origin
	glm::mat3x2 testView = glm::translate(glm::mat3(1.f), v2f(1280, 720) * 0.5f);
	SDL_Log("testProjection=%s size=%i\n", 
		glm::to_string(testProjection).c_str(), sizeof(testProjection));
	SDL_Log("testView=%s size=%i\n", 
		glm::to_string(testView).c_str(), sizeof(testView));
	SDL_Log("gGlobalUniformBuffer.getSize()=%i\n", gGlobalUniformBuffer.getSize());
	gGlobalUniformBuffer.update(testProjection, testView);

	vector<v2f> positions = { v2f(-300, 300), v2f(0, -300), v2f(300, 300) };
	gVbPosition.create(3*sizeof(v2f), sizeof(v2f), VertexBuffer::MemoryUsage::STATIC);
	gVbPosition.update(positions.data());

	vector<Color> colors = { Color::Red, Color::Green, Color::Blue };
	gVbColor.create(3 * sizeof(Color), sizeof(Color), VertexBuffer::MemoryUsage::DYNAMIC);
	gVbColor.update(colors.data());

	gVbModel.create(1 * sizeof(glm::mat3x2), sizeof(glm::mat3x2), VertexBuffer::MemoryUsage::DYNAMIC);
	vector<glm::mat3x2> models = { glm::mat3x2(1.f) };
	//vector<glm::mat3x2> models = { glm::translate(glm::mat3(1.f), v2f(0,0)) };
	gVbModel.update(models.data());
	
	// Debug querying.... //
	{
		const GLuint uniformBlockIndex = 
			glGetUniformBlockIndex(gProgTextureless.getProgramId(), "GlobalMatrixBlock");
		GLuint indices[2];
		GLint offsets[2];
		GLsizei sizes[2];
		glGetActiveUniformBlockiv(gProgTextureless.getProgramId(), uniformBlockIndex,
			GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES, (GLint*)indices);
		glGetActiveUniformsiv(gProgTextureless.getProgramId(), 2, indices, GL_UNIFORM_OFFSET, offsets);
		glGetActiveUniformsiv(gProgTextureless.getProgramId(), 2, indices, GL_UNIFORM_SIZE, sizes);
		for (size_t i = 0; i < 2; i++)
		{
			SDL_Log("uniform[%i] - offset=%i size=%i\n", i, offsets[i], sizes[i]);
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
		///glViewport(0, 0,1280,720);
		window->clear({ 0.2f, 0.2f, 0.2f, 1.f });
		VertexArray::useTextureless(&gVaTextureless, gGlobalUniformBuffer,
									gVbPosition, gVbColor, gVbModel);
		glUseProgram(gProgTextureless.getProgramId());
//		glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 1);
		glUseProgram(NULL);
		glBindVertexArray(NULL);
		window->swapBuffer();
	}
	return cleanup(EXIT_SUCCESS);
}
