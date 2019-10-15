#include "Clock.h"
#include "GuiFrameDiagnosticData.h"
#include "InstancedMeshCache.h"
int main(int argc, char** argv)
{
	InstancedMeshCache testIMeshCache;
	GuiFrameDiagnosticData guiFrameDiagData;
	auto cleanup = [&](int retVal)->int
	{
		if (retVal != EXIT_SUCCESS)
		{
			SDL_assert(false);
		}
		testIMeshCache.destroy();
		k10::cleanupGlobalAssets();
		k10::cleanupGlobalAppData();
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
	if (!k10::initializeGlobalAppData())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
					 "initializeGlobalAppData failed!\n");
		return cleanup(EXIT_FAILURE);
	}
	if (!k10::loadGlobalAssets())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
					 "loadGlobalAssets failed!\n");
		return cleanup(EXIT_FAILURE);
	}
	// instanced 2D mesh testing //
	//	1081080 is the first anti-prime above 1 million~
	if (!testIMeshCache.create(1000, 2000000, 100))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to create testIMeshCache!\n");
		return cleanup(EXIT_FAILURE);
	}
	InstancedMeshCache::MeshId meshIdTriangle;
	InstancedMeshCache::MeshId meshIdCircle;
	InstancedMeshCache::MeshId meshIdOrigin;
	{
		static const v2f MESH_AABB = {50, 50};
		vector<v2f> positions = { 
			v2f(-MESH_AABB.x/2,  MESH_AABB.y/2),
			v2f(0             , -MESH_AABB.y/2),
			v2f( MESH_AABB.x/2,  MESH_AABB.y/2) };
		vector<Color> colors = { Color::Red, Color::Green, Color::Blue };
		meshIdTriangle = testIMeshCache.addMesh(1, positions, colors,
												{GL_TRIANGLES}, {3});
	}
	// create a circle mesh w/ radius of 1
	{
		static const size_t circlePointCount = 24;
		static const float radiansPerPoint = 2*k10::PI / circlePointCount;
		vector<v2f> positionsFill(circlePointCount + 2);
		vector<v2f> positionsOutline(circlePointCount + 2);
		vector<Color> colorsFill(positionsFill.size());
		vector<Color> colorsOutline(positionsOutline.size());
		positionsFill[0] = v2f(0, 0);
		for (size_t c = 0; c < circlePointCount; c++)
		{
			const float radians = c * radiansPerPoint;
			const v2f point = glm::rotate(v2f(1, 0), radians);
			positionsFill[c + 1] = point;
			positionsOutline[c]  = point;
		}
		positionsFill   .back() = v2f(1, 0);
		positionsOutline[positionsOutline.size() - 2] = v2f(1, 0);
		positionsOutline[positionsOutline.size() - 1] = v2f(0, 0);
		for (size_t c = 0; c < colorsFill.size(); c++)
		{
			colorsFill[c] = Color(1.f, 1.f, 1.f, 0.2f);
		}
		for (size_t c = 0; c < colorsOutline.size(); c++)
		{
			colorsOutline[c] = Color::White;
		}
		positionsFill.reserve(positionsFill.size() + positionsOutline.size());
		positionsFill.insert(std::end(positionsFill), 
			std::begin(positionsOutline), std::end(positionsOutline));
		colorsFill.reserve(colorsFill.size() + colorsOutline.size());
		colorsFill.insert(std::end(colorsFill), 
			std::begin(colorsOutline), std::end(colorsOutline));
		meshIdCircle = testIMeshCache.addMesh(1500000,
			positionsFill, colorsFill,
			{GL_TRIANGLE_FAN, GL_LINE_STRIP}, 
			{static_cast<GLint>(positionsFill.size() - positionsOutline.size()), 
			 static_cast<GLint>(positionsOutline.size())});
//		meshIdCircle = testIMeshCache.addMesh(positionsFill, colorsFill,
//			1000000, { GL_TRIANGLE_FAN }, 
//			{ static_cast<GLint>(positionsFill.size()) });
	}
	{
		vector<v2f> positions = {
			v2f(0,0), v2f(1,0),
			v2f(0,0), v2f(0,1) };
		vector<Color> colors = {
			Color::Red, Color::Red,
			Color::Green, Color::Green };
		meshIdOrigin = testIMeshCache.addMesh(1, positions, colors,
											  { GL_LINES }, { 4 });
	}
	// MAIN APPLICATION LOOP //////////////////////////////////////////////////
	static const v2f MESH_AABB = v2f(40,40);
	const size_t NUM_MESH_COLS = static_cast<size_t>(k10::gWindow->getSize().x / MESH_AABB.x);
	vector<v2f> originPositions = { {0,0} };
	vector<float> originRadians = { 0 };
	vector<v2f> originScales = { {100,100} };
	vector<v2f> circlePositions(1000);
	vector<float> circleRadians(1000);
	vector<v2f> circleScales(1000, v2f(20,20));
	for (size_t c = 0; c < circlePositions.size(); c++)
	{
		circleRadians[c] = c * k10::PI * 0.1f;
	}
	circlePositions.reserve(1500000);
	circleRadians.reserve(1500000);
	circleScales.reserve(1500000);
	size_t modelsPerJob = 1000;
	float debugRadians = 0;
	Time frameTimeAccumulator;
	Clock frameClock;
	GfxProgram::use(&k10::gProgTextureless);
	while (true)
	{
		SDL_Event event;
		bool quit = false;
		while (SDL_PollEvent(&event))
		{
			k10::gWindow->processEvent(event);
			switch (event.type)
			{
			case SDL_MOUSEBUTTONDOWN:
				switch (event.button.button)
				{
				case SDL_BUTTON_LEFT: {
///					const v2f worldPosition = 
///						k10::gWindow->transformToWorldSpace(
///							{ event.button.x, event.button.y });
///					circlePositions.push_back(worldPosition);
///					circleRadians.push_back(debugRadians);
///					circleScales.push_back(v2f(20, 20));
					} break;
				}
				break;
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
				for (size_t m = 0; m < circlePositions.size(); m += modelsPerJob)
				{
					const JobDataProcessModels job = {
						circlePositions.data(),
						circleRadians.data(),
						circleScales.data(),
						min(circlePositions.size() - m, size_t(modelsPerJob)), // modelCount
						m, // modelOffset
						MESH_AABB, NUM_MESH_COLS
					};
					k10::gThreadPool.postJob(job);
				}
				while (!k10::gThreadPool.allWorkFinished())
				{
					// Spin the main thread while waiting for jobs to complete //
				}
				logicTicks++;
			}
			else
			{
				unusedLogicTicks++;
			}
			frameTimeAccumulator -= k10::FIXED_TIME_PER_FRAME;
		}
		guiFrameDiagData.append(frameDelta.milliseconds(), 
								logicTicks, unusedLogicTicks);
		///TODO: @fix-your-timestep
		///	const float interFrameRatio = 
		///		frameAccumulator.count() / 
		///			k10::FIXED_SECONDS_PER_FRAME.count();
		///	const state = previousState*(1 - interFrameRatio) + 
		///				  currentState*interFrameRatio
		k10::gWindow->clear({ 0.1f, 0.1f, 0.1f, 1.f });
		// MAIN DRAW LOGIC //
		{
			// DEBUG TESTING GUI //
			{
				ImGui::Begin("DEBUG");
				int actorCount = static_cast<int>(circlePositions.size());
				if (ImGui::SliderInt("actorCount", &actorCount, 0, 1500000))
				{
					circlePositions.resize(actorCount);
					circleRadians.resize(actorCount);
					circleScales.resize(actorCount);
				}
				ImGui::End();
			}
			testIMeshCache.updateMeshInstances(meshIdOrigin, originPositions.size(),
				originPositions.data(), originRadians.data(), originScales.data());
			testIMeshCache.updateMeshInstances(meshIdCircle, circlePositions.size(),
				circlePositions.data(), circleRadians.data(), circleScales.data());
			k10::gWindow->use();
			testIMeshCache.draw(k10::gVaTextureless);
			guiFrameDiagData.draw();
		}
		k10::gWindow->swapBuffer();
	}
	return cleanup(EXIT_SUCCESS);
}
