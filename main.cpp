#include "Clock.h"
#include "GuiFrameDiagnosticData.h"
#include "InstancedMeshCache.h"
size_t modelsPerJob = 10000;
static const v2f MESH_AABB = v2f(40, 40);
size_t numMeshCols = 100;
vector<v2f> originPositions = { {0,0} };
vector<float> originRadians = { 0 };
vector<v2f> originScales = { {100,100} };
vector<v2f> circlePositions(1000);
vector<float> circleRadians(1000);
vector<v2f> circleScales(1000, v2f(20, 20));
u32 simulationTicksPerFrame = 0;
SDL_Thread* threadSimulation = nullptr;
// taken from: https://stackoverflow.com/a/14792685/4526664
class OrderedLock
{
public:
	void lock()
	{
		OPTICK_EVENT();
		unique_lock<mutex> mutexLock(orderedMutex);
		if (locked)
		{
			conditionQ.emplace();
			conditionQ.back().wait(mutexLock);
		}
		else
		{
			locked = true;
		}
	}
	void unlock()
	{
///		OPTICK_EVENT();
		unique_lock<mutex> mutexLock(orderedMutex);
		if (conditionQ.empty())
		{
			locked = false;
		}
		else
		{
			conditionQ.front().notify_one();
			conditionQ.pop();
		}
	}
private:
	queue<condition_variable> conditionQ;
	mutex orderedMutex;
	bool locked = false;
};
OrderedLock orderedLockSimulation;
int simulationThreadMain(void* data)
{
	OPTICK_THREAD("SimulationThread");
	while (true)
	{
		orderedLockSimulation.lock();
		Clock logicTimer;
		{
			OPTICK_EVENT("SimulationFrame");
			for (size_t m = 0; m < circlePositions.size(); m += modelsPerJob)
			{
				const JobDataProcessModels job = {
					JobTitle::PROCESS_MODELS,
					circlePositions.data(),
					circleRadians.data(),
					circleScales.data(),
					min(circlePositions.size() - m, size_t(modelsPerJob)), // modelCount
					m, // modelOffset
					MESH_AABB, numMeshCols
				};
				k10::gThreadPool.postJob(job);
			}
			k10::gThreadPool.waitUntilAllWorkIsFinished();
		}
		simulationTicksPerFrame++;
		orderedLockSimulation.unlock();
///		Time logicTime = logicTimer.getElapsedTime();
///		if (logicTime < k10::FIXED_TIME_PER_FRAME)
///		{
///			SDL_Delay((k10::FIXED_TIME_PER_FRAME - logicTime).milliseconds());
///		}
		while (logicTimer.getElapsedTime() < k10::FIXED_TIME_PER_FRAME)
		{
			// spin //
		}
	}
	return EXIT_SUCCESS;
}
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
		const VertexBuffer::MemoryUnmapResult unmapResult =
			testIMeshCache.unmapBuffers();
		SDL_assert(unmapResult == VertexBuffer::MemoryUnmapResult::SUCCESS);
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
	if (!testIMeshCache.create(1000, 2000010, 100, 3))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to create testIMeshCache!\n");
		return cleanup(EXIT_FAILURE);
	}
	if (!testIMeshCache.mapBuffers())
	{
		return cleanup(EXIT_FAILURE);
	}
	InstancedMeshCache::MeshId meshIdTriangle;
	InstancedMeshCache::MeshId meshIdCircle;
	InstancedMeshCache::MeshId meshIdOrigin;
	InstancedMeshCache::MeshId meshIdQuad;
	{
		vector<v2f> positions = {
			v2f(-1,-1),//upper left
			v2f(-1,1),//lower left
			v2f(1,-1),//upper right
			v2f(1,-1),//upper right
			v2f(-1,1),//lower left
			v2f(1,1),//lower right
		};
		vector<Color> colors = { Color::Red, Color::Green, Color::Blue,
			Color::Red, Color::Green, Color::Blue };
		meshIdQuad = testIMeshCache.addMesh(2000000, positions, colors,
			{ GL_TRIANGLES }, { 6 });
	}
	{
		static const v2f MESH_AABB = {50, 50};
		vector<v2f> positions = { 
			v2f(0             , -MESH_AABB.y/2), // top corner
			v2f(-MESH_AABB.x/2,  MESH_AABB.y/2), // bottom left
			v2f( MESH_AABB.x/2,  MESH_AABB.y/2)  // bottom right
		};
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
		meshIdCircle = testIMeshCache.addMesh(1,
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
///	{
///		if (!testIMeshCache.mapBuffers())
///		{
///			return cleanup(EXIT_FAILURE);
///		}
///		const VertexBuffer::MemoryUnmapResult unmapResult =
///			testIMeshCache.unmapBuffers();
///		SDL_assert(unmapResult == VertexBuffer::MemoryUnmapResult::SUCCESS);
///	}
	// MAIN APPLICATION LOOP //////////////////////////////////////////////////
	numMeshCols = static_cast<size_t>(k10::gWindow->getSize().x / MESH_AABB.x);
	for (size_t c = 0; c < circlePositions.size(); c++)
	{
		circleRadians[c] = c * k10::PI * 0.1f;
	}
	circlePositions.reserve(1500000);
	circleRadians.reserve(1500000);
	circleScales.reserve(1500000);
	threadSimulation = 
		SDL_CreateThread(simulationThreadMain, "SimulationThread", nullptr);
	SDL_assert(threadSimulation);
///	Time frameTimeAccumulator;
	Clock frameClock;
	GfxProgram::use(&k10::gProgTextureless);
	while (true)
	{
		OPTICK_FRAME("MainThread");
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
		orderedLockSimulation.lock();
		const Time frameDelta = frameClock.restart();
///		frameTimeAccumulator += frameDelta;
///		u32 logicTicks = 0, unusedLogicTicks = 0;
///		Clock logicClock;
///		while (frameTimeAccumulator >= k10::FIXED_TIME_PER_FRAME)
///		{
///			///TODO: @fix-your-timestep
///			///	previousState = currentState;
///			// MAIN LOOP LOGIC //
///			//if(logicClock.getElapsedTime() <= k10::MAX_LOGIC_TIME_PER_FRAME)
///			{
///				for (size_t m = 0; m < circlePositions.size(); m += modelsPerJob)
///				{
///					const JobDataProcessModels job = {
///						JobTitle::PROCESS_MODELS,
///						circlePositions.data(),
///						circleRadians.data(),
///						circleScales.data(),
///						min(circlePositions.size() - m, size_t(modelsPerJob)), // modelCount
///						m, // modelOffset
///						MESH_AABB, numMeshCols
///					};
///					k10::gThreadPool.postJob(job);
///				}
///				k10::gThreadPool.waitUntilAllWorkIsFinished();
///				logicTicks++;
///			}
/////			else
/////			{
/////				unusedLogicTicks++;
/////			}
///			frameTimeAccumulator -= k10::FIXED_TIME_PER_FRAME;
///		}
		guiFrameDiagData.append(frameDelta.milliseconds(), simulationTicksPerFrame, 0);
								//logicTicks, unusedLogicTicks);
		simulationTicksPerFrame = 0;
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
				OPTICK_EVENT("DEBUG TESTING GUI");
				ImGui::Begin("DEBUG");
				int actorCount = static_cast<int>(circlePositions.size());
				if (ImGui::SliderInt("actorCount", &actorCount, 0, 2000000) |
					ImGui::InputInt("actorCountSet", &actorCount))
				{
					circlePositions.resize(actorCount);
					circleRadians.resize(actorCount);
					circleScales.resize(actorCount);
				}
				ImGui::End();
			}
///			if (!testIMeshCache.mapBuffers())
///			{
///				return cleanup(EXIT_FAILURE);
///			}
			testIMeshCache.postUpdateInstanceJobs(meshIdOrigin, originPositions.size(),
				originPositions.data(), originRadians.data(), originScales.data(),
				modelsPerJob);
			testIMeshCache.postUpdateInstanceJobs(meshIdQuad, circlePositions.size(),
				circlePositions.data(), circleRadians.data(), circleScales.data(),
				modelsPerJob);
			k10::gThreadPool.waitUntilAllWorkIsFinished();
///			const VertexBuffer::MemoryUnmapResult unmapResult =
///				testIMeshCache.unmapBuffers();
///			SDL_assert(unmapResult == VertexBuffer::MemoryUnmapResult::SUCCESS);
			orderedLockSimulation.unlock();
			k10::gWindow->use();
			testIMeshCache.draw(k10::gVaTextureless);
			guiFrameDiagData.draw();
		}
		k10::gWindow->swapBuffer();
///		{
///			OPTICK_EVENT("glFinish");
///			glFinish();
///		}
	}
	return cleanup(EXIT_SUCCESS);
}
