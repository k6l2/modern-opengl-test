#include "VertexBuffer.h"
bool VertexBuffer::create(size_t elementCount, GLsizei s, MemoryUsage u)
{
	destroy();
	glGenBuffers(1, &bufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
	glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizei>(elementCount*s), 
				 NULL, decodeMemoryUsage(u));
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	const GLenum oglStatus = glGetError();
	if (oglStatus != GL_NO_ERROR)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to create VertexBuffer. OGL Error='%s'\n",
					 glewGetErrorString(oglStatus));
		return false;
	}
	dataSize = static_cast<GLsizei>(elementCount * s);
	elementStride = s;
	usage = u;
	return true;
}
void VertexBuffer::destroy()
{
	if (bufferObject)
	{
		glDeleteBuffers(1, &bufferObject);
		bufferObject = NULL;
	}
	dataSize = 0;
}
void VertexBuffer::update(void const* newData)
{
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
	glBufferData(GL_ARRAY_BUFFER, dataSize, NULL, decodeMemoryUsage(usage));
	glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, newData);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	const GLenum oglStatus = glGetError();
	if (oglStatus != GL_NO_ERROR)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to update VertexBuffer. OGL Error='%s'\n",
					 glewGetErrorString(oglStatus));
		SDL_assert(false);
	}
}
void VertexBuffer::update(void const* newData, 
						  GLint offset, GLsizei size)
{
	SDL_assert(bufferObject);
	glBindBuffer(GL_ARRAY_BUFFER, bufferObject);
	glBufferSubData(GL_ARRAY_BUFFER, offset, size, newData);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	const GLenum oglStatus = glGetError();
	if (oglStatus != GL_NO_ERROR)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to update VertexBuffer. OGL Error='%s'\n",
					 glewGetErrorString(oglStatus));
		SDL_assert(false);
	}
}
void VertexBuffer::bind(GLuint bufferBindingIndex) const
{
	bind(bufferBindingIndex, 0);
}
void VertexBuffer::bind(GLuint bufferBindingIndex, 
					   GLint offset) const
{
	SDL_assert(bufferObject);
	SDL_assert(glGetError() == GL_NO_ERROR);
	glBindVertexBuffer(bufferBindingIndex, bufferObject, offset, elementStride);
	const GLenum oglStatus = glGetError();
	if (oglStatus != GL_NO_ERROR)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to use VertexBuffer. ");
		switch (oglStatus)
		{
		case GL_INVALID_ENUM:
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"GL_INVALID_ENUM ");
			break;
		case GL_INVALID_VALUE:
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"GL_INVALID_VALUE ");
			break;
		default:
			SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
				"UNKNOWN ERROR ");
			break;
		}
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "OGL Error='%s'\n",
					 glewGetErrorString(oglStatus));
		SDL_assert(false);
	}
}
GLenum VertexBuffer::decodeMemoryUsage(MemoryUsage mu)
{
	switch (mu)
	{
	case MemoryUsage::STATIC:
		return GL_STATIC_DRAW;
	case MemoryUsage::DYNAMIC:
		return GL_DYNAMIC_DRAW;
	case MemoryUsage::STREAM:
		return GL_STREAM_DRAW;
	}
	SDL_assert(false);
	return GL_DYNAMIC_DRAW;
}