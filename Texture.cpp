#include "Texture.h"
bool Texture::loadFromFile(string const& fileName)
{
	Image img;
	if (!img.loadFromFile(fileName))
	{
		return false;
	}
	return loadFromImage(img);
}
bool Texture::loadFromImage(Image const& image)
{
	free();
	const v2u imageSize = image.getSize();
	// Send the decoded image data to the GPU //
	const v2u textureSize = imageSize;
	glGenTextures(1, &textureId);
	glBindTexture(GL_TEXTURE_2D, textureId);
	{
		const GLenum errorStateOpenGL = glGetError();
		if (errorStateOpenGL != GL_NO_ERROR)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"failed to bind texture! textureId=%i GL error string='%s'\n",
				textureId, gluErrorString(errorStateOpenGL));
			return false;
		}
	}
	const GLint mipmapLevel = 0;
	glTexImage2D(GL_TEXTURE_2D, mipmapLevel, GL_RGBA, 
				 textureSize.x, textureSize.y, 0,
				 GL_RGBA, GL_UNSIGNED_BYTE, &(image.getPixelData()[0]));
	{
		const GLenum errorStateOpenGL = glGetError();
		if (errorStateOpenGL != GL_NO_ERROR)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"failed to send image to texture! GL error string='%s'\n",
				gluErrorString(errorStateOpenGL));
			return false;
		}
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	{
		const GLenum errorStateOpenGL = glGetError();
		if (errorStateOpenGL != GL_NO_ERROR)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"failed to set texture params! GL error string='%s'\n",
				gluErrorString(errorStateOpenGL));
			return false;
		}
	}
	glBindTexture(GL_TEXTURE_2D, NULL);
	{
		const GLenum errorStateOpenGL = glGetError();
		if (errorStateOpenGL != GL_NO_ERROR)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"failed to unbind texture! GL error string='%s'\n",
				gluErrorString(errorStateOpenGL));
			return false;
		}
	}
	return true;
}
bool Texture::updateFromImage(Image const& image,
							  v2u const& offset)
{
	SDL_assert(textureId != NULL);
	const v2u textureSize = getSize();
	SDL_assert(offset.x + image.getSize().x <= textureSize.x);
	SDL_assert(offset.y + image.getSize().y <= textureSize.y);
	glBindTexture(GL_TEXTURE_2D, textureId);
	const GLint mipmapLevel = 0;
	glTexSubImage2D(GL_TEXTURE_2D, mipmapLevel, 
		offset.x, offset.y, image.getSize().x, image.getSize().y,
		GL_RGBA, GL_UNSIGNED_BYTE, &(image.getPixelData()[0]));
	glBindTexture(GL_TEXTURE_2D, NULL);
	{
		const GLenum errorStateOpenGL = glGetError();
		if (errorStateOpenGL != GL_NO_ERROR)
		{
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"failed! GL error string='%s'\n",
				gluErrorString(errorStateOpenGL));
			return false;
		}
	}
	return true;
}
Image Texture::copyToImage() const
{
	Image retVal;
	const v2u textureSize = getSize();
	retVal.resize(textureSize);
	vector<Uint8>& pixelData = retVal.getPixelData();
	glBindTexture(GL_TEXTURE_2D, textureId);
	const GLint mipmapLevel = 0;
	glGetTexImage(GL_TEXTURE_2D, mipmapLevel, GL_RGBA, GL_UNSIGNED_BYTE, 
				  &(pixelData[0]));
	glBindTexture(GL_TEXTURE_2D, NULL);
	const GLenum errorStateOpenGL = glGetError();
	if (errorStateOpenGL != GL_NO_ERROR)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
			"failed to copy texture! GL error string='%s'\n",
			gluErrorString(errorStateOpenGL));
		return retVal;
	}
	// If we ever need to account for old versions of OpenGL that don't support
	//	non-power-of-two sized textures, we should do it somewhere in here...
	return retVal;
}
void Texture::free()
{
	if (textureId)
	{
		glDeleteTextures(1, &textureId);
		textureId = NULL;
	}
}
void Texture::setSmooth(bool smooth)
{
	SDL_assert(textureId != NULL);
	glBindTexture(GL_TEXTURE_2D, textureId);
	if (smooth)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	glBindTexture(GL_TEXTURE_2D, NULL);
}
void Texture::setRepeated(bool repeated)
{
	glBindTexture(GL_TEXTURE_2D, textureId);
	if (repeated)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}
	glBindTexture(GL_TEXTURE_2D, NULL);
}
v2u Texture::getSize() const
{
	if (textureId == NULL)
	{
		return { 0,0 };
	}
	int w, h;
	glBindTexture(GL_TEXTURE_2D, textureId);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH , &w);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	glBindTexture(GL_TEXTURE_2D, NULL);
	return v2u(
		static_cast<Uint32>(w),
		static_cast<Uint32>(h));
}
GLuint Texture::getTextureId() const
{
	return textureId;
}
