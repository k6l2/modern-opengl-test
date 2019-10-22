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
		DYNAMIC,
		STREAM
	};
	enum class MemoryUnmapResult : u8
	{
		SUCCESS,
		FAILURE,
		ERROR
	};
public:
	bool create(size_t elementCount, GLsizei stride, MemoryUsage usage);
	bool createFixedSize(size_t elementCount, GLsizei stride, MemoryUsage usage);
	void destroy();
	void resize(size_t elementCount);
	void update(void const* newData) const;
	void update(void const* newData, 
				size_t elementOffset, size_t elementCount) const;
	void bind(GLuint bufferBindingIndex) const;
	void bind(GLuint bufferBindingIndex, GLint offset) const;
	void* mapWriteOnly(size_t elementOffset, size_t elementCount, 
					   bool orphanCurrentData) const;
	// If unmap returns FAILURE, that does not necessarily mean a critical error
	//	has occurred.  There are cases where the system state changes during
	//	the time where the memory was mapped, causing corruption to the memory,
	//	such as during screen mode changes.  The caller has to account for this
	//	scenario and retry the data transfers.
	MemoryUnmapResult unmap() const;
private:
	static GLenum decodeMemoryUsage(MemoryUsage mu);
	static GLbitfield decodeFixedSizeMemoryUsage(MemoryUsage mu);
private:
	GLuint bufferObject = NULL;
	GLsizei elementStride = 0;
	GLsizei dataSize = 0;
	MemoryUsage usage;
};
