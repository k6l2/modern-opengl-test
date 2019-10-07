#include "GlobalUniformBuffer.h"
bool GlobalUniformBuffer::create()
{
	destroy();
	glGenBuffers(1, &bufferObject);
	glBindBuffer(GL_UNIFORM_BUFFER, bufferObject);
	glBufferData(GL_UNIFORM_BUFFER, getSize(), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, NULL);
	const GLenum oglStatus = glGetError();
	if (oglStatus != GL_NO_ERROR)
	{
		SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
					 "Failed to create GlobalUniformBuffer. OGL Error='%s'\n",
					 glewGetErrorString(oglStatus));
		return false;
	}
	return true;
}
void GlobalUniformBuffer::destroy()
{
	if (bufferObject)
	{
		glDeleteBuffers(1, &bufferObject);
		bufferObject = NULL;
	}
}
GLsizei GlobalUniformBuffer::getSize() const
{
	return sizeof(GlobalMatrixBlock);
}
void GlobalUniformBuffer::update(glm::mat4 const& projection3d, 
								 glm::mat3x2 const& view2d) const
{
	SDL_assert(bufferObject);
	glBindBuffer(GL_UNIFORM_BUFFER, bufferObject);
	glBufferData(GL_UNIFORM_BUFFER, getSize(), NULL, GL_STATIC_DRAW);
	GlobalMatrixBlock gmb;
	gmb.projection3d = projection3d;
	gmb.view2d       = glm::mat3(
		view2d[0][0], view2d[0][1], 0,
		view2d[1][0], view2d[1][1], 0,
		view2d[2][0], view2d[2][1], 0);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(gmb), &gmb);
	glBindBuffer(GL_UNIFORM_BUFFER, NULL);
}
void GlobalUniformBuffer::use(GLuint bufferBindingIndex) const
{
	SDL_assert(bufferObject);
	glBindBufferBase(GL_UNIFORM_BUFFER, bufferBindingIndex, bufferObject);
	//glBindBufferRange(GL_UNIFORM_BUFFER, bufferBindingIndex, 
	//				  bufferObject, 0, getSize());
}