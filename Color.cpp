#include "Color.h"
const Color Color::Transparent = glm::u8vec4{   0,   0,   0,   0 };
const Color Color::Black       = glm::u8vec4{   0,   0,   0, 255 };
const Color Color::Red         = glm::u8vec4{ 255,   0,   0, 255 };
const Color Color::Green       = glm::u8vec4{   0, 255,   0, 255 };
const Color Color::Yellow      = glm::u8vec4{ 255, 255,   0, 255 };
const Color Color::White       = glm::u8vec4{ 255, 255, 255, 255 };
Color::Color(glm::u8vec4 glColor)
{
	r = glColor.r;
	g = glColor.g;
	b = glColor.b;
	a = glColor.a;
}
Color::Color(float r, float g, float b, float a)
{
	SDL_assert(r >= 0 && r <= 1);
	SDL_assert(g >= 0 && g <= 1);
	SDL_assert(b >= 0 && b <= 1);
	SDL_assert(a >= 0 && a <= 1);
	this->r = static_cast<Uint8>(r * 255);
	this->g = static_cast<Uint8>(g * 255);
	this->b = static_cast<Uint8>(b * 255);
	this->a = static_cast<Uint8>(a * 255);
}
float Color::fR() const
{
	return r / 255.f;
}
float Color::fG() const
{
	return g / 255.f;
}
float Color::fB() const
{
	return b / 255.f;
}
float Color::fA() const
{
	return a / 255.f;
}