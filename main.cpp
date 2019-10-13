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
	if (!testIMeshCache.create(1000, 1000002))
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
		meshIdTriangle = testIMeshCache.addMesh(positions, colors, 1, 
												{GL_TRIANGLES}, {3});
	}
	// create a circle mesh w/ radius of 1
	{
		static const size_t circlePointCount = 30;
		static const float radiansPerPoint = 2*k10::PI / circlePointCount;
		vector<v2f> positionsFill(circlePointCount + 2);
		vector<v2f> positionsOutline(circlePointCount + 1);
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
		positionsOutline.back() = v2f(1, 0);
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
		meshIdCircle = testIMeshCache.addMesh(positionsFill, colorsFill,
			1000000, {GL_TRIANGLE_FAN, GL_LINE_STRIP}, 
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
		meshIdOrigin = testIMeshCache.addMesh(positions, colors, 1, 
											  { GL_LINES }, { 4 });
	}
	const InstancedMeshCache::InstanceId instanceOrigin =
		testIMeshCache.createInstance(meshIdOrigin);
	const v2f originScale = v2f(100, 100);
	testIMeshCache.setModel(instanceOrigin, v2f(0, 0), 0, &originScale);
	// MAIN APPLICATION LOOP //////////////////////////////////////////////////
	Time frameTimeAccumulator;
	Clock frameClock;
	float debugRadians = 0;
	int debugInstanceId = 0;
	vector<InstancedMeshCache::InstanceId> iids;
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
					const v2f worldPosition = 
						k10::gWindow->transformToWorldSpace(
							{ event.button.x, event.button.y });
///					SDL_Log("left click @ [%i,%i]::window-space "
///							"[%f,%f]::world-space\n",
///							event.button.x, event.button.y,
///							worldPosition.x, worldPosition.y);
					// Create a triangle mesh at this location //
					const InstancedMeshCache::InstanceId iid = 
						testIMeshCache.createInstance(meshIdCircle);
///					if (iid != InstancedMeshCache::INVALID_INSTANCE_ID)
///					{
///						iids.push_back(iid);
///					}
					const v2f scale = { 20,20 };
					testIMeshCache.setModel(iid, worldPosition, debugRadians, &scale);
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
///				for (size_t m = 0; m < modelTranslations.size(); m += modelsPerJob)
///				{
///					const JobDataProcessModels job = {
///						modelTranslations.data(),
///						modelRadians.data(),
///						min(modelTranslations.size() - m, size_t(modelsPerJob)), // modelCount
///						m, // modelOffset
///						MESH_AABB, NUM_MESH_COLS
///					};
///					k10::gThreadPool.postJob(job);
///				}
///				while (!k10::gThreadPool.allWorkFinished())
///				{
///					// Spin the main thread while waiting for jobs to complete //
///				}
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
			k10::gWindow->use();
			testIMeshCache.draw(k10::gVaTextureless);
			guiFrameDiagData.draw();
			// debug instance GUI //
			{
				ImGui::Begin("DEBUG");
				ImGui::SliderFloat("rad", &debugRadians, 0, 2*k10::PI);
				ImGui::Separator();
				stringstream ssInstances;
				for (InstancedMeshCache::InstanceId i : iids)
				{
					ssInstances << i << ",";
				}
				ImGui::Text("%s", ssInstances.str().c_str());
				ImGui::InputInt("iid", &debugInstanceId);
				if (ImGui::Button("destroy tri"))
				{
					testIMeshCache.destroyInstance(meshIdTriangle, debugInstanceId);
					auto findIt = std::find(iids.begin(), iids.end(), debugInstanceId);
					SDL_assert(findIt != iids.end());
					iids.erase(findIt);
				}
				ImGui::End();
			}
		}
		k10::gWindow->swapBuffer();
	}
	return cleanup(EXIT_SUCCESS);
}
