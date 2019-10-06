#include "Image.h"
void Image::resize(glm::uvec2 const& newSize)
{
	size = newSize;
	pixelData.resize(size.x*size.y*4);
}
bool Image::loadFromFile(string const& fileName)
{
	// Load the png file //
	vector<Uint8> bufferFileData = k10::readFile(fileName);
	if (bufferFileData.empty())
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
			"failed to load image file '%s'\n", fileName.c_str());
		SDL_assert(false);
		return false;
	}
	// Decode the image file //
	const unsigned decodeResult = lodepng::decode(
		pixelData, size.x, size.y, bufferFileData);
	if (decodeResult)
	{
		SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
			": Could not decode png '%s'! error string='%s'\n",
			fileName.c_str(), lodepng_error_text(decodeResult));
		return false;
	}
	return true;
}
glm::uvec2 const& Image::getSize() const
{
	return size;
}
vector<Uint8> const& Image::getPixelData() const
{
	return pixelData;
}
vector<Uint8>& Image::getPixelData()
{
	return pixelData;
}
Color Image::getPixel(glm::u32 x, glm::u32 y) const
{
	const size_t pixelOffset = 4*(y * size.x + x);
	return Color{ glm::u8vec4{
		pixelData[pixelOffset + 0],
		pixelData[pixelOffset + 1],
		pixelData[pixelOffset + 2],
		pixelData[pixelOffset + 3]} };
}
Color Image::getPixel(glm::uvec2 const& position) const
{
	return getPixel(position.x, position.y);
}
void Image::setPixel(glm::u32 x, glm::u32 y, Color const& color)
{
	const size_t pixelOffset = 4 * (y * size.x + x);
	pixelData[pixelOffset + 0] = color.r;
	pixelData[pixelOffset + 1] = color.g;
	pixelData[pixelOffset + 2] = color.b;
	pixelData[pixelOffset + 3] = color.a;
}
void Image::setPixel(glm::uvec2 const& position, Color const& color)
{
	setPixel(position.x, position.y, color);
}