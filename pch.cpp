#include "pch.h"
namespace k10
{
	Window* gWindow;
	ThreadPool gThreadPool;
	VertexArray gVaTextureless;
	GlobalUniformBuffer gGlobalUniformBuffer;
	GfxProgram gProgTextureless;
}
vector<u8> k10::readFile(string const& fileName)
{
	vector<u8> retVal;
	SDL_RWops* file = SDL_RWFromFile(fileName.c_str(), "rb");
	if (!file)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to open file '%s'!\n", fileName.c_str());
		SDL_assert(false);
		return {};
	}
	const i64 fileSize = SDL_RWseek(file, 0, RW_SEEK_END);
	if (fileSize < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to get file size of '%s'!\n", fileName.c_str());
		SDL_assert(false);
		SDL_RWclose(file);
		return {};
	}
	if (SDL_RWseek(file, 0, RW_SEEK_SET) < 0)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to seek to file start for '%s'!\n", fileName.c_str());
		SDL_assert(false);
		SDL_RWclose(file);
		return {};
	}
	retVal.resize(static_cast<size_t>(fileSize));
	size_t currentReadByte = 0;
	while (true)
	{
		SDL_assert(retVal.size() >= currentReadByte);
		size_t maxBytesLeft = retVal.size() - currentReadByte;
		if (maxBytesLeft == 0)
		{
			// There are no more bytes to read.  We're done!
			break;
		}
		const size_t bytesRead = SDL_RWread(file, 
											&retVal[currentReadByte], 
											sizeof(Uint8), 
											maxBytesLeft);
		currentReadByte += bytesRead;
		if (bytesRead <= 0)
		{
			// the only time bytesRead is 0 is on error, or if we reached EOF,
			//	so there's no point in continuing...
			break;
		}
	}
	SDL_RWclose(file);
	if (currentReadByte != retVal.size())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"Failed to read file '%s'!\n", fileName.c_str());
		SDL_assert(false);
		return {};
	}
	return retVal;
}
bool k10::initializeGlobalAppData()
{
	SDL_Log("========== CPU Attributes ==================\n");
	SDL_Log("\tLogic core count=%i\n", SDL_GetCPUCount());
	SDL_Log("\tL1 cache line size=%i bytes\n", SDL_GetCPUCacheLineSize());
	SDL_Log("============================================\n");
	// As soon as possible (when SDL is initialized), spawn our job thread pool //
	const size_t desiredThreadPoolSize = max(SDL_GetCPUCount() - 1, 1);
	SDL_Log("Creating ThreadPool with %lli worker threads...\n", desiredThreadPoolSize);
	gThreadPool.create(desiredThreadPoolSize);
	// As soon as our job thread pool is running, create our application window.
	//	This way, we could potentially do work while the main thread is 
	//	communicating w/ the OS to create a window, drawing context, etc... //
	gWindow = Window::create("modern-opengl-test", { 1280,720 });
	if (!gWindow)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Window creation failed!\n");
		return false;
	}
	if (!gVaTextureless.create(VertexArray::VertexType::TEXTURELESS_MESH_INSTANCES))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to create textureless Vertex Array Object!\n");
		return false;
	}
	if (!gGlobalUniformBuffer.create())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to create GlobalUniformBuffer!\n");
		return false;
	}
	return true;
}
void k10::cleanupGlobalAppData()
{
	gVaTextureless.destroy();
	gGlobalUniformBuffer.destroy();
	gThreadPool.destroy();
	if (gWindow)
	{
		Window::destroy(gWindow);
		gWindow = nullptr;
	}
}
bool k10::loadGlobalAssets()
{
	if (!gProgTextureless.load("shader-bin/simple-draw-vert.spv", 
							   "shader-bin/simple-draw-frag.spv"))
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
					 "Failed to load textureless GfxProgram!\n");
		return false;
	}
	else
	{
		SDL_Log("Loaded textureless GfxProgram!\n");
	}
	return true;
}
void k10::cleanupGlobalAssets()
{
	gProgTextureless.free();
}