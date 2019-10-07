#pragma once
class Color : public glm::u8vec4
{
public:
	static const Color Transparent;
	static const Color Black;
	static const Color White;
	static const Color Red;
	static const Color Green;
	static const Color Yellow;
	static const Color Blue;
public:
	Color(glm::u8vec4 glColor = { 255,255,255,255 });
	Color(float r = 1.f, float g = 1.f, float b = 1.f, float a = 1.f);
	float fR() const;
	float fG() const;
	float fB() const;
	float fA() const;
};
