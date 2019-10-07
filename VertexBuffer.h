#pragma once
// Unlike the VertexBuffer class in SFML, which is completely fucking useless,
//	this class is used to manage an OpenGL3+ Vertex Buffer Object (VBO).
// These buffers are always used as GL_ARRAY_BUFFER
class VertexBuffer
{
public:
	enum class MemoryUsage : u8
	{
		STATIC,
		DYNAMIC
	};
public:
	bool create(GLsizei dataSize, GLsizei stride, MemoryUsage usage);
	void destroy();
	void update(void const* newData);
	void update(void const* newData, GLint offset, GLsizei size);
	void bind(GLuint bufferBindingIndex) const;
	void bind(GLuint bufferBindingIndex, GLint offset) const;
private:
	static GLenum decodeMemoryUsage(MemoryUsage mu);
private:
	GLuint bufferObject = NULL;
	GLsizei elementStride = 0;
	GLsizei dataSize = 0;
	MemoryUsage usage;
};
