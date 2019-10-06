#pragma once
#include "Image.h"
class TextureGfxProgramInterface
{
	friend class GfxProgram;
protected:
	virtual GLuint getTextureId() const = 0;
};
class Texture : public TextureGfxProgramInterface
{
public:
	bool loadFromFile(string const& fileName);
	bool loadFromImage(Image const& image);
	// Texture must already be loaded for this function to work.
	// Do not specify an update region that lies outside of the texel 
	//	region indicated by textureSize.
	bool updateFromImage(Image const& image, 
						 v2u const& textureDestinationOffset);
	Image copyToImage() const;
	void free();
	void setSmooth(bool smooth);
	void setRepeated(bool repeated);
	v2u getSize() const;
private:
	virtual GLuint getTextureId() const override;
private:
	GLuint textureId = NULL;
};