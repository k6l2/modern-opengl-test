#include "Window.h"
#include "Clock.h"
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
///VertexBuffer gVbModel;
VertexBuffer gVbModelTranslation;
VertexBuffer gVbModelRadians;
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
	// initialize global matrix uniform buffer //
	{
		glm::mat4 testProjection = glm::ortho(0.f, 1280.f, 720.f, 0.f);
		glm::mat3x2 testView = glm::mat3x2(1.f);
		//glm::mat3x2 testView = glm::translate(glm::mat3(1.f), v2f(1280, 720) * 0.5f);
		gGlobalUniformBuffer.update(testProjection, testView);
		SDL_Log("testProjection=%s size=%i\n", 
			glm::to_string(testProjection).c_str(), sizeof(testProjection));
		SDL_Log("testView=%s size=%i\n", 
			glm::to_string(testView).c_str(), sizeof(testView));
	}
	// instanced 2D mesh testing //
	static const v2f MESH_AABB = {10, 10};
	vector<v2f> positions = { 
		v2f(-MESH_AABB.x/2,  MESH_AABB.y/2),
		v2f(0             , -MESH_AABB.y/2),
		v2f( MESH_AABB.x/2,  MESH_AABB.y/2) };
	vector<Color> colors = { Color::Red, Color::Green, Color::Blue };
	static const size_t NUM_MESH_COLS = static_cast<size_t>((MESH_AABB.x + 1280.f) / MESH_AABB.x);
	static const size_t NUM_MODELS = 50000;
	///vector<glm::mat3x2> models(NUM_MODELS);
	vector<v2f> modelTranslations(NUM_MODELS);
	vector<float> modelRadians(NUM_MODELS);
	for (size_t m = 0; m < modelTranslations.size(); m++)
	{
		const size_t meshCol = m % NUM_MESH_COLS;
		const size_t meshRow = m / NUM_MESH_COLS;
		modelRadians[m] = k10::PI * 0.1f * m;
		///models[m] = glm::rotate(glm::translate(glm::mat3(1.f),
		///	MESH_AABB * v2f(meshCol, meshRow)),
		///	k10::PI * 0.1f * m);
		modelTranslations[m] = MESH_AABB * v2f(meshCol, meshRow);
	}
	gVbPosition.create(positions.size(), sizeof(positions[0])   , VertexBuffer::MemoryUsage::STATIC);
	gVbColor   .create(colors.size()   , sizeof(colors[0])      , VertexBuffer::MemoryUsage::STATIC);
	///gVbModel   .create(models.size()   , sizeof(glm::mat3x2), VertexBuffer::MemoryUsage::STREAM);
	gVbModelTranslation.create(modelTranslations.size(), sizeof(modelTranslations[0]), VertexBuffer::MemoryUsage::STREAM);
	gVbModelRadians    .create(modelRadians.size()     , sizeof(modelRadians[0])     , VertexBuffer::MemoryUsage::STREAM);
	gVbPosition.update(positions.data());
	gVbColor   .update(colors.data());
	///gVbModel   .update(models.data());
	gVbModelTranslation.update(modelTranslations.data());
	gVbModelRadians    .update(modelRadians.data());
	VertexArray::useTextureless(&gVaTextureless, gGlobalUniformBuffer,
								gVbPosition, gVbColor, 
								gVbModelTranslation, gVbModelRadians);
	glUseProgram(gProgTextureless.getProgramId());
	// MAIN APPLICATION LOOP //////////////////////////////////////////////////
	Time frameTimeAccumulator;
	Clock frameClock;
	while (true)
	{
		SDL_Event event;
		bool quit = false;
		while (SDL_PollEvent(&event))
		{
			window->processEvent(event);
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
		const Time frameDelta = frameClock.restart();
		frameTimeAccumulator += frameDelta;
		u32 logicTicks = 0, unusedLogicTicks = 0;
		Clock logicClock;
		while (frameTimeAccumulator >= k10::FIXED_TIME_PER_FRAME)
		{
			///TODO: @fix-your-timestep
			///	previousState = currentState;
			// MAIN LOOP LOGIC //
			if(logicClock.getElapsedTime() <= k10::MAX_LOGIC_TIME_PER_FRAME)
			{
				for (size_t m = 0; m < modelTranslations.size(); m++)
				{
					static const float RADIANS_PER_SECOND = k10::PI * 2.f;
					const size_t meshCol = m % NUM_MESH_COLS;
					const size_t meshRow = m / NUM_MESH_COLS;
					///models[m] = glm::rotate(glm::translate(glm::mat3(1.f),
					///	MESH_AABB * v2f(meshCol, meshRow)),
					///	modelRadians[m] + 
					///		RADIANS_PER_SECOND*k10::FIXED_TIME_PER_FRAME.seconds());
					modelTranslations[m] = MESH_AABB * v2f(meshCol, meshRow);
					modelRadians[m] +=
						RADIANS_PER_SECOND * k10::FIXED_TIME_PER_FRAME.seconds();
				}
				///gVbModel.update(models.data());
				gVbModelTranslation.update(modelTranslations.data());
				gVbModelRadians    .update(modelRadians.data());
				logicTicks++;
			}
			else
			{
				unusedLogicTicks++;
			}
			frameTimeAccumulator -= k10::FIXED_TIME_PER_FRAME;
		}
		SDL_Log("frameDelta.milliseconds()=%i logicTicks=%i unusedLogicTicks=%i\n", 
				frameDelta.milliseconds(), logicTicks, unusedLogicTicks);
		///TODO: @fix-your-timestep
		///	const float interFrameRatio = 
		///		frameAccumulator.count() / 
		///			k10::FIXED_SECONDS_PER_FRAME.count();
		///	const state = previousState*(1 - interFrameRatio) + 
		///				  currentState*interFrameRatio
		///glViewport(0, 0,1280,720);
		window->clear({ 0.2f, 0.2f, 0.2f, 1.f });
		// MAIN DRAW LOGIC //
		{
			int models_size = static_cast<int>(modelTranslations.size());
			ImGui::Begin("DEBUG");
			ImGui::SliderInt("models.size()", &models_size, 1, 1000000);
			ImGui::InputInt("models.size() set", &models_size);
			ImGui::End();
			if (models_size != modelTranslations.size())
			{
				modelTranslations.resize(models_size);
				modelRadians     .resize(models_size);
				gVbModelTranslation.resize(models_size);
				gVbModelRadians    .resize(models_size);
				gVbModelTranslation.update(modelTranslations.data());
				gVbModelRadians    .update(modelRadians.data());
			}
			//glDrawArrays(GL_TRIANGLES, 0, 3);
			glDrawArraysInstanced(GL_TRIANGLES, 0, 3, static_cast<GLsizei>(modelTranslations.size()));
///			glUseProgram(NULL);
///			glBindVertexArray(NULL);
		}
		window->swapBuffer();
	}
	return cleanup(EXIT_SUCCESS);
}
