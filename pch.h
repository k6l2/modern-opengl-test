#pragma once
#include <SDL.h>
using u8  = Uint8;
using u16 = Uint16;
using u32 = Uint32;
using u64 = Uint64;
using i8  = Sint8;
using i16 = Sint16;
using i32 = Sint32;
using i64 = Sint64;
#include <GL/glew.h>
#include <SDL_opengl.h>
#define GLM_FORCE_ALIGNED_GENTYPES
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "glm/gtx/matrix_transform_2d.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtc/type_aligned.hpp"
using v2f = glm::vec2;
using v2i = glm::ivec2;
using v2u = glm::uvec2;
#include "lodepng/lodepng.h"
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
	const float PI = glm::pi<float>();
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
	vector<u8> readFile(string const& fileName);
}