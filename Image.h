#pragma once
#include "Color.h"
class Image
{
public:
	void resize(glm::uvec2 const& newSize);
	bool loadFromFile(string const& fileName);
	glm::uvec2 const& getSize() const;
	vector<Uint8> const& getPixelData() const;
	vector<Uint8>& getPixelData();
	Color getPixel(glm::u32 x, glm::u32 y) const;
	Color getPixel(glm::uvec2 const& position) const;
	void setPixel(glm::u32 x, glm::u32 y, Color const& color);
	void setPixel(glm::uvec2 const& position, Color const& color);
private:
	vector<Uint8> pixelData;
	glm::uvec2 size;
};
