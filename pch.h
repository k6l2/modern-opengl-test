#pragma once
#include <SDL.h>
#include <GL/glew.h>
#include <SDL_opengl.h>
#include <cstdlib>
#include <vector>
using std::vector;
#include <string>
using std::string;
#include <chrono>
namespace k10
{
	const int FIXED_FRAMES_PER_SECOND = 240;
	const std::chrono::duration<double> FIXED_SECONDS_PER_FRAME =
		std::chrono::duration<double>(1) / FIXED_FRAMES_PER_SECOND;
	const GLint SHADER_BINARY_FORMAT = GL_SHADER_BINARY_FORMAT_SPIR_V;
}
template<class T>
inline T clamp(T value, T min, T max)
{
	return value < min ? min :
		value > max ? max :
		value;
}
namespace k10
{
	vector<Uint8> readFile(string const& fileName);
}