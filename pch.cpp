#include "pch.h"
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